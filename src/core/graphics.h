// Ricardas Navickas 2020
#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "scene.h"
#include "object.h"

#include <vector>
#include <string>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "../imgui/imgui.h"

#define DEFAULT_WINDOW_WIDTH 800
#define DEFAULT_WINDOW_HEIGHT 600
#define DEFAULT_WINDOW_TITLE "Mars lander"

extern struct WindowState {
	GLFWwindow* window;
	unsigned int window_width;
	unsigned int window_height;

	Scene* current_scene;
	void (*mouse_callback)(GLFWwindow* w, double x, double y);
	void (*scroll_callback)(GLFWwindow* w, double x, double y);
} wstate;

void init_graphics();

#endif
