#pragma once

#include <string>
#include <vector>

#include <glm/glm.hpp>

struct aiNode;
struct aiScene;
struct aiMesh;
struct aiMaterial;
struct aiString;


class IRenderer;
class SceneNode;
class Mesh;
class Material;

/*

  Mesh load functionality.

*/
class MeshLoader
{
private:
	// NOTE(Joey): keep track of all loaded mesh
	static std::vector<Mesh*> meshStore;
public:
	static void       Clean();
	static SceneNode* LoadMesh(IRenderer* renderer, std::string path, bool setDefaultMaterial = true);
private:
	static SceneNode* processNode(IRenderer* renderer, aiNode* aNode, const aiScene* aScene, std::string directory, bool setDefaultMaterial);
	static Mesh* parseMesh(aiMesh* aMesh, const aiScene* aScene, glm::vec3& out_Min, glm::vec3& out_Max);
	static Material* parseMaterial(IRenderer* renderer, aiMaterial* aMaterial, const aiScene* aScene, std::string directory);

	static std::string processPath(aiString* path, std::string directory);
};

