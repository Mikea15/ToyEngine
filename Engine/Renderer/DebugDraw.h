#pragma once

#include "Shading/Shader.h"
#include "Resources/ShaderLoader.h"

#include <glm/glm.hpp>
#include <GL/glew.h>

//
// memory is reserved to support this many lines at one time
// 1 point - 3vert, 4col
// 1 line - 2 points
// 56 bytes = 7 * 4 * 2
// 20000 lines = 20000 * 56 -> 1120000 bytes -> 1.06 Mb
#define MAX_APG_GL_DB_LINES 20000

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
// const used to convert degrees into radians

#ifndef TAU
#define TAU 2.0 * M_PI
#endif
#define ONE_DEG_IN_RAD (2.0 * M_PI) / 360.0 // 0.017444444
#define ONE_RAD_IN_DEG 360.0 / (2.0 * M_PI) //57.2957795

#define ARRAY_SIZE(d) static_cast<unsigned int>(sizeof(d) / sizeof(d[0]))

namespace DebugDraw
{
	struct Line
	{
		Line(const glm::vec3& start, const glm::vec3& end, const glm::vec4& col)
			: start(start), end(end), col(col)
		{ }

		glm::vec3 start, end;
		glm::vec4 col;
	};
	

	bool Init();
	
	void Clear();
	void Clean();

	int AddLine(const glm::vec3& start, const glm::vec3& end, const glm::vec4& col);
	int AddPersistentLine(const glm::vec3& start, const glm::vec3& end, const glm::vec4& col);
	int AddNormal(const glm::vec3& n, const glm::vec3& pos, float scale = 1.0f, const glm::vec4& col = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
	int AddPosition(const glm::vec3& pos, float scale = 1.0f, const glm::vec4& col = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	int AddRect(const glm::vec2& min, const glm::vec2& max, const glm::vec4& col = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	int AddAABB(const glm::vec3& min, const glm::vec3& max, const glm::vec4& col = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f));
	int AddFrustrum(const glm::vec3& ftl, const glm::vec3& ftr, const glm::vec3& fbl, const glm::vec3& fbr,
		const glm::vec3& ntl, const glm::vec3& ntr, const glm::vec3& nbl, const glm::vec3& nbr, const glm::vec4& col = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));


	int add_gl_db_rad_circle(float* centre_xyz, float radius, float* colour_rgba);
	bool ModifyLine(unsigned int line_id, float* start_xyz, float* end_xyz,
		float* colour_rgba);


	void Update(const glm::mat4& viewProj);
	void Draw(bool x_ray);

	unsigned int InternalDraw(float* data, unsigned int size);
}