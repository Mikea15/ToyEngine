#pragma once

#include "Shading/Shader.h"
#include "Shading/Material.h"
#include "Shading/Texture.h"
#include "Shading/TextureCube.h"
#include "Mesh/Mesh.h"

#include <map>
#include <string>

class SceneNode;
class IRenderer;

class Resources
{
public:
	static void Init();
	static void Clean();

	// shader resources
	static Shader* LoadShader(const std::string& name, const std::string& vsPath,  const std::string& fsPath, std::vector<std::string> defines = std::vector<std::string>());
	static Shader* GetShader(const std::string& name);

	// texture resources
	static Texture* LoadTexture(const std::string& name, const std::string& path, GLenum target = GL_TEXTURE_2D, GLenum format = GL_RGBA, bool srgb = false, bool fullpath = false);
	static Texture* LoadHDR(const std::string& name, const std::string& path);
	static Texture* GetTexture(const std::string& name);
	static TextureCube* LoadTextureCube(const std::string& name, const std::string& folder);
	static TextureCube* GetTextureCube(const std::string& name);

	// mesh/scene resources
	static SceneNode* LoadMesh(IRenderer* renderer, const std::string& name, const std::string& path);
	static SceneNode* GetMesh(const std::string& name);

private:
	Resources() = delete;

private:
	// we index all resources w/ a hashed string ID
	static std::map<unsigned int, Shader>      m_shaders;
	static std::map<unsigned int, Texture>     m_textures;
	static std::map<unsigned int, TextureCube> m_texturesCube;
	static std::map<unsigned int, SceneNode*>  m_meshes;

	static const std::string s_mainAssetDirectory;
	static const std::string s_assetShaderDir;
	static const std::string s_assetModelDir;
	static const std::string s_assetImagesDir;
};

