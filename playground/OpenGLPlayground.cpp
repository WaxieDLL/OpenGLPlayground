#define IMGUI_DEFINE_MATH_OPERATORS
#include <iostream>
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>

#include<fstream>
#include<sstream>
#include <glm/glm.hpp>
#include "utils/draw.hpp"
bool InitGLFW();
bool InitGLAD();
GLFWwindow* InitWindow();
bool SetupImGuiContext(GLFWwindow* window);
void ImGuiProcess();
void GUIOverlay();
void ProcessInput();
void framebuffer_size_callback(GLFWwindow* window, int width, int height);

void ProcessShader(unsigned int& shader, std::string file, int shader_type);
void ProcessShaderProgram(unsigned int& program,
	std::vector<unsigned int*> shaders_to_attach,
	bool should_link = true,
	bool delete_shaders_after_attach = true
);

std::string ReadFromFile(std::string file);


GLFWwindow* g_Window;


static void glfw_error_callback(int error, const char* description)
{
	fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

constexpr const char* WINDOW_NAME = "OpenGL Playground";
constexpr bool ENABLE_VSYNC = false;
constexpr float WINDOW_WIDTH = 1280;
constexpr float WINDOW_HEIGHT = 720;
const char* glsl_version = "#version 130";



int main()
{
	if (!InitGLFW())
		return -1;

	if (!InitGLAD())
		return -1;

	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

	glfwSetFramebufferSizeCallback(g_Window, framebuffer_size_callback);

	spdlog::info("OpenGL Version: {}", (const char*)(glGetString(GL_VERSION)));

	glfwSwapInterval(ENABLE_VSYNC);

	SetupImGuiContext(g_Window);

	ImVec4 clear_color = ImVec4(0, 0, 0, 1.f);

	glEnable(GL_BLEND);
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO); //for transparency of shapes

	unsigned int rect_vertex_shader;
	unsigned int rect_fragment_shader;
	ProcessShader(rect_vertex_shader, "shaders/rect_vertex.glsl", GL_VERTEX_SHADER);
	ProcessShader(rect_fragment_shader, "shaders/rect_fragment.glsl", GL_FRAGMENT_SHADER);

	unsigned int shaderprogram1;
	ProcessShaderProgram(shaderprogram1, { &rect_vertex_shader, &rect_fragment_shader });


	draw::CreateRectangle("1rect", { 100, 100 }, { 100,100 }, { 255,100,100,255 });
	draw::CreateRectangle("2rect", { 400, 400 }, { 100,100 }, { 255,255,255,255 });

	while (!glfwWindowShouldClose(g_Window))
	{
		int display_w, display_h;
		glfwGetFramebufferSize(g_Window, &display_w, &display_h);
		const float ratio = display_w / (float)display_h;
		ProcessInput();

		ImGuiProcess();

		glViewport(0, 0, display_w, display_h);
		glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
		glClear(GL_COLOR_BUFFER_BIT);

		draw::RenderAll(shaderprogram1);

		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData()); //render imgui draw data
		glfwSwapBuffers(g_Window); //new page!
		glfwPollEvents();
	}

	// Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	draw::Cleanup();

	glDeleteProgram(shaderprogram1);

	glfwDestroyWindow(g_Window);
	glfwTerminate();



	return 0;
}

static bool InitGLFW()
{
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	glfwSetErrorCallback(glfw_error_callback);
	if (!glfwInit())
	{
		spdlog::error("GLFW Initialization Failed (glfwInit)");
		glfwTerminate();
		return false;
	}

	spdlog::info("GLFW Initialized");

	g_Window = InitWindow();
	if (!g_Window)
		return false;

	glfwMakeContextCurrent(g_Window);

	return true;
}


bool InitGLAD()
{
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		spdlog::error("Unable to create window (glfwCreateWindow)");
		return false;
	}
	spdlog::info("GLAD Initialized");
	return true;
}


