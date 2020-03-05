#pragma once

#include <string>
#include <fstream>

#include "Shading/Shader.h"


class ShaderLoader
{
public:
	static Shader Load(std::string name, std::string vsPath, std::string fsPath, std::vector<std::string> defines = std::vector<std::string>());
	static Shader LoadWithString(std::string name, std::string vsString, std::string fsString, std::vector<std::string> defines = std::vector<std::string>());

private:
	static std::string readShader(std::ifstream& file, const std::string& name, std::string path);
};


