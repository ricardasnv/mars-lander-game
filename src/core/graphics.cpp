// Ricardas Navickas 2020
#include "graphics.h"
#include "error.h"

#include <iostream>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

// ======== Compilation unit specific declarations ========
static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
static void mouse_callback_dispatcher(GLFWwindow* window, double xpos, double ypos);
static void scroll_callback_dispatcher(GLFWwindow* window, double xoffset, double yoffset);

// ======== Declared in header ========
WindowState wstate;

void init_graphics() {
	wstate.window = NULL;
	wstate.window_width = DEFAULT_WINDOW_WIDTH;
	wstate.window_height = DEFAULT_WINDOW_HEIGHT;
	wstate.current_scene = NULL;
	wstate.mouse_callback = NULL;
	wstate.scroll_callback = NULL;

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	wstate.window = glfwCreateWindow(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT, DEFAULT_WINDOW_TITLE, NULL, NULL);

	if (wstate.window == NULL) {
		glfwTerminate();
		fatal("init()", "Failed to create GLFW window.");
	}

	glfwMakeContextCurrent(wstate.window);

	glfwSetFramebufferSizeCallback(wstate.window, framebuffer_size_callback);
	glfwSetCursorPosCallback(wstate.window, mouse_callback_dispatcher);
	glfwSetScrollCallback(wstate.window, scroll_callback_dispatcher);

	// Enable depth testing
	glEnable(GL_DEPTH_TEST);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glfwSetInputMode(wstate.window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	glfwSwapInterval(0);

	// Enable transparency
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Enable MSAA antialiasing
	glfwWindowHint(GLFW_SAMPLES, 4);
	glEnable(GL_MULTISAMPLE);

	// set up OpenGL function pointers
	glewInit(); 

	debug("init_graphics()", "Initialized graphics.");
}

// ======== Compilation unit specific definitions ========
static void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
	wstate.window_width = width;
	wstate.window_height = height;
}

static void mouse_callback_dispatcher(GLFWwindow* window, double xpos, double ypos) {
	if (ImGui::GetIO().WantCaptureMouse == 1)
		return;

	if (wstate.mouse_callback == NULL)
		fatal("mouse_callback_dispatcher()", "wstate.mouse_callback is NULL!");

	wstate.mouse_callback(window, xpos, ypos);
}

static void scroll_callback_dispatcher(GLFWwindow* window, double xoffset, double yoffset) {
	if (ImGui::GetIO().WantCaptureMouse == 1)
		return;
	
	if (wstate.scroll_callback == NULL)
		fatal("scroll_callback_dispatcher()", "wstate.scroll_callback is NULL!");
	
	wstate.scroll_callback(window, xoffset, yoffset);
}

