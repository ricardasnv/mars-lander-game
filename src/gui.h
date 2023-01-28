// Ricardas Navickas 2020
#ifndef GUI_H
#define GUI_H

#include "core/object.h"
#include "autopilot.h"
#include <vector>
#include <string>

// Scene selection enum
#define CLOSEUP_SCENE_SELECTED 0
#define ORBIT_SCENE_SELECTED 1

extern struct GUIState {
	// ==== Internal state ====
	unsigned int indicator_update_period;
	bool should_update_indicators; // update fps/timing indicators every indicator_update_period frames

	bool show_mainmenu_window;
	bool show_help_window;
	bool show_sim_window;
	bool show_lander_window;
	bool show_autopilot_window;
	bool show_debug_window;

	// ==== Inputs to UI ====
	float frame_duration, render_duration, sim_duration, fc_duration, other_duration; // Frame timings
	float sim_time, sim_timestep; // Simulation params
	std::vector<std::string> autopilot_message_log;

	// ==== Outputs from UI ====
	int selected_scene;
	int selected_scenario;
	bool scenario_changed;
	glm::dvec3 custom_scenario_lander_pos;
	glm::dvec3 custom_scenario_lander_vel;
	AutopilotProgram ap;
	bool lock_manual_controls;
	bool autopilot_active;

	// ==== Inputs/outputs ====
	float physics_updates_per_frame;
} guistate;

extern std::vector<std::string> scenario_info;

void init_gui();
void render_gui();

#endif
