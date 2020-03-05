#include "SimpleRenderer.h"

#include <GL/glew.h>

#include "Camera/Camera.h"

#include "Mesh/Mesh.h"
#include "Scene/SceneNode.h"
#include "Shading/Shader.h"
#include "Shading/Material.h"
#include "MaterialLibrary.h"
#include "RenderTarget.h"

#include "Utils/Logger.h"
#include "DebugDraw.h"

#include <stack>
#include <algorithm>

#define ENABLE_GLSTATE_CACHE 1

SimpleRenderer::~SimpleRenderer()
{
	delete m_materialLibrary;
	// shadows
	for (int i = 0; i < m_ShadowRenderTargets.size(); ++i)
	{
		delete m_ShadowRenderTargets[i];
	}
}

void SimpleRenderer::Init()
{
	m_materialLibrary = new MaterialLibrary();

	// shadows
	for (int i = 0; i < 4; ++i) // allow up to a total of 4 dir/spot shadow casters
	{
		RenderTarget* rt = new RenderTarget(2048, 2048, GL_UNSIGNED_BYTE, 1, true);
		rt->m_DepthStencil.Bind();
		rt->m_DepthStencil.SetFilterMin(GL_NEAREST);
		rt->m_DepthStencil.SetFilterMax(GL_NEAREST);
		rt->m_DepthStencil.SetWrapMode(GL_CLAMP_TO_BORDER);
		float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
		m_ShadowRenderTargets.push_back(rt);
	}

	// ubo
	glGenBuffers(1, &m_GlobalUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, m_GlobalUBO);
	glBufferData(GL_UNIFORM_BUFFER, 720, nullptr, GL_STREAM_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_GlobalUBO);
}

void SimpleRenderer::SetCamera(Camera* camera)
{
	m_camera = camera;
}

void SimpleRenderer::SetRenderSize(int width, int height)
{
	m_renderTargetWidth = width;
	m_renderTargetHeight = height;
}

Material* SimpleRenderer::CreateMaterial(std::string base)
{
	return m_materialLibrary->CreateMaterial(base);
}

Material* SimpleRenderer::CreateCustomMaterial(Shader* shader)
{
	return m_materialLibrary->CreateCustomMaterial(shader);
}

Material* SimpleRenderer::CreatePostProcessingMaterial(Shader* shader)
{
	return m_materialLibrary->CreatePostProcessingMaterial(shader);
}

void SimpleRenderer::PushRender(Mesh* mesh, Material* material, glm::mat4 transform, glm::mat4 prevTransform)
{
	RenderCommand command;
	command.Mesh = mesh;
	command.Material = material;
	command.Transform = transform;
	command.PrevTransform = prevTransform;
	command.BoxMin = glm::vec3(-1.0f);
	command.BoxMax = glm::vec3(1.0f);

	m_renderCommands.push_back(command);
}

void SimpleRenderer::PushRender(SceneNode* node)
{
	node->UpdateTransform(true);

	std::stack<SceneNode*> nodeStack;
	nodeStack.push(node);
	while (!nodeStack.empty())
	{
		SceneNode* currentNode = nodeStack.top();
		nodeStack.pop();

		if (currentNode->Mesh != nullptr) 
		{
			const glm::vec3 boxMinWorld = node->GetWorldPosition() + (node->GetWorldScale() * node->BoxMin);
			const glm::vec3 boxMaxWorld = node->GetWorldPosition() + (node->GetWorldScale() * node->BoxMax);
			const glm::vec3 boxMin = node->GetLocalPosition() + (node->GetLocalScale() * node->BoxMin);
			const glm::vec3 boxMax = node->GetLocalPosition() + (node->GetLocalScale() * node->BoxMax);

			RenderCommand command;
			command.Mesh = currentNode->Mesh;
			command.Material = currentNode->Material;
			command.Transform = currentNode->GetTransform();
			command.PrevTransform = currentNode->GetPrevTransform();
			command.BoxMin = boxMin;
			command.BoxMax = boxMax;
			m_renderCommands.push_back(command);
		}

		for (SceneNode* childNode : currentNode->GetChildren())
		{
			nodeStack.push(childNode);
		}
	}
}

