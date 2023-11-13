#include "draw.hpp"
#include "glad/glad.h"
#include <imgui.h>

//I'm using map instead of unordered_map on purpose since I want to order them.
//like 1_Name etc.

static std::unordered_map<std::string, DrawObjectData> draw_queue;

void draw::CreateRectangle(std::string name, const glm::vec2& p_min, const glm::vec2& p_max, const glm::vec4& col)
{
	if (draw_queue.contains(name))
		return;

	const DrawObjectData draw_data = {
		draw_queue.size(),
		SHAPE_RECT,
		p_min,
		p_max,
		col / 255.f,
	};


	draw_queue.try_emplace(name, draw_data);

}
void draw::CreateCircle(std::string name, const glm::vec2& p_pos, float size, const glm::vec4& col)
{
	if (draw_queue.contains(name))
		return;

	const DrawObjectData draw_data = {
		draw_queue.size(),
		SHAPE_CIRCLE,
		p_pos,
		{size, size},
		col / 255.f,
	};


	draw_queue.try_emplace(name, draw_data);
}
std::optional<DrawObjectData*> draw::GetObjectData(std::string name)
{
	if (!draw_queue.contains(name))
		return std::nullopt;

	return &draw_queue.at(name);
}
static float normalize(float input, float max) {
	return (2.0f * input / max) - 1.0f;
}
void draw::RenderAll(unsigned int shaderprogram)
{
	int i = 0;
	for (auto& [key, val] : draw_queue)
	{
		if (val.shape == SHAPE_RECT)
		{
			auto& render_item = val.internal_data;

			const glm::vec2 normalized_pos = {
			normalize(val.position.x, ImGui::GetIO().DisplaySize.x),
			normalize(val.position.y, ImGui::GetIO().DisplaySize.y)
			};

			const glm::vec2 normalized_size = {
			(val.size.x / ImGui::GetIO().DisplaySize.x) * 2,
			(val.size.y / ImGui::GetIO().DisplaySize.y) * 2
			};

			RectPoints points;
			points.top_left = { normalized_pos.x,						-normalized_pos.y };
			points.top_right = { (normalized_pos.x + normalized_size.x), -normalized_pos.y };
			points.bottom_left = { normalized_pos.x,						-normalized_pos.y - normalized_size.y };
			points.bottom_right = { (normalized_pos.x + normalized_size.x),	-normalized_pos.y - normalized_size.y };

			RectColors colors;
			colors.top_left = val.color;
			colors.top_right = val.color;
			colors.bottom_left = val.color;
			colors.bottom_right = val.color;

			RectVertex vertices = {
				points,
				colors
			};

			int indices[] = {
				RECT_TL,
				RECT_TR,
				RECT_BR,

				RECT_TL,
				RECT_BL,
				RECT_BR,
			};


			//Vertex Array Object -> It will store vertices, indices in a container
			if (!render_item.vao)
				glGenVertexArrays(1, &render_item.vao);

			glBindVertexArray(render_item.vao);

			//Vertex Buffer Object -> It willl store vertices
			if (!render_item.vbo)
				glGenBuffers(1, &render_item.vbo);
			glBindBuffer(GL_ARRAY_BUFFER, render_item.vbo);


			glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_DYNAMIC_DRAW);



			/*EBO*/
			if (!render_item.ebo)
				glGenBuffers(1, &render_item.ebo);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, render_item.ebo);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_DYNAMIC_DRAW);

			//Shader Inputs
			glVertexAttribPointer(LAYOUT_POS, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(0)); // specifying offset of X and Y
			glEnableVertexAttribArray(0);



			glVertexAttribPointer(LAYOUT_COL, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(8 * sizeof(float))); // specifying offset of R G B A (there are 4 Vector2 so it means 8 float.)
			glEnableVertexAttribArray(1);


			glUseProgram(shaderprogram);
				
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

			glBindBuffer(GL_ARRAY_BUFFER, 0);
		}
#if 0
		else if (val.shape == SHAPE_CIRCLE) {
			auto& render_item = val.internal_data;

			const glm::vec2 normalized_pos = {
			normalize(val.position.x, ImGui::GetIO().DisplaySize.x),
			normalize(val.position.y, ImGui::GetIO().DisplaySize.y)
			};

			const glm::vec2 normalized_size = {
			(val.size.x / ImGui::GetIO().DisplaySize.x) * 2,
			(val.size.y / ImGui::GetIO().DisplaySize.y) * 2
			};

			std::vector< glm::vec2 > positions;
			std::vector< glm::vec4 > colors;
			std::vector< int > indices;

			float angle = 360.0f / 64;

			int triangleCount = 64 - 2;

			std::vector<glm::vec3> temp;
			// positions
			for (int i = 0; i < 64; i++)
			{
				float currentAngle = angle * i;
				float x = normalized_size.x * cos(glm::radians(currentAngle));
				float y = normalized_size.x * sin(glm::radians(currentAngle));
				float z = 0.0f;

				positions.push_back(glm::vec2(x, y));
			}

			for (int i = 0; i < 64; i++)
			{
				float currentAngle = angle * i;
				float x = normalized_size.x * cos(glm::radians(currentAngle));
				float y = normalized_size.x * sin(glm::radians(currentAngle));
				float z = 0.0f;

				colors.push_back(val.color);
			}

			// push indexes of each triangle points
			for (int i = 0; i < triangleCount; i++)
			{
				indices.push_back(0);
				indices.push_back(i + 1);
				indices.push_back(i + 2);
			}


			//Vertex Array Object -> It will store vertices, indices in a container
			if (!render_item.vao)
				glGenVertexArrays(1, &render_item.vao);

			glBindVertexArray(render_item.vao);

			//Vertex Buffer Object -> It willl store vertices
			if (!render_item.vbo)
				glGenBuffers(1, &render_item.vbo);
			glBindBuffer(GL_ARRAY_BUFFER, render_item.vbo);


			glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_DYNAMIC_DRAW);



			/*EBO*/
			if (!render_item.ebo)
				glGenBuffers(1, &render_item.ebo);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, render_item.ebo);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_DYNAMIC_DRAW);

			//Shader Inputs
			glVertexAttribPointer(LAYOUT_POS, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(0)); // specifying offset of X and Y
			glEnableVertexAttribArray(0);



			glVertexAttribPointer(LAYOUT_COL, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(8 * sizeof(float))); // specifying offset of R G B A (there are 4 Vector2 so it means 8 float.)
			glEnableVertexAttribArray(1);


			glUseProgram(shaderprogram);

			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

			glBindBuffer(GL_ARRAY_BUFFER, 0);
		}
#endif
	}
}
void draw::Cleanup()
{
	for (auto& [key, val] : draw_queue)
	{
		auto& render_item = val.internal_data;

		glDeleteVertexArrays(1, &render_item.vao);
		glDeleteBuffers(1, &render_item.vbo);
		glDeleteBuffers(1, &render_item.ebo);
	}
}

std::unordered_map<std::string, DrawObjectData>& draw::GetDrawData()
{
	return draw_queue;
}
