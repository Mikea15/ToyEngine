#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include "ShadingTypes.h"


/*

  Shader object for quickly creating and using a GPU shader object. When compiling/linking a
  shader object from source code, all vertex attributes and uniforms are extracted for saving
  unnecessary additional CPU->GPU roundtrip times.

*/
class Shader
{
public:
	unsigned int ID;
	std::string  Name;

	std::vector<Uniform>         Uniforms;
	std::vector<VertexAttribute> Attributes;

public:
	Shader();
	Shader(const std::string& name, std::string vsCode, std::string fsCode, std::vector<std::string> defines = std::vector<std::string>());

	void Load(const std::string& name, std::string vsCode, std::string fsCode, std::vector<std::string> defines = std::vector<std::string>());

	void Use();

	bool HasUniform(const std::string& name);

	void SetInt(const std::string& location, int   value);
	void SetBool(const std::string& location, bool  value);
	void SetFloat(const std::string& location, float value);
	void SetVector(const std::string& location, glm::vec2  value);
	void SetVector(const std::string& location, glm::vec3  value);
	void SetVector(const std::string& location, glm::vec4  value);
	void SetVectorArray(const std::string& location, int size, const std::vector<glm::vec2>& values);
	void SetVectorArray(const std::string& location, int size, const std::vector<glm::vec3>& values);
	void SetVectorArray(const std::string& location, int size, const std::vector<glm::vec4>& values);
	void SetMatrix(const std::string& location, glm::mat2 value);
	void SetMatrix(const std::string& location, glm::mat3 value);
	void SetMatrix(const std::string& location, glm::mat4 value);
	void SetMatrixArray(const std::string& location, int size, glm::mat2* values);
	void SetMatrixArray(const std::string& location, int size, glm::mat3* values);
	void SetMatrixArray(const std::string& location, int size, glm::mat4* values);
private:
	// retrieves uniform location from pre-stored uniform locations and reports an error if a 
	// non-uniform is set.
	int getUniformLocation(const std::string& name);

	std::unordered_map<unsigned int, unsigned int> m_hashToUniformIndex;
	std::unordered_map<unsigned int, Uniform> m_uniformMap;
};

