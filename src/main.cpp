// Ricardas Navickas 2020
#include <iostream>
#include <cmath>
#include <vector>
#include <thread>
#include <chrono>

#include "core/core.h"
#include "physics.h"
#include "global.h"
#include "gui.h"
#include "simulation.h"
#include "closeup_scene.h"
#include "orbit_scene.h"
#include "autopilot.h"

#include <fenv.h>

#define FPS_MAX 60

static void init_everything();
static void process_input(GLFWwindow* window);
static void do_frame();
static void do_rendering();
static void do_simulation();

int main(int argc, char** argv) {
	debug("main()", "Starting...");
	init_everything();

	while (!glfwWindowShouldClose(wstate.window)) {
		do_frame();
	}

	glfwTerminate();
	debug("main()", "Quitting.");

	return 0;
}

static void init_everything() {
	// NOTE: init_graphics() must be called before anything else to set up all OpenGL function pointers
	init_graphics();
	init_global_vars();
	init_gui();
	init_simulation(1.0 / FPS_MAX);
	init_orbit_scene(world_shader, world_nofx_shader);
	init_closeup_scene(world_shader, world_nofx_shader);

	// Set closeup_scene as the initial scene
	activate_closeup_scene();
}

static void process_input(GLFWwindow* window) {
	if (ImGui::GetIO().WantCaptureKeyboard == 1) return;
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	static auto up_prev_status = glfwGetKey(window, GLFW_KEY_UP);
	static auto down_prev_status = glfwGetKey(window, GLFW_KEY_DOWN);

	// Control simulation speed with up/down arrow keys
	if (glfwGetKey(window, GLFW_KEY_UP) != up_prev_status && up_prev_status != GLFW_PRESS) {
		guistate.physics_updates_per_frame = glm::clamp(guistate.physics_updates_per_frame * 2.0f, 0.25f, 2048.0f);
	} if (glfwGetKey(window, GLFW_KEY_DOWN) != down_prev_status && down_prev_status != GLFW_PRESS) {
		guistate.physics_updates_per_frame = glm::clamp(guistate.physics_updates_per_frame * 0.5f, 0.25f, 2048.0f);
	}

	up_prev_status = glfwGetKey(window, GLFW_KEY_UP);
	down_prev_status = glfwGetKey(window, GLFW_KEY_DOWN);
}

static void do_frame() {
	// Timings
	float frame_begin_time, render_begin_time, sim_begin_time, fc_begin_time, other_begin_time;

	frame_begin_time = glfwGetTime();

	// ======== RENDERING PHASE ========
	render_begin_time = glfwGetTime();
	do_rendering();
	guistate.render_duration = glfwGetTime() - render_begin_time;

	// ======== SIMULATION PHASE ========
	sim_begin_time = glfwGetTime();
	//feenableexcept(FE_ALL_EXCEPT ^ FE_INEXACT);
	do_simulation();
	feenableexcept(0);
	guistate.sim_duration = glfwGetTime() - sim_begin_time;

	// ======== FRAMERATE CONTROL ========
	fc_begin_time = glfwGetTime();
	std::this_thread::sleep_for(std::chrono::milliseconds(1000 / FPS_MAX - (int)((glfwGetTime() - frame_begin_time) * 1000)));
	guistate.fc_duration = glfwGetTime() - fc_begin_time;

	// ======== OTHER ========
	other_begin_time = glfwGetTime();

	process_input(wstate.window);
	update_closeup_scene();
	update_orbit_scene();

	guistate.sim_time = simstate.time;
	guistate.sim_timestep = simstate.timestep;

	guistate.other_duration = glfwGetTime() - other_begin_time;
	guistate.frame_duration = glfwGetTime() - frame_begin_time;
}

static void do_rendering() {
	const glm::vec3 fog_color(0.6f, 0.5f, 0.5f);
	const float fog_density = mars_atm_density(mars, wstate.current_scene->camera->position);
	const float fog_factor = glm::clamp(100 * double(fog_density) / 0.008e9, 0.0, 1.0);

	world_shader->setf("fog_density", fog_density);
	world_shader->set3f("fog_color", fog_color.x, fog_color.y, fog_color.z);
	glClearColor(fog_color.x * fog_factor, fog_color.y * fog_factor, fog_color.z * fog_factor, 1.0f);

	switch (guistate.selected_scene) {
	case CLOSEUP_SCENE_SELECTED:
		activate_closeup_scene();
		break;
	case ORBIT_SCENE_SELECTED:
		activate_orbit_scene();
		break;
	default:
		error("do_rendering()", "Invalid scene " + std::to_string(guistate.selected_scene) + " selected. Switching to closeup.");
		guistate.selected_scene = CLOSEUP_SCENE_SELECTED;
		break;
	}

	wstate.current_scene->render();
	render_gui();

	glfwSwapBuffers(wstate.window);
	glfwPollEvents();
}

static void do_simulation() {
	static float num_of_updates = 0.0f;
	num_of_updates += guistate.physics_updates_per_frame;

	while (num_of_updates >= 1.0f) {
		simulation_step();
		num_of_updates -= 1.0f;
	}
}