static GLFWwindow* InitWindow()
{
	GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_NAME, nullptr, nullptr);
	if (window == nullptr)
	{
		spdlog::error("Unable to create window (glfwCreateWindow)");
		glfwTerminate();
		return nullptr;
	}

	spdlog::info("Window Created");
	return window;
}
static bool SetupImGuiContext(GLFWwindow* window)
{
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);

	return true;
}
static void ImGuiProcess()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	GUIOverlay();

	ImGui::Render(); //processes imgui draw data
}

static float normalize(float input, float max) {
	return (2.0f * input / max) - 1.0f;
}

static void GUIOverlay()
{
	ImGui::Begin("Window");
	static bool wireframe = false;

	if (ImGui::Checkbox("Wireframe Mode", &wireframe))
	{
		if (wireframe)
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		else
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	static std::string selected_obj_name = "Select Object##Combo_Identifier";

	if (ImGui::BeginListBox("Object")) {
		for (auto& [key, val] : draw::GetDrawData())
		{
	
			const bool is_selected = (selected_obj_name == key);
			if (ImGui::Selectable(key.c_str(), is_selected))
				selected_obj_name = key;

			if (is_selected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndListBox();
	}


	if (draw::GetObjectData(selected_obj_name).has_value())
	{
		auto data = draw::GetObjectData(selected_obj_name).value();

		if (data->shape == SHAPE_RECT)
		{
			ImGui::PushItemWidth(ImGui::GetWindowSize().x / 2 - (ImGui::GetStyle().WindowPadding.x + ImGui::GetStyle().ItemSpacing.x / 2));
			ImGui::SliderFloat("##X", &data->position.x, 0.f, ImGui::GetIO().DisplaySize.x, "X: %.3f");
			ImGui::SameLine();
			ImGui::SliderFloat("##Y", &data->position.y, 0.f, ImGui::GetIO().DisplaySize.y, "Y: %.3f");

			ImGui::SliderFloat("##Width", &data->size.x, 0.f, ImGui::GetIO().DisplaySize.x, "WIDTH: %.3f");
			ImGui::SameLine();
			ImGui::SliderFloat("##Height", &data->size.y, 0.f, ImGui::GetIO().DisplaySize.y, "HEIGHT: %.3f");

			ImGui::PopItemWidth();

			float cols[4] = {
				data->color.r,
				data->color.g,
				data->color.b,
				data->color.a,
			};
			ImGui::ColorEdit4("##ColEditRect", cols);
			data->color.r = cols[0];
			data->color.g = cols[1];
			data->color.b = cols[2];
			data->color.a = cols[3];
		}
	}

	ImGui::End();
}

static void ProcessInput()
{
	if (glfwGetKey(g_Window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(g_Window, true);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

std::string ReadFromFile(std::string file) {
	std::ifstream f(file.c_str()); //taking file as inputstream
	std::string str;
	if (f) {
		std::ostringstream ss;
		ss << f.rdbuf(); // reading data
		str = ss.str();
	}
	return str;
}

void ProcessShader(unsigned int& shader, std::string file, int shader_type) {
	std::string vertexShaderSource = ReadFromFile(file);
	const char* vertexShaderSource2 = vertexShaderSource.c_str();
	shader = glCreateShader(shader_type);
	glShaderSource(shader, 1, &vertexShaderSource2, NULL);
	glCompileShader(shader);

	int  success;
	char infoLog[512];
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

	if (!success)
	{
		glGetShaderInfoLog(shader, 512, NULL, infoLog);
		spdlog::error("Unable to compile shader ({})", infoLog);
	}
}
void ProcessShaderProgram(unsigned int& program, std::vector<unsigned int*> shaders_to_attach, bool should_link, bool delete_shaders_after_attach) {
	program = glCreateProgram();
	for (auto& shader : shaders_to_attach)
	{
		glAttachShader(program, *shader);
	}

	if (should_link)
		glLinkProgram(program);

	int  success;
	char infoLog[512];
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(program, 512, NULL, infoLog);
		spdlog::error("Unable to process shader program ({})", infoLog);
	}

	if (delete_shaders_after_attach)
	{
		for (auto& shader : shaders_to_attach)
		{
			glDeleteShader(*shader);
		}
	}

}