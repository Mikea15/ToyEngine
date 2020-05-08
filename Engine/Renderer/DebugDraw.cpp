
#include "DebugDraw.h"

#include "Utils/Logger.h"

#include "Engine/Vector.h"
#include <GL/glew.h>

#include <vector>


namespace DebugDraw
{
	GLuint m_linesVAO;
	GLuint m_linesVBO;
	GLuint m_linesShader;

	GLint m_viewProjecLoc = -1;

	size_t m_linesAdded = 0;
	std::vector<Line> m_lines;
	std::vector<float> m_scratchPadLineData;

	void _print_shader_info_log(unsigned int shader_index) {
		int max_length = 2048;
		int actual_length = 0;
		char log[2048];
		glGetShaderInfoLog(shader_index, max_length, &actual_length, log);
		printf("shader info log for GL index %u\n%s\n", shader_index, log);
	}

	void _print_programme_info_log(unsigned int m_programme_idx) {
		int max_length = 2048;
		int actual_length = 0;
		char log[2048];
		glGetProgramInfoLog(m_programme_idx, max_length, &actual_length, log);
		printf("program info log for GL index %u\n%s\n", (int)m_programme_idx, log);
	}

	const char* vertexShader =
		R"foo(
		#version 330
		layout (location = 0) in vec3 pos;
		layout (location = 1) in vec4 color;
		uniform mat4 viewProj;
		out vec4 Color;
		void main () {
			gl_Position = viewProj * vec4 (pos, 1.0);
			Color = color;
		}
	)foo";

	const char* fragmentShader =
		R"foo(#version 330
		out vec4 FragColor;
		in vec4 Color;
		void main () {
			FragColor = Color;
		}
	)foo";



	//
	// reserve memory for drawing stuff
	bool Init() {
		// vao for drawing properties of lines
		glGenVertexArrays(1, &m_linesVAO);
		glBindVertexArray(m_linesVAO);

		// create GPU-side buffer
		// size is 32-bits for GLfloat * num lines * 7 comps per vert * 2 per lines
		glGenBuffers(1, &m_linesVBO);
		glBindBuffer(GL_ARRAY_BUFFER, m_linesVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * MAX_APG_GL_DB_LINES * 14, NULL,
			GL_DYNAMIC_DRAW);

		GLsizei stride = sizeof(float) * 7;
		GLintptr offs = sizeof(float) * 3;
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, NULL); // point
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, stride, (GLvoid*)offs); // colour
		glEnableVertexAttribArray(0); // point
		glEnableVertexAttribArray(1); // colour

		GLuint vs = glCreateShader(GL_VERTEX_SHADER);
		GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(vs, 1, (const char**)&vertexShader, NULL);
		glCompileShader(vs);
		_print_shader_info_log(vs);
		glShaderSource(fs, 1, (const char**)&fragmentShader, NULL);
		glCompileShader(fs);
		_print_shader_info_log(fs);
		m_linesShader = glCreateProgram();
		glAttachShader(m_linesShader, fs);
		glAttachShader(m_linesShader, vs);
		glBindAttribLocation(m_linesShader, 0, "pos");
		glBindAttribLocation(m_linesShader, 1, "color");
		glLinkProgram(m_linesShader);
		_print_programme_info_log(m_linesShader);
		m_viewProjecLoc = glGetUniformLocation(m_linesShader, "viewProj");
		assert(m_viewProjecLoc >= -1);
		// flag that shaders can be deleted whenever the program is deleted
		glDeleteShader(fs);
		glDeleteShader(vs);
		float PV[16];
		memset(PV, 0, 16 * sizeof(float));
		PV[0] = PV[5] = PV[10] = PV[15] = 1.0f;
		glUseProgram(m_linesShader);
		glUniformMatrix4fv(m_viewProjecLoc, 1, GL_FALSE, PV);

		m_scratchPadLineData.resize(MAX_APG_GL_DB_LINES * 14); // 14 floats per line.
		m_lines.resize(MAX_APG_GL_DB_LINES);

		return true;
	}

	void Clear()
	{
		m_linesAdded = 0;
		// m_lines.clear();
	}

	//
	// free memory
	void Clean() {
		glDeleteBuffers(1, &m_linesVBO);
		glDeleteVertexArrays(1, &m_linesVAO);
		// attached shaders have prev been flagged to delete so will also be deleted
		glDeleteProgram(m_linesShader);
	}

	//
	// add a new debug line to world space coords
	// params are 2 arrays of 3 floats
	// return index corresponding to internal memory offset
	int AddLine(const glm::vec3& start, const glm::vec3& end, const glm::vec4& col) {
		if (m_linesAdded >= MAX_APG_GL_DB_LINES)
		//if (m_lines.size() >= MAX_APG_GL_DB_LINES) 
		{
			// LOG_WARNING("DebugDraw: Can't add more lines to draw");
			return -1;
		}

		m_lines[m_linesAdded++] = { start, end, col };
		return m_linesAdded;
		// m_lines.push_back({ start, end, col });
		// return static_cast<int>(m_lines.size());
	}

	int AddNormal(const glm::vec3& n, const glm::vec3& pos, float scale, const glm::vec4& col)
	{
		return AddLine(pos, pos + n * scale, col);
	}

	int AddPosition(const glm::vec3& pos, float scale, const glm::vec4& col)
	{
		const glm::vec3 up(0.0f, 1.0f, 0.0f);
		const glm::vec3 right(1.0f, 0.0f, 0.0f);
		const glm::vec3 forward(0.0f, 0.0f, 1.0f);

		int id = AddLine(pos - up * scale, pos + up * scale, col);
		AddLine(pos - right * scale, pos + right * scale, col);
		AddLine(pos - forward * scale, pos + forward * scale, col);

		return id;
	}

	int AddRect(const glm::vec2& min, const glm::vec2& max, const glm::vec4& col)
	{
		glm::vec2 bfl = min;
		glm::vec2 bfr = min; bfr.x = max.x;
		glm::vec2 bbr = min; bbr.x = max.x; bbr.y = max.y;
		glm::vec2 bbl = min; bbl.y = max.y;

		int id = AddLine( { bfl.x, 0.0f, bfl.y }, { bfr.x, 0.0f, bfr.y }, col);
				 AddLine( { bbl.x, 0.0f, bbl.y }, { bbr.x, 0.0f, bbr.y }, col);
				 AddLine( { bfl.x, 0.0f, bfl.y }, { bbl.x, 0.0f, bbl.y }, col);
				 AddLine( { bfr.x, 0.0f, bfr.y }, { bbr.x, 0.0f, bbr.y }, col);

		return id;
	}

	int AddRect(const glm::vec3& pos, float extent, const glm::vec3& dir, const glm::vec4& col)
	{
		// Calculate change-of-basis matrix
		glm::mat3 transform(1.0f);
		if (dir.x == 0 && dir.z == 0)
		{
			if (dir.y < 0) { // rotate 180 degrees
				transform = glm::mat3(glm::vec3(-1.0f, 0.0f, 0.0f),
					glm::vec3(0.0f, -1.0f, 0.0f),
					glm::vec3(0.0f, 0.0f, 1.0f));
			}
			// else if direction.y >= 0, leave transform as the identity matrix.
		}
		else
		{
			const glm::vec3 worldUp = { 0.0f, 1.0f, 0.0f };
			glm::vec3 new_y = glm::normalize(dir);
			glm::vec3 new_z = glm::normalize(glm::cross(new_y, worldUp));
			glm::vec3 new_x = glm::normalize(glm::cross(new_y, new_z));

			transform = glm::mat3(new_x, new_y, new_z);
		}

		const glm::vec3 bfl = transform * (pos + glm::vec3(-1.0f, 0.0f, -1.0f) * extent);
		const glm::vec3 bfr = transform * (pos + glm::vec3(1.0f, 0.0f, -1.0f) * extent);
		const glm::vec3 bbl = transform * (pos + glm::vec3(-1.0f, 0.0f, 1.0f) * extent);
		const glm::vec3 bbr = transform * (pos + glm::vec3(1.0f, 0.0f, 1.0f) * extent);

		int id = AddLine(bfl, bfr, col);
				 AddLine(bbl, bbr, col);
				 AddLine(bfl, bbl, col);
				 AddLine(bfr, bbr, col);

		return id;
	}

	int AddAABB(const glm::vec3& min, const glm::vec3& max, const glm::vec4& col)
	{
		glm::vec3 bfl = min;
		glm::vec3 bfr = min; bfr.x = max.x;
		glm::vec3 bbr = min; bbr.x = max.x; bbr.z = max.z;
		glm::vec3 bbl = min; bbl.z = max.z;

		glm::vec3 tbr = max;
		glm::vec3 tbl = max; tbl.x = min.x;
		glm::vec3 tfl = max; tfl.x = min.x; tfl.z = min.z;
		glm::vec3 tfr = max; tfr.z = min.z;

		int id = AddLine(bfl, bfr, col);
		AddLine(bbl, bbr, col);
		AddLine(bfl, bbl, col);
		AddLine(bfr, bbr, col);

		AddLine(tfl, tfr, col);
		AddLine(tbl, tbr, col);
		AddLine(tfl, tbl, col);
		AddLine(tfr, tbr, col);

		AddLine(tfl, bfl, col);
		AddLine(tbl, bbl, col);
		AddLine(tfr, bfr, col);
		AddLine(tbr, bbr, col);
		
		return id;
	}


	int AddFrustrum(const glm::vec3& ftl, const glm::vec3& ftr, const glm::vec3& fbl, const glm::vec3& fbr, 
		const glm::vec3& ntl, const glm::vec3& ntr, const glm::vec3& nbl, const glm::vec3& nbr, const glm::vec4& col)
	{
		int id = AddLine(fbl, fbr, col);
		AddLine(ftl, ftr, col);
		AddLine(ftl, fbl, col);
		AddLine(ftr, fbr, col);

		AddLine(nbl, nbr, col);
		AddLine(ntl, ntr, col);
		AddLine(ntl, nbl, col);
		AddLine(ntr, nbr, col);

		AddLine(nbl, fbl, col);
		AddLine(nbr, fbr, col);
		AddLine(ntl, ftl, col);
		AddLine(ntr, ftr, col);

		return id;
	}
	
	/*
	//
	// draw a circle+radius to represent a sphere
	// returns first of 39 line ids
	int add_gl_db_rad_circle(float* centre_xyz, float radius,
		float* colour_rgba) {
		int rid = -1;
		float start[3], end[3];
		// 3 radius lines in a cross first
		start[0] = centre_xyz[0];
		start[1] = centre_xyz[1] - radius;
		start[2] = centre_xyz[2];
		end[0] = centre_xyz[0];
		end[1] = centre_xyz[1] + radius;
		end[2] = centre_xyz[2];
		rid = AddLine(start, end, colour_rgba);
		start[0] = centre_xyz[0] - radius;
		start[1] = centre_xyz[1];
		start[2] = centre_xyz[2];
		end[0] = centre_xyz[0] + radius;
		end[1] = centre_xyz[1];
		end[2] = centre_xyz[2];
		AddLine(start, end, colour_rgba);
		start[0] = centre_xyz[0];
		start[1] = centre_xyz[1];
		start[2] = centre_xyz[2] - radius;
		end[0] = centre_xyz[0];
		end[1] = centre_xyz[1];
		end[2] = centre_xyz[2] + radius;
		AddLine(start, end, colour_rgba);
		// circles of 12 segments
		int segs = 12;
		// x,y around z loop
		for (int i = 0; i < segs; i++) {
			start[0] = centre_xyz[0] + radius * cos(
				2.0f * M_PI * (float)i / (float)segs);
			start[1] = centre_xyz[1] + radius * sin(
				2.0f * M_PI * (float)i / (float)segs);
			start[2] = centre_xyz[2];
			end[0] = centre_xyz[0] + radius * cos(
				2.0f * M_PI * (float)(i + 1) / (float)segs);
			end[1] = centre_xyz[1] + radius * sin(
				2.0f * M_PI * (float)(i + 1) / (float)segs);
			end[2] = centre_xyz[2];
			AddLine(start, end, colour_rgba);
		}
		// x,z around y loop
		for (int i = 0; i < segs; i++) {
			start[0] = centre_xyz[0] + radius * cos(
				2.0f * M_PI * (float)i / (float)segs);
			start[1] = centre_xyz[1];
			start[2] = centre_xyz[2] + radius * sin(
				2.0f * M_PI * (float)i / (float)segs);
			end[0] = centre_xyz[0] + radius * cos(
				2.0f * M_PI * (float)(i + 1) / (float)segs);
			end[1] = centre_xyz[1];
			end[2] = centre_xyz[2] + radius * sin(
				2.0f * M_PI * (float)(i + 1) / (float)segs);
			AddLine(start, end, colour_rgba);
		}
		// y,z around xloop
		for (int i = 0; i < segs; i++) {
			start[0] = centre_xyz[0];
			start[1] = centre_xyz[1] + radius * cos(
				2.0f * M_PI * (float)i / (float)segs);
			start[2] = centre_xyz[2] + radius * sin(
				2.0f * M_PI * (float)i / (float)segs);
			end[0] = centre_xyz[0];
			end[1] = centre_xyz[1] + radius * cos(
				2.0f * M_PI * (float)(i + 1) / (float)segs);
			end[2] = centre_xyz[2] + radius * sin(
				2.0f * M_PI * (float)(i + 1) / (float)segs);
			AddLine(start, end, colour_rgba);
		}

		return rid;
	}



	//
	// modify or move a line previously added
	// returns false if line_id wasn't right
	bool ModifyLine(unsigned int line_id, float* start_xyz, float* end_xyz,
		float* colour_rgba) {
		if (line_id >= g_count_gl_db_lines) {
			fprintf(stderr, "ERROR: modifying gl db line - bad ID\n");
			return false;
		}

		float sd[14];
		sd[0] = start_xyz[0];
		sd[1] = start_xyz[1];
		sd[2] = start_xyz[2];
		sd[3] = colour_rgba[0];
		sd[4] = colour_rgba[1];
		sd[5] = colour_rgba[2];
		sd[6] = colour_rgba[3];
		sd[7] = end_xyz[0];
		sd[8] = end_xyz[1];
		sd[9] = end_xyz[2];
		sd[10] = colour_rgba[0];
		sd[11] = colour_rgba[1];
		sd[12] = colour_rgba[2];
		sd[13] = colour_rgba[3];

		glBindBuffer(GL_ARRAY_BUFFER, m_linesVBO);
		GLintptr os = sizeof(sd) * g_count_gl_db_lines;
		GLsizei sz = sizeof(sd);
		glBufferSubData(GL_ARRAY_BUFFER, os, sz, sd);

		return true;
	}
	*/
	//
	// update the camera matrix so that line points given are defined as being in
	// world coord space
	// matrix is a 16 float column-major matrix as 1d array in column order
	void Update(const glm::mat4& viewProj) {
		glUseProgram(m_linesShader);
		glUniformMatrix4fv(m_viewProjecLoc, 1, GL_FALSE, &viewProj[0][0]);
	}

	void Draw(bool x_ray) 
	{
		// m_scratchPadLineData.clear();

		unsigned int floatIndex = 0;
		// const unsigned int lineCount = m_lines.size();
		const unsigned int lineCount = m_linesAdded;
		for (unsigned int i = 0; i < lineCount; ++i)
		{
			auto& l = m_lines[i];
			m_scratchPadLineData[floatIndex++] = l.start.x;
			m_scratchPadLineData[floatIndex++] = l.start.y;
			m_scratchPadLineData[floatIndex++] = l.start.z;
			m_scratchPadLineData[floatIndex++] = l.col.x;
			m_scratchPadLineData[floatIndex++] = l.col.y;
			m_scratchPadLineData[floatIndex++] = l.col.z;
			m_scratchPadLineData[floatIndex++] = l.col.a;
			m_scratchPadLineData[floatIndex++] = l.end.x;
			m_scratchPadLineData[floatIndex++] = l.end.y;
			m_scratchPadLineData[floatIndex++] = l.end.z;
			m_scratchPadLineData[floatIndex++] = l.col.x;
			m_scratchPadLineData[floatIndex++] = l.col.y;
			m_scratchPadLineData[floatIndex++] = l.col.z;
			m_scratchPadLineData[floatIndex++] = l.col.a;
		}

		GLintptr offset = sizeof(float) * m_scratchPadLineData.size() * 0;
		GLsizei size = sizeof(float) * m_scratchPadLineData.size();
		glBindBuffer(GL_ARRAY_BUFFER, m_linesVBO);
		glBufferSubData(GL_ARRAY_BUFFER, offset, size, &m_scratchPadLineData[0]);

		GLboolean dwe = false;
		glGetBooleanv(GL_DEPTH_TEST, &dwe);
		if (dwe && x_ray) {
			glDisable(GL_DEPTH_TEST);
		}
		else if (!dwe && !x_ray) {
			glEnable(GL_DEPTH_TEST);
		}

		GLboolean blendBool = false;
		glGetBooleanv(GL_BLEND, &blendBool);
		
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glUseProgram(m_linesShader);
		glBindVertexArray(m_linesVAO);
		glDrawArrays(GL_LINES, 0, lineCount * 2);

		if( !blendBool )
		{
			glDisable(GL_BLEND);
		}

		if (dwe && x_ray) {
			glEnable(GL_DEPTH_TEST);
		}
		else if (!dwe && !x_ray) {
			glDisable(GL_DEPTH_TEST);
		}
	}
}


