#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <unordered_map>
#include <string>
#include <optional>

enum DrawObjectShape {
	SHAPE_RECT,
	SHAPE_TRIANGLE,
	SHAPE_CIRCLE,
};

struct DrawObjectInternalData {
	unsigned int vao;
	unsigned int vbo;
	unsigned int ebo;
};
struct DrawObjectData {
	int index;
	DrawObjectShape shape;
	glm::vec2 position;
	glm::vec2 size;
	glm::vec4 color; //RGBA normalized to 1.f
	DrawObjectInternalData internal_data;
};



enum RectPointNames {
	RECT_TL,
	RECT_TR,
	RECT_BL,
	RECT_BR,
};

struct RectPoints {
	glm::vec2 top_left;
	glm::vec2 top_right;
	glm::vec2 bottom_left;
	glm::vec2 bottom_right;
};
struct RectColors {
	glm::vec4 top_left;
	glm::vec4 top_right;
	glm::vec4 bottom_left;
	glm::vec4 bottom_right;
};

struct RectVertex {
	RectPoints points;
	RectColors colors;
};



namespace draw {
	void CreateRectangle(std::string name, const glm::vec2& p_min, const glm::vec2& p_max, const glm::vec4& col);
	void CreateCircle(std::string name, const glm::vec2& p_pos, float size, const glm::vec4& col);


	std::optional<DrawObjectData*> GetObjectData(std::string name);
	void RenderAll(unsigned int shaderprogram);
	void Cleanup();
	std::unordered_map<std::string, DrawObjectData>& GetDrawData();
}

enum ShaderLayouts {
	LAYOUT_POS,
	LAYOUT_COL,
};