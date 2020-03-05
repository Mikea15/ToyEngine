#pragma once

#include <GL/glew.h>
#include <stack>

class GLStateCache
{
public:
	void SetDepthTest(bool enable);
	void SetDepthFunc(GLenum depthFunc);
	void SetBlend(bool enable);
	void SetBlendFunc(GLenum src, GLenum dst);
	void SetCull(bool enable);
	void SetCullFace(GLenum face);
	void SetPolygonMode(GLenum mode);

	void SwitchShader(unsigned int ID);

	void PushState(GLenum state);
	void PopState();

private:
	void ToggleState(GLenum state, bool enable);

private:
	// toggles
	bool m_depthTest;
	bool m_blend;
	bool m_cullFace;

	// state
	GLenum m_depthFunc;
	GLenum m_blendSrc;
	GLenum m_blendDst;
	GLenum m_frontFace;
	GLenum m_polygonMode;

	unsigned int m_activeShaderID;

	std::stack<GLenum> m_stateStack;
};

