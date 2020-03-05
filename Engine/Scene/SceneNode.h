#pragma once

#include <vector>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

class Scene;
class Mesh;
class Material;

/* NOTE(Joey):

  An individual scene node that links a mesh to a material
  to render the mesh with while at the same time
  maintaining a parent-child hierarchy for transform
  relations.

  Each node can have any number of children (via a linked
  list) and represents a single render able entity in a
  larger scene where each child transform on top of their
  parent node.

*/
class SceneNode
{
public:
	SceneNode(unsigned int id);
	~SceneNode();

	void SetPosition(glm::vec3 position);
	void SetRotation(glm::vec4 rotation);
	void SetScale(glm::vec3 scale);
	void SetScale(float scale);
	glm::vec3 GetLocalPosition();
	glm::vec4 GetLocalRotation();
	glm::vec3 GetLocalScale();
	glm::vec3 GetWorldPosition();
	glm::vec3 GetWorldScale();

	unsigned int GetID();
	void AddChild(SceneNode* node);
	void RemoveChild(unsigned int id);
	std::vector<SceneNode*> GetChildren();
	unsigned int GetChildCount();
	SceneNode* GetChild(unsigned int id);
	SceneNode* GetChildByIndex(unsigned int index);
	SceneNode* GetParent();

	glm::mat4 GetTransform();
	glm::mat4 GetPrevTransform();

	void UpdateTransform(bool updatePrevTransform = false);

	void ShowNode(int depth);

public:
	Mesh* Mesh{};
	Material* Material{};

	// bounding box 
	glm::vec3 BoxMin = glm::vec3(-1.0f);
	glm::vec3 BoxMax = glm::vec3(1.0f);

private:
	std::vector<SceneNode*> m_children;
	SceneNode* m_parent;

	// per-node transform (w/ parent-child relationship)
	glm::mat4 m_transform = glm::identity<glm::mat4>();
	glm::mat4 m_prevTransform = glm::identity<glm::mat4>();
	glm::vec3 m_position = glm::vec3(0.0f);
	glm::vec4 m_rotation = glm::vec4(0, 1, 0, 0);
	glm::vec3 m_scale = glm::vec3(1.0f);

	bool m_isDirty;

	unsigned int m_id;
};