void SimpleRenderer::RenderPushedCommands()
{
	glClearColor(0.2f, 0.2f, 0.6f, 1.0f);
	glViewport(0, 0, m_renderTargetWidth, m_renderTargetHeight);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Update Uniform Buffers.
	glBindBuffer(GL_UNIFORM_BUFFER, m_GlobalUBO);
	// transformation matrices
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), &(m_camera->GetProjection() * m_camera->GetView())[0][0]); // sizeof(glm::mat4) = 64 bytes
	glBufferSubData(GL_UNIFORM_BUFFER, 64, sizeof(glm::mat4), &m_prevViewProjection[0][0]);
	glBufferSubData(GL_UNIFORM_BUFFER, 128, sizeof(glm::mat4), &m_camera->GetProjection()[0][0]);
	glBufferSubData(GL_UNIFORM_BUFFER, 192, sizeof(glm::mat4), &m_camera->GetView()[0][0]);
	glBufferSubData(GL_UNIFORM_BUFFER, 256, sizeof(glm::mat4), &m_camera->GetView()[0][0]); // TODO: make inv function in math library
	// scene data
	glBufferSubData(GL_UNIFORM_BUFFER, 320, sizeof(glm::vec4), &m_camera->GetPosition()[0]);
	// lighting
	unsigned int stride = 2 * sizeof(glm::vec4);
	for (unsigned int i = 0; i < m_DirectionalLights.size() && i < 4; ++i) // no more than 4 directional lights
	{
		glBufferSubData(GL_UNIFORM_BUFFER, 336 + i * stride, sizeof(glm::vec4), &m_DirectionalLights[i]->m_direction[0]);
		glBufferSubData(GL_UNIFORM_BUFFER, 336 + i * stride + sizeof(glm::vec4), sizeof(glm::vec4), &m_DirectionalLights[i]->m_color[0]);
	}
	// No PointLights to add.
	//for (unsigned int i = 0; i < m_PointLights.size() && i < 8; ++i) //  constrained to max 8 point lights in forward context
	//{
	//	glBufferSubData(GL_UNIFORM_BUFFER, 464 + i * stride, sizeof(glm::vec4), &m_PointLights[i]->m_position[0]);
	//	glBufferSubData(GL_UNIFORM_BUFFER, 464 + i * stride + sizeof(glm::vec4), sizeof(glm::vec4), &m_PointLights[i]->m_color[0]);
	//}

	const glm::mat4 view = m_camera->GetView();
	const glm::mat4 projection = m_camera->GetProjection();
	const glm::vec3 cameraPosition = m_camera->GetPosition();

	std::vector<RenderCommand> transparents;
	std::vector<RenderCommand> solids;
	for (auto it = m_renderCommands.rbegin(), end = m_renderCommands.rend(); it != end; ++it)
	{
		if (it->Material->Blend)
		{
			transparents.push_back(*it);
		}
		else
		{
			solids.push_back(*it);
		}
	}
	m_renderCommands.clear();

	if (m_enableShadows)
	{
		m_glState.SetCullFace(GL_FRONT);
		m_ShadowViewProjections.clear();

		unsigned int shadowRtIndex = 0;
		for (int i = 0; i < m_DirectionalLights.size(); ++i)
		{
			DirectionalLight* light = m_DirectionalLights[i];
			if (light->m_castShadows)
			{
				m_materialLibrary->dirShadowShader->Use();
				glBindFramebuffer(GL_FRAMEBUFFER, m_ShadowRenderTargets[shadowRtIndex]->ID);
				glViewport(0, 0, m_ShadowRenderTargets[shadowRtIndex]->Width, m_ShadowRenderTargets[shadowRtIndex]->Height);
				glClear(GL_DEPTH_BUFFER_BIT);

				glm::mat4 lightProjection = glm::ortho(-20.0f, 20.0f, 20.0f, -20.0f, -15.0f, 20.0f);
				glm::mat4 lightView = glm::lookAt(-light->m_direction * 10.0f, glm::vec3(0.0), glm::vec3(0, 1, 0));

				m_DirectionalLights[i]->m_lightSpaceViewPrrojection = lightProjection * lightView;
				m_DirectionalLights[i]->m_shadowMatRenderTarget = m_ShadowRenderTargets[shadowRtIndex];

				for (int j = 0; j < solids.size(); ++j)
				{
					RenderShadowCastCommand(&solids[j], lightView, lightProjection);
				}
				++shadowRtIndex;
			}
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		m_glState.SetCullFace(GL_BACK);
	}

	glClearColor(0.2f, 0.2f, 0.6f, 1.0f);
	glViewport(0, 0, m_renderTargetWidth, m_renderTargetHeight);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	for (RenderCommand rc : solids)
	{

		// Frustum Culling.
		if (m_enableFrustumCulling && !m_camera->GetFrustum().Intersect(rc.BoxMin, rc.BoxMax)) {
			// DebugDraw::AddAABB(rc.BoxMin, rc.BoxMax, { 1.0f, 1.0f, 1.0f, 1.0f });
			continue;
		}

		// DebugDraw::AddAABB(rc.BoxMin, rc.BoxMax, { 0.0f, 1.0f, 0.0f, 1.0f });

		Shader* currentShader = rc.Material->GetShader();

		if (m_enableGLCache) 
		{
			m_glState.SetBlend(rc.Material->Blend);
			if (rc.Material->Blend)
			{
				m_glState.SetBlendFunc(rc.Material->BlendSrc, rc.Material->BlendDst);
			}
			m_glState.SetDepthFunc(rc.Material->DepthCompare);
			m_glState.SetDepthTest(rc.Material->DepthTest);
			m_glState.SetCull(rc.Material->Cull);
			m_glState.SetCullFace(rc.Material->CullFace);

			m_glState.SwitchShader(currentShader->ID);
		}
		else 
		{
			// Blend
			if (rc.Material->Blend) 
			{ 
				glEnable(GL_BLEND);
				glBlendFunc(rc.Material->BlendSrc, rc.Material->BlendDst);
			}
			else 
			{ 
				glDisable(GL_BLEND); 
			}

			glEnable(rc.Material->Cull);
			glDepthFunc(rc.Material->DepthCompare);
			if (rc.Material->DepthTest)
			{
				glEnable(GL_DEPTH_TEST);
			}
			else
			{
				glDisable(GL_DEPTH_TEST);
			}

			if (rc.Material->Cull)
			{
				glEnable(GL_CULL_FACE);
			}
			else
			{
				glDisable(GL_CULL_FACE);
			}
			glCullFace(rc.Material->CullFace);
			currentShader->Use();
		}

		currentShader->SetMatrix("view", view);
		currentShader->SetMatrix("projection", projection);
		currentShader->SetVector("CamPos", cameraPosition);

		currentShader->SetMatrix("model", rc.Transform);
		currentShader->SetMatrix("prevModel", rc.PrevTransform);

		currentShader->SetBool("ShadowsEnabled", m_enableShadows);
		if (m_enableShadows && rc.Material->Type == MATERIAL_CUSTOM && rc.Material->ShadowReceive)
		{
			for (int i = 0; i < m_DirectionalLights.size(); ++i)
			{
				if (m_DirectionalLights[i]->m_shadowMatRenderTarget)
				{
					currentShader->SetMatrix("lightShadowViewProjection" + std::to_string(i + 1), m_DirectionalLights[i]->m_lightSpaceViewPrrojection);
					m_DirectionalLights[i]->m_shadowMatRenderTarget->GetDepthStencilTexture()->Bind(10 + i);
				}
			}
		}

		std::map<std::string, UniformValueSampler>* samplers = rc.Material->GetSamplerUniforms();
		for (auto it = samplers->begin(), end = samplers->end(); it != end; ++it)
		{
			if (it->second.Type == SHADER_TYPE_SAMPLERCUBE) 
			{
				it->second.TextureCube->Bind(it->second.Unit);
			}
			else
			{
				it->second.Texture->Bind(it->second.Unit);
			}
		}

		std::map<std::string, UniformValue>* uniforms = rc.Material->GetUniforms();
		for (auto it = uniforms->begin(), end = uniforms->end(); it != end; ++it)
		{
			switch (it->second.Type)
			{
			case SHADER_TYPE_BOOL:
				currentShader->SetBool(it->first, it->second.Bool);
				break;
			case SHADER_TYPE_INT:
				currentShader->SetInt(it->first, it->second.Int);
				break;
			case SHADER_TYPE_FLOAT:
				currentShader->SetFloat(it->first, it->second.Float);
				break;
			case SHADER_TYPE_VEC2:
				currentShader->SetVector(it->first, it->second.Vec2);
				break;
			case SHADER_TYPE_VEC3:
				currentShader->SetVector(it->first, it->second.Vec3);
				break;
			case SHADER_TYPE_VEC4:
				currentShader->SetVector(it->first, it->second.Vec4);
				break;
			case SHADER_TYPE_MAT2:
				currentShader->SetMatrix(it->first, it->second.Mat2);
				break;
			case SHADER_TYPE_MAT3:
				currentShader->SetMatrix(it->first, it->second.Mat3);
				break;
			case SHADER_TYPE_MAT4:
				currentShader->SetMatrix(it->first, it->second.Mat4);
				break;
			default:
				LOG_ERROR("Unrecognized Uniform type set.");
				break;
			}
		}

		// Render Mesh
		RenderMesh(rc.Mesh);
	}
	solids.clear();

#if 0
	// back to front render for transparents.
	std::sort(transparents.begin(), transparents.end(), [](RenderCommand lhs, RenderCommand rhs) {
		return lhs.Transform[3].z > rhs.Transform[3].z;
		});

	for (RenderCommand rc : transparents)
	{
		// Frustum Culling.
		if (m_enableFrustumCulling && !m_camera->GetFrustum().Intersect(rc.BoxMin, rc.BoxMax)) {
			continue;
		}

		Shader* currentShader = rc.Material->GetShader();

		if (m_enableGLCache)
		{
			m_glState.SetBlend(rc.Material->Blend);
			if (rc.Material->Blend)
			{
				m_glState.SetBlendFunc(rc.Material->BlendSrc, rc.Material->BlendDst);
			}
			m_glState.SetDepthFunc(rc.Material->DepthCompare);
			m_glState.SetDepthTest(rc.Material->DepthTest);
			m_glState.SetCull(rc.Material->Cull);
			m_glState.SetCullFace(rc.Material->CullFace);

			m_glState.SwitchShader(currentShader->ID);
		}
		else
		{
			// Blend
			if (rc.Material->Blend)
			{
				glEnable(GL_BLEND);
				glBlendFunc(rc.Material->BlendSrc, rc.Material->BlendDst);
			}
			else
			{
				glDisable(GL_BLEND);
			}

			glEnable(rc.Material->Cull);
			glDepthFunc(rc.Material->DepthCompare);
			if (rc.Material->DepthTest)
			{
				glEnable(GL_DEPTH_TEST);
			}
			else
			{
				glDisable(GL_DEPTH_TEST);
			}

			if (rc.Material->Cull)
			{
				glEnable(GL_CULL_FACE);
			}
			else
			{
				glDisable(GL_CULL_FACE);
			}
			glCullFace(rc.Material->CullFace);
			currentShader->Use();
		}

		currentShader->SetMatrix("view", view);
		currentShader->SetMatrix("projection", projection);
		currentShader->SetVector("CamPos", cameraPosition);

		currentShader->SetMatrix("model", rc.Transform);
		currentShader->SetMatrix("prevModel", rc.PrevTransform);

		std::map<std::string, UniformValueSampler>* samplers = rc.Material->GetSamplerUniforms();
		for (auto it = samplers->begin(), end = samplers->end(); it != end; ++it)
		{
			if (it->second.Type == SHADER_TYPE_SAMPLERCUBE)
			{
				it->second.TextureCube->Bind(it->second.Unit);
			}
			else
			{
				it->second.Texture->Bind(it->second.Unit);
			}
		}

		std::map<std::string, UniformValue>* uniforms = rc.Material->GetUniforms();
		for (auto it = uniforms->begin(), end = uniforms->end(); it != end; ++it)
		{
			switch (it->second.Type)
			{
			case SHADER_TYPE_BOOL:
				currentShader->SetBool(it->first, it->second.Bool);
				break;
			case SHADER_TYPE_INT:
				currentShader->SetInt(it->first, it->second.Int);
				break;
			case SHADER_TYPE_FLOAT:
				currentShader->SetFloat(it->first, it->second.Float);
				break;
			case SHADER_TYPE_VEC2:
				currentShader->SetVector(it->first, it->second.Vec2);
				break;
			case SHADER_TYPE_VEC3:
				currentShader->SetVector(it->first, it->second.Vec3);
				break;
			case SHADER_TYPE_VEC4:
				currentShader->SetVector(it->first, it->second.Vec4);
				break;
			case SHADER_TYPE_MAT2:
				currentShader->SetMatrix(it->first, it->second.Mat2);
				break;
			case SHADER_TYPE_MAT3:
				currentShader->SetMatrix(it->first, it->second.Mat3);
				break;
			case SHADER_TYPE_MAT4:
				currentShader->SetMatrix(it->first, it->second.Mat4);
				break;
			default:
				LOG_ERROR("Unrecognized Uniform type set.");
				break;
			}
		}

		// Render Mesh
		RenderMesh(rc.Mesh);
	}

	transparents.clear();
#endif

	// store view projection as previous view projection for next frame's motion blur
	m_prevViewProjection = m_camera->GetProjection() * m_camera->GetView();
}

