#include "ShaderLoader.h"

#include "Utils/Logger.h"

Shader ShaderLoader::Load(std::string name, std::string vsPath, std::string fsPath, std::vector<std::string> defines)
{
	std::ifstream vsFile, fsFile;
	vsFile.open(vsPath);
	fsFile.open(fsPath);

	// if either of the two files don't exist, return w/ error message
	if (!vsFile.is_open() || !fsFile.is_open())
	{
		LOG_ERROR("Shader failed to load at path: %s and %s", vsPath.c_str(), fsPath.c_str());
		return Shader();
	}

	// retrieve directory (for relative paths in shader includes)
	std::string vsDirectory = vsPath.substr(0, vsPath.find_last_of("/\\"));
	std::string fsDirectory = fsPath.substr(0, fsPath.find_last_of("/\\"));
	std::string vsSource = readShader(vsFile, name, vsPath);
	std::string fsSource = readShader(fsFile, name, fsPath);

	// now build the shader with the source code
	Shader shader(name, vsSource, fsSource, defines);

	vsFile.close();
	fsFile.close();

	return shader;
}

Shader ShaderLoader::LoadWithString(std::string name, std::string vsString, std::string fsString, std::vector<std::string> defines)
{
	return Shader(name, vsString, fsString, defines);
}

std::string ShaderLoader::readShader(std::ifstream& file, const std::string& name, std::string path)
{
	std::string directory = path.substr(0, path.find_last_of("/\\"));
	std::string source, line;
	while (std::getline(file, line))
	{
		// if we encounter an #include line, include other shader source
		if (line.substr(0, 8) == "#include")
		{
			std::string includePath = directory + "/" + line.substr(9);
			std::ifstream includeFile(includePath);
			if (includeFile.is_open())
			{
				// we recursively read the shader file to support any shader include depth
				source += readShader(includeFile, name, includePath);
			}
			else
			{
				LOG_ERROR("Shader: %s include: %s failed to open.", name, includePath);
			}
			includeFile.close();
		}
		else
			source += line + "\n";
	}
	return source;
}

