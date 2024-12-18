#pragma once
#include "RenderCommand.h"

#include <map>
#include <vector>


class Renderer;
class Mesh;
class Material;
class RenderTarget;

class CommandBuffer
{
public:

private:
	Renderer* m_Renderer;

	std::vector<RenderCommand> m_DeferredRenderCommands;
	std::vector<RenderCommand> m_AlphaRenderCommands;
	std::vector<RenderCommand> m_PostProcessingRenderCommands;
	std::map<RenderTarget*, std::vector<RenderCommand>> m_CustomRenderCommands;


public:
	CommandBuffer(Renderer* renderer);
	~CommandBuffer();

	// pushes render state relevant to a single render call to the command buffer.
	void Push(Mesh* mesh, Material* material, glm::mat4 transform = glm::mat4(), glm::mat4 prevTransform = glm::mat4(), glm::vec3 boxMin = glm::vec3(-99999.0f), glm::vec3 boxMax = glm::vec3(99999.0f), RenderTarget* target = nullptr);

	// clears the command buffer; usually done after issuing all the stored render commands.
	void Clear();

	// sorts the command buffer; first by shader, then by texture bind.
	void Sort();

	// returns the list of render commands. For minimizing state changes it is advised to first 
	// call Sort() before retrieving and issuing the render commands.
	std::vector<RenderCommand> GetDeferredRenderCommands(bool cull = false);

	// returns the list of render commands of both deferred and forward pushes that require 
	// alpha blending; which have to be rendered last. 
	std::vector<RenderCommand> GetAlphaRenderCommands(bool cull = false);

	// returns the list of custom render commands per render target.
	std::vector<RenderCommand> GetCustomRenderCommands(RenderTarget* target, bool cull = false);

	// returns the list of post-processing render commands.
	std::vector<RenderCommand> GetPostProcessingRenderCommands();

	// returns the list of all render commands with mesh shadow casting
	std::vector<RenderCommand> GetShadowCastRenderCommands();
};


