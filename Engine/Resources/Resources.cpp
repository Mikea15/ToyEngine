#include "Resources.h"

#include "ShaderLoader.h"
#include "TextureLoader.h"
#include "MeshLoader.h"

#include "Scene/Scene.h"
#include "Scene/SceneNode.h"

#include "Utils/Utils.h"

#include <stack>
#include <vector>

#include "Utils/Logger.h"

const std::string Resources::s_mainAssetDirectory = "../Data/";
const std::string Resources::s_assetShaderDir = "Shaders/";
const std::string Resources::s_assetModelDir = "Objects/";
const std::string Resources::s_assetImagesDir = "Images/";

std::map<unsigned int, Shader>      Resources::m_shaders = std::map<unsigned int, Shader>();
std::map<unsigned int, Texture>     Resources::m_textures = std::map<unsigned int, Texture>();
std::map<unsigned int, TextureCube> Resources::m_texturesCube = std::map<unsigned int, TextureCube>();
std::map<unsigned int, SceneNode*>  Resources::m_meshes = std::map<unsigned int, SceneNode*>();

void Resources::Init()
{
	Texture placeholderTexture;
}

void Resources::Clean()
{
	for (auto it = m_meshes.begin(); it != m_meshes.end(); it++)
	{
		delete it->second;
	}
}

Shader* Resources::LoadShader(const std::string& name, const std::string& vsPath, const std::string& fsPath, std::vector<std::string> defines)
{
	unsigned int id = Utils::Hash(name);

	auto it = m_shaders.find(id);
	if (it != m_shaders.end())
	{
		return &it->second;
	}

	Shader shader = ShaderLoader::Load(name,
		s_mainAssetDirectory + vsPath,
		s_mainAssetDirectory + fsPath, defines);

	auto result = m_shaders.emplace(id, shader);
	if (result.second)
	{
		return &result.first->second;
	}

	LOG_ERROR("Could not load shader: %s", name.c_str());
	// return error shader
	return nullptr;
}

Shader* Resources::GetShader(const std::string& name)
{
	unsigned int id = Utils::Hash(name);

	auto it = m_shaders.find(id);
	if (it != m_shaders.end())
	{
		return &it->second;
	}

	LOG_ERROR("Requested shader: %s not found!", name.c_str());
	// return error shader
	return nullptr;
}

Texture* Resources::LoadTexture(const std::string& name, const std::string& path, GLenum target, GLenum format, bool srgb, bool fullpath)
{
	unsigned int id = Utils::Hash(name);

	auto it = m_textures.find(id);
	if (it != m_textures.end())
	{
		return &it->second;
	}

	std::string finalPath = fullpath ? path : s_mainAssetDirectory + path;

	LOG("Loading texture file at: %s", finalPath.c_str());

	Texture texture = TextureLoader::LoadTexture(finalPath, target, format, srgb);

	if (texture.Width > 0)
	{
		LOG("Succesfully loaded: %s", finalPath.c_str());

		m_textures[id] = texture;
		return &m_textures[id];
	}

	LOG_ERROR("Texture not loaded: %s", finalPath.c_str());

	return nullptr;
}

Texture* Resources::LoadHDR(const std::string& name, const std::string& path)
{
	unsigned int id = Utils::Hash(name);

	auto it = m_textures.find(id);
	if (it != m_textures.end())
	{
		return &it->second;
	}

	LOG("Loading texture file at: %s", path.c_str());

	Texture texture = TextureLoader::LoadHDRTexture(s_mainAssetDirectory + path);

	LOG("Succesfully loaded: %s", path.c_str());

	if (texture.Width > 0)
	{
		m_textures[id] = texture;
		return &m_textures[id];
	}

	return nullptr;
}

Texture* Resources::GetTexture(const std::string& name)
{
	unsigned int id = Utils::Hash(name);

	auto it = m_textures.find(id);
	if (it != m_textures.end())
	{
		return &it->second;
	}

	LOG("Requested texture: %s not found", name.c_str());

	// return invalid texture
	return nullptr;
}

TextureCube* Resources::LoadTextureCube(const std::string& name, const std::string& folder)
{
	unsigned int id = Utils::Hash(name);

	if (TextureCube* texture = GetTextureCube(name))
	{
		return texture;
	}

	TextureCube texture = TextureLoader::LoadTextureCube(s_mainAssetDirectory + folder);

	auto result = m_texturesCube.emplace(id, texture);
	if (result.second)
	{
		return &result.first->second;
	}

	// return invalid texture
	return nullptr;
}

TextureCube* Resources::GetTextureCube(const std::string& name)
{
	unsigned int id = Utils::Hash(name);

	auto it = m_texturesCube.find(id);
	if (it != m_texturesCube.end())
	{
		return &it->second;
	}

	LOG("Requested texture cube: %s not found!", name.c_str());
	
	// return invalid texture
	return nullptr;
}

SceneNode* Resources::LoadMesh(IRenderer* renderer, const std::string& name, const std::string& path)
{
	unsigned int id = Utils::Hash(name);

	auto it = m_meshes.find(id);
	if (it != m_meshes.end())
	{
		// Return copy of pointer.
		return Scene::MakeSceneNode(it->second);
	}
	
	SceneNode* node = MeshLoader::LoadMesh(renderer, s_mainAssetDirectory + path);
	auto result = m_meshes.emplace(id, node);
	if (result.second)
	{
		// Return copy of pointer.
		return Scene::MakeSceneNode(result.first->second);
	}

	return nullptr;
}

SceneNode* Resources::GetMesh(const std::string& name)
{
	unsigned int id = Utils::Hash(name);

	auto it = m_meshes.find(id);
	if (it != m_meshes.end())
	{
		// Return copy of pointer.
		return Scene::MakeSceneNode(it->second);
	}

	LOG("Requested mesh: %s not found!", name.c_str());
	return nullptr;
}
