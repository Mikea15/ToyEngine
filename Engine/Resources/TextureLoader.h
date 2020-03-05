#pragma once

#include <GL/glew.h>
#include <string>

class Texture;
class TextureCube;

class TextureLoader
{
public:
	static Texture LoadTexture(std::string path, GLenum target, GLenum internalFormat, bool srgb = false);
	static Texture LoadHDRTexture(std::string path);

	static TextureCube LoadTextureCube(std::string top, std::string bottom, std::string left, std::string right, std::string front, std::string back);
	static TextureCube LoadTextureCube(std::string folder);
};

