#pragma once

#include <string>
#include <algorithm>
#include <vector>
#include <cstdlib>

#include <glm/glm.hpp>
#include <imgui.h>

template<typename E>
struct EnableBitMaskOperators
{
	static const bool enable = false;
};

template<typename E>
typename std::enable_if<EnableBitMaskOperators<E>::enable, E>::type
operator |(E lhs, E rhs)
{
	using underlyingType = typename std::underlying_type<E>::type;
	return static_cast<E>(static_cast<underlyingType>(lhs) | static_cast<underlyingType>(lhs));
}

template<typename E>
typename std::enable_if<EnableBitMaskOperators<E>::enable, E>::type
operator &(E lhs, E rhs)
{
	using underlyingType = typename std::underlying_type<E>::type;
	return static_cast<E>(static_cast<underlyingType>(lhs) & static_cast<underlyingType>(lhs));
}

#define ENABLE_BITMASK_OPERATORS(x)  \
template<>                           \
struct EnableBitMaskOperators<x>     \
{                                    \
	static const bool enable = true; \
};

namespace Utils
{
	static std::string Lowercase(const std::string& text)
	{
		std::string lowercase = text;
		std::transform(lowercase.begin(), lowercase.end(), lowercase.begin(), ::tolower);
		return lowercase;
	}

	static std::string NormalizePath(const std::string& in)
	{
		std::string result = in;
		std::replace(result.begin(), result.end(), '\\', '/');
		return result;
	}

	static unsigned int Hash(const std::string& text)
	{
		return static_cast<unsigned int>(std::hash<std::string>{}(text));
	}
}

namespace ImGui
{
	static auto vector_getter = [](void* vec, int idx, const char** out_text)
	{
		auto& vector = *static_cast<std::vector<std::string>*>(vec);
		if (idx < 0 || idx >= static_cast<int>(vector.size())) { return false; }
		*out_text = vector.at(idx).c_str();
		return true;
	};

	static bool Combo(const char* label, int* currIndex, std::vector<std::string>& values)
	{
		if (values.empty()) { return false; }
		return Combo(label, currIndex, vector_getter,
			static_cast<void*>(&values), static_cast<int>(values.size()));
	}

	static bool ListBox(const char* label, int* currIndex, std::vector<std::string>& values)
	{
		if (values.empty()) { return false; }
		return ListBox(label, currIndex, vector_getter,
			static_cast<void*>(&values), static_cast<int>(values.size()));
	}

	static void Color3(const char* label, glm::vec3& outColor, ImGuiColorEditFlags flags = ImGuiColorEditFlags_RGB)
	{
		float color[3] = { outColor.x, outColor.y, outColor.z };
		ColorEdit3(label, color, flags);
		outColor = glm::vec3(color[0], color[1], color[2]);
	}

	static void SliderFloat3(const char* label, glm::vec3& outPos, float min, float max)
	{
		float pos[3] = { outPos.x, outPos.y, outPos.z };
		SliderFloat3(label, pos, min, max);
		outPos = glm::vec3(pos[0], pos[1], pos[2]);
	}
}