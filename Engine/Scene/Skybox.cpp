#include "Skybox.h"

#include "Scene.h"

#include "Renderer/Renderer.h"
#include "Resources/Resources.h"
#include "Shading/Shader.h"
#include "Shading/Material.h"
#include "Shading/TextureCube.h"
#include "Mesh/Cube.h"


Skybox::Skybox() 
	: SceneNode(Scene::CounterID++)
{
	// Auto Add Skybox to scene.
	Scene::Root->AddChild(this);

	m_shader = Resources::LoadShader("background", "shaders/background.vs", "shaders/background.fs");
	// Material = new Material(m_shader);
	Mesh = new Cube();
	BoxMin = glm::vec3(-99999.0);
	BoxMax = glm::vec3(99999.0);

	// default material configuration
	Material->SetFloat("Exposure", 1.0f);
	Material->DepthCompare = GL_LEQUAL;
	Material->Cull = false;
	Material->ShadowCast = false;
	Material->ShadowReceive = false;
}

Skybox::~Skybox()
{

}

void Skybox::SetCubemap(TextureCube* cubemap)
{
	m_cubeMap = cubemap;
	// Material->SetTextureCube("background", m_cubeMap, 0);
}
