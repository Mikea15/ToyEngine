#pragma once

#include "IRenderer.h"

#include "Lighting/PointLight.h"
#include "Lighting/DirectionalLight.h"
#include "Mesh/Quad.h"
#include "CommandBuffer.h"
#include "PBRCapture.h"
#include "GLStateCache.h"

#include <string>

#include <GL/glew.h>

#include <glm/glm.hpp>



/*

  Forward declare the types, s.t. we don't need to include the required header files here.
  Seeing as several objects throughout Cell will link to renderer.h we want to reduce as much
  unnecessary additional header code as much as possible (saves in compilation times).

*/
class Mesh;
class Material;
class Scene;
class SceneNode;
class Camera;
class RenderTarget;
class MaterialLibrary;
class PBR;
class PostProcessor;
class Texture;
class Shader;

/*

  The main renderer; responsible for maintaining a render buffer queue, providing the front
  push commands for filling the buffer, sorting the buffer, manage multiple render passes and
  render the buffer(s) accordingly.

*/
class Renderer
	: public IRenderer
{
	friend PostProcessor;
	friend PBR;
public:
	// configuration
	bool IrradianceGI = true;
	bool Shadows = true;
	bool Lights = true;
	bool RenderLights = true;
	bool LightVolumes = false;
	bool RenderProbes = false;
	bool Wireframe = false;
private:
	// render state
	CommandBuffer* m_CommandBuffer{};
	GLStateCache        m_GLCache;
	glm::vec2      m_RenderSize;

	// lighting
	std::vector<DirectionalLight*> m_DirectionalLights;
	std::vector<PointLight*>       m_PointLights;
	RenderTarget* m_GBuffer{};
	Mesh* m_DeferredPointMesh{};

	// materials
	MaterialLibrary* m_MaterialLibrary;

	// camera
	Camera* m_Camera{};
	glm::mat4 m_PrevViewProjection;

	// render-targets/post
	std::vector<RenderTarget*> m_RenderTargetsCustom;
	RenderTarget* m_CurrentRenderTargetCustom{};
	RenderTarget* m_CustomTarget{};
	RenderTarget* m_PostProcessTarget1{};
	PostProcessor* m_PostProcessor{};
	Quad* m_NDCPlane{};
	unsigned int m_FramebufferCubemap;
	unsigned int m_CubemapDepthRBO;

	// shadow buffers
	std::vector<RenderTarget*> m_ShadowRenderTargets;
	std::vector<glm::mat4>    m_ShadowViewProjections;

	// pbr
	PBR* m_PBR{};
	unsigned int m_PBREnvironmentIndex;
	std::vector<glm::vec4> m_ProbeSpatials;

	// ubo
	unsigned int m_GlobalUBO;

	// debug
	Mesh* m_DebugLightMesh{};

public:
	Renderer();
	~Renderer();

	void Init(/*GLADloadproc loadProcFunc*/);

	void SetRenderSize(unsigned int width, unsigned int height);
	glm::vec2 GetRenderSize();

	void SetTarget(RenderTarget* renderTarget, GLenum target = GL_TEXTURE_2D);

	Camera* GetCamera();
	void    SetCamera(Camera* camera);

	PostProcessor* GetPostProcessor();

	// create either a deferred default material (based on default set of materials available (like glass)), or a custom material (with custom you have to supply your own shader)
	Material* CreateMaterial(std::string base = "default") override; // these don't have the custom flag set (default material has default state and uses checkerboard texture as albedo (and black metallic, half roughness, purple normal, white ao)
	Material* CreateCustomMaterial(Shader* shader);         // these have the custom flag set (will be rendered in forward pass)
	Material* CreatePostProcessingMaterial(Shader* shader); // these have the post-processing flag set (will be rendered after deferred/forward pass)

	void PushRender(Mesh* mesh, Material* material, glm::mat4 transform = glm::mat4(), glm::mat4 prevFrameTransform = glm::mat4());
	void PushRender(SceneNode* node);
	void PushPostProcessor(Material* postProcessor);

	void AddLight(DirectionalLight* light);
	void AddLight(PointLight* light);

	void RenderPushedCommands();

	void Blit(Texture* src, RenderTarget* dst = nullptr, Material* material = nullptr, std::string textureUniformName = "TexSrc");

	// pbr
	void        SetSkyCapture(PBRCapture* pbrEnvironment);
	PBRCapture* GetSkypCature();
	void        AddIrradianceProbe(glm::vec3 position, float radius);
	void        BakeProbes(SceneNode* scene = nullptr);
private:
	// renderer-specific logic for rendering a custom (forward-pass) command
	void renderCustomCommand(RenderCommand* command, Camera* customCamera, bool updateGLSettings = true);
	// renderer-specific logic for rendering a list of commands to a target cubemap
	void renderToCubemap(SceneNode* scene, TextureCube* target, glm::vec3 position = glm::vec3(0.0f), unsigned int mipLevel = 0);
	void renderToCubemap(std::vector<RenderCommand>& renderCommands, TextureCube* target, glm::vec3 position = glm::vec3(0.0f), unsigned int mipLevel = 0);
	// minimal render logic to render a mesh 
	void renderMesh(Mesh* mesh, Shader* shader);
	// updates the global uniform buffer objects
	void updateGlobalUBOs();
	// returns the currently active render target
	RenderTarget* getCurrentRenderTarget();

	// deferred logic:
	// renders all ambient lighting (including indirect IBL)
	void renderDeferredAmbient();
	// render directional light
	void renderDeferredDirLight(DirectionalLight* light);
	// render point light
	void renderDeferredPointLight(PointLight* light);

	// render mesh for shadow buffer generation
	void renderShadowCastCommand(RenderCommand* command, const glm::mat4& projection, const glm::mat4& view);
};


