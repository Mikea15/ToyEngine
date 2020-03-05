#include "SceneNode.h"

#include "Scene.h"

#include "Mesh/Mesh.h"
#include "Shading/Material.h"

#include <imgui.h>
#include <assert.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/compatibility.hpp>

SceneNode::SceneNode(unsigned int id)
	: m_id(id)
	, m_parent(nullptr)
{

}

SceneNode::~SceneNode()
{
	// traverse the list of children and delete accordingly.
	for (unsigned int i = 0; i < m_children.size(); ++i)
	{
		// it should not be possible that a scene node is childed by more than one
		// parent, thus we don't need to care about deleting dangling pointers.
		delete m_children[i];
	}
}

void SceneNode::SetPosition(glm::vec3 position)
{
	m_position = position;
	m_isDirty = true;
}

void SceneNode::SetRotation(glm::vec4 rotation)
{
	m_rotation = rotation;
	m_isDirty = true;
}

void SceneNode::SetScale(glm::vec3 scale)
{
	m_scale = scale;
	m_isDirty = true;
}

void SceneNode::SetScale(float scale)
{
	m_scale = glm::vec3(scale);
	m_isDirty = true;
}

glm::vec3 SceneNode::GetLocalPosition()
{
	return m_position;
}

glm::vec4 SceneNode::GetLocalRotation()
{
	return m_rotation;
}

glm::vec3 SceneNode::GetLocalScale()
{
	return m_scale;
}

glm::vec3 SceneNode::GetWorldPosition()
{
	glm::mat4 transform = GetTransform();
	glm::vec4 pos = transform * glm::vec4(m_position, 1.0f);
	return glm::vec3(pos);
}

glm::vec3 SceneNode::GetWorldScale()
{
	glm::mat4 transform = GetTransform();
	glm::vec3 scale = glm::vec3(transform[0][0], transform[1][1], transform[2][2]);
	if (scale.x < 0.0f) scale.x *= -1.0f;
	if (scale.y < 0.0f) scale.y *= -1.0f;
	if (scale.z < 0.0f) scale.z *= -1.0f;
	return scale;
}

unsigned int SceneNode::GetID()
{
	return m_id;
}

void SceneNode::AddChild(SceneNode* node)
{
	// check if this child already has a parent. If so, first remove this scene node from its 
	// current parent. Scene nodes aren't allowed to exist under multiple parents.
	if (node->m_parent)
	{
		node->m_parent->RemoveChild(node->m_id);
	}
	node->m_parent = this;
	m_children.push_back(node);
}

void SceneNode::RemoveChild(unsigned int id)
{
	auto it = std::find(m_children.begin(), m_children.end(), GetChild(id));
	if (it != m_children.end())
		m_children.erase(it);
}

std::vector<SceneNode*> SceneNode::GetChildren()
{
	return m_children;
}

unsigned int SceneNode::GetChildCount()
{
	return m_children.size();
}

SceneNode* SceneNode::GetChild(unsigned int id)
{
	for (unsigned int i = 0; i < m_children.size(); ++i)
	{
		if (m_children[i]->GetID() == id)
			return m_children[i];
	}
	return nullptr;
}

SceneNode* SceneNode::GetChildByIndex(unsigned int index)
{
	assert(index < GetChildCount());
	return m_children[index];
}

SceneNode* SceneNode::GetParent()
{
	return m_parent;
}

glm::mat4 SceneNode::GetTransform()
{
	if (m_isDirty)
	{
		UpdateTransform(false);
	}
	return m_transform;
}

glm::mat4 SceneNode::GetPrevTransform()
{
	return m_prevTransform;
}

void SceneNode::UpdateTransform(bool updatePrevTransform)
{
	// if specified, store current transform as prev transform (for calculating motion vectors)
	if (updatePrevTransform)
	{
		m_prevTransform = m_transform;
	}
	// we only do this if the node itself or its parent is flagged as dirty
	if (m_isDirty)
	{
		// first scale, then rotate, then translation
		m_transform = glm::translate(m_position);
		m_transform = glm::scale(m_transform, m_scale); // TODO: order is off here for some reason, figure out why
		m_transform = glm::rotate(m_transform, glm::radians(m_rotation.w), glm::vec3(m_rotation.x, m_rotation.y, m_rotation.z));

		if (m_parent)
		{
			m_transform = m_parent->m_transform * m_transform;
		}
	}
	for (int i = 0; i < m_children.size(); ++i)
	{
		if (m_isDirty)
		{
			m_children[i]->m_isDirty = true;
		}
		m_children[i]->UpdateTransform(updatePrevTransform);
	}
	m_isDirty = false;
}

void SceneNode::ShowNode(int depth)
{
	ImGui::PushID(m_id);               // Use object uid as identifier. Most commonly you could also use the object pointer as a base ID.
	ImGui::AlignTextToFramePadding();  // Text and Tree nodes are less high than regular widgets, here we add vertical spacing to make the tree lines equal high.
	
	bool node_open = ImGui::TreeNode("Object", "%s_%u", "-", m_id);
	
	ImGui::NextColumn();
	ImGui::AlignTextToFramePadding();

	unsigned int childCount = m_children.size();
	ImGui::Text("Children: %d", childCount);

	ImGui::NextColumn();

	if (node_open)
	{
		ImGui::NextColumn();
		ImGui::AlignTextToFramePadding();

		ImGui::Text("Data when open");

		ImGui::NextColumn();

		for (int i = 0; i < childCount; ++i)
		{
			m_children[i]->ShowNode(depth++);
		}
		ImGui::TreePop();
	}
	ImGui::PopID();
}
