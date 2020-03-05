
#pragma once

#include <glm/fwd.hpp>
#include <nlohmann/json.hpp>

using namespace nlohmann;

class SceneNode;

class Serializer
{
public:
	static SceneNode* LoadScene();
	static void SaveScene(SceneNode* node);
};

namespace glm
{
	void to_json(json& j, const vec3& v);
	void from_json(const json& j, vec3& v);
	void to_json(json& j, const vec4& v);
	void from_json(const json& j, vec4& v);
}