#pragma once

#include "SceneNode.h"

class TextureCube;
class Shader;

/*
  A SkyBox represented as a scene node for easy scene management. The Background Scene Node
  is set up in such a way that when passed to the renderer it'll automatically render behind
  all visible geometry (with no camera parallax).
*/

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

