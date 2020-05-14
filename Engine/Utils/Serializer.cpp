
#include "Serializer.h"

#include "Scene/Scene.h"
#include "Scene/SceneNode.h"

#include <glm/glm.hpp>

SceneNode* Serializer::LoadScene()
{
	std::string tempLevelPath = "../../../../data/level.json";

	return nullptr;
}

void Serializer::SaveScene(SceneNode* node)
{
	std::string tempLevelPath = "../../../../data/level.json";
}


namespace glm
{
	void to_json(json& j, const vec3& v)
	{
		j = json{
			{ "x", v.x },
			{ "y", v.y },
			{ "z", v.z }
		};
	}

	void from_json(const json& j, vec3& v)
	{
		j.at("x").get_to(v.x);
		j.at("y").get_to(v.y);
		j.at("z").get_to(v.z);
	}

	std::string toJson(const glm::vec3& v)
	{
		return static_cast<json>(v).dump();
	}

	void to_json(json& j, const vec4& v)
	{
		j = json{
			{ "x", v.x },
			{ "y", v.y },
			{ "z", v.z },
			{ "w", v.w }
		};
	}

	void from_json(const json& j, vec4& v)
	{
		j.at("x").get_to(v.x);
		j.at("y").get_to(v.y);
		j.at("z").get_to(v.z);
		j.at("w").get_to(v.w);
	}

	std::string toJson(const glm::vec4& v)
	{
		return static_cast<json>(v).dump();
	}
}