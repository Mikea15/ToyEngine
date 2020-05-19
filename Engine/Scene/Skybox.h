#pragma once

#include "SceneNode.h"

class TextureCube;
class Shader;

class Skybox
	: public SceneNode
{
public:
	Skybox();
	~Skybox();

	void SetCubemap(TextureCube* cubemap);

private:
	TextureCube* m_cubeMap;
	Shader* m_shader;
};