void SimpleRenderer::RenderUIMenu()
{
	if (ImGui::BeginMenu("Simple Renderer"))
	{
		ImGui::Checkbox("Enable Frustum Culling", &m_enableFrustumCulling);
		ImGui::Checkbox("Enable GL Cache", &m_enableGLCache);
		ImGui::Checkbox("Enable Shadows", &m_enableShadows);
		ImGui::EndMenu();
	}
}

void SimpleRenderer::RenderShadowCastCommand(RenderCommand* rc, const glm::mat4& view, const glm::mat4& projection)
{
	Shader* shadowShader = m_materialLibrary->dirShadowShader;

	shadowShader->SetMatrix("view", view);
	shadowShader->SetMatrix("projection", projection);
	shadowShader->SetMatrix("model", rc->Transform);

	RenderMesh(rc->Mesh);
}


void SimpleRenderer::RenderMesh(Mesh* mesh)
{
	// Render Mesh
	glBindVertexArray(mesh->m_VAO);

	const GLenum mode = mesh->Topology == TOPOLOGY::TRIANGLE_STRIP ? GL_TRIANGLE_STRIP : GL_TRIANGLES;
	if (!mesh->Indices.empty())
	{
		glDrawElements(mode, mesh->Indices.size(), GL_UNSIGNED_INT, 0);
	}
	else
	{
		glDrawArrays(mode, 0, mesh->Positions.size());
	}
}