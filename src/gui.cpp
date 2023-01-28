// Ricardas Navickas 2020
#include "core/graphics.h"
#include "core/error.h"
#include "gui.h"
#include "physics.h"
#include "lander.h"
#include "mars.h"
#include "simulation.h"
#include "global.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

static void draw_mainmenu_window();
static void draw_help_window();
static void draw_sim_window();
static void draw_lander_window();
static void draw_autopilot_window();
static void draw_debug_window();

GUIState guistate;
std::vector<std::string> scenario_info;

void init_gui() {
	guistate.indicator_update_period = 10;
	guistate.should_update_indicators = false;
	guistate.show_mainmenu_window = true;
	guistate.show_help_window = false;
	guistate.show_sim_window = false;
	guistate.show_lander_window = false;
	guistate.show_autopilot_window = false;
	guistate.show_debug_window = false;
	guistate.frame_duration = 0.0f;
	guistate.sim_time = 0.0f;
	guistate.sim_timestep = 0.0f;
	guistate.selected_scene = CLOSEUP_SCENE_SELECTED;
	guistate.selected_scenario = 0;
	guistate.scenario_changed = false;
	guistate.custom_scenario_lander_pos = glm::dvec3(0.0);
	guistate.custom_scenario_lander_vel = glm::dvec3(0.0);
	guistate.physics_updates_per_frame = 1;
	guistate.lock_manual_controls = false;
	guistate.autopilot_active = false;

	scenario_info.push_back("0. Circular orbit");
	scenario_info.push_back("1. 10km straight descent");
	scenario_info.push_back("2. Elliptical polar orbit");
	scenario_info.push_back("3. Polar surface launch at escape vel. (drag prevents escape)");
	scenario_info.push_back("4. Elliptical orbit that clips the atmosphere each time round");
	scenario_info.push_back("5. Straight descent from rest at the edge of the exosphere");
	scenario_info.push_back("6. Areostationary orbit");
	scenario_info.push_back("7. Inclined elliptical orbit");
	scenario_info.push_back("8. Custom");

	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(wstate.window, true);
	ImGui_ImplOpenGL3_Init("#version 330 core");
	ImGui::StyleColorsDark();

	debug("init_gui()", "Initialized GUI.");
}

void render_gui() {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	//ImGui::ShowDemoWindow();

	if (guistate.show_mainmenu_window) draw_mainmenu_window();
	if (guistate.show_help_window) draw_help_window();
	if (guistate.show_sim_window) draw_sim_window();
	if (guistate.show_lander_window) draw_lander_window();
	if (guistate.show_autopilot_window) draw_autopilot_window();
	if (guistate.show_debug_window) draw_debug_window();

	static unsigned int counter = 0;
	if (counter >= guistate.indicator_update_period) {
		guistate.should_update_indicators = true;
		counter = 0;
	} else {
		guistate.should_update_indicators = false;
		counter++;
	}

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

static void draw_mainmenu_window() {
	ImVec2 window_pos(10.0f, 10.0f);
	ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, ImVec2(0.0f, 0.0f));
	ImGui::SetNextWindowSize(ImVec2(200, 0), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowBgAlpha(0.35f);

	ImGuiWindowFlags window_flags = 0;
	window_flags |= ImGuiWindowFlags_AlwaysAutoResize;
	window_flags |= ImGuiWindowFlags_NoDecoration;
	window_flags |= ImGuiWindowFlags_NoSavedSettings;
	window_flags |= ImGuiWindowFlags_NoFocusOnAppearing;
	window_flags |= ImGuiWindowFlags_NoNav;
	window_flags |= ImGuiWindowFlags_NoMove;

	ImGui::Begin("Main menu", &guistate.show_mainmenu_window, window_flags);

	static float fps = 1.0f / guistate.frame_duration;
	if (guistate.should_update_indicators) fps = 1.0f / guistate.frame_duration;
	ImGui::Text("FPS: %.1f", fps);

	ImGui::Separator();

	ImGui::Text("Current view:");
	ImGui::RadioButton("Closeup", &guistate.selected_scene, CLOSEUP_SCENE_SELECTED);
	ImGui::SameLine();
	ImGui::RadioButton("Orbit", &guistate.selected_scene, ORBIT_SCENE_SELECTED);

	ImGui::Separator();

	ImGui::Text("Open windows:");
	ImGui::Checkbox("Help", &guistate.show_help_window);
	ImGui::Checkbox("Simulation parameters", &guistate.show_sim_window);
	ImGui::Checkbox("Lander dashboard", &guistate.show_lander_window);
	ImGui::Checkbox("Autopilot menu", &guistate.show_autopilot_window);
	ImGui::Checkbox("Debug", &guistate.show_debug_window);

	ImGui::End();
}

static void draw_help_window() {
	ImGui::SetNextWindowBgAlpha(0.35f);
	ImGui::SetNextWindowPos(ImVec2(wstate.window_width / 2 - 240, wstate.window_height / 2 - 160), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(480, 0));

	ImGuiWindowFlags window_flags = 0;
	//window_flags |= ImGuiWindowFlags_NoResize;
	window_flags |= ImGuiWindowFlags_NoSavedSettings;

	ImGui::Begin("Help", &guistate.show_help_window, window_flags);

	ImGui::TextWrapped("Your goal is to softly land your spacecraft on the surface of Mars. "
	                   "A soft landing must satisfy the following conditions: descent rate < 1.0 m/s, ground speed < 1.0 m/s. "
					   "The spacecraft is equipped with an engine with a throttle range of 0%%-100%%, a reaction control system and a parachute.");

	if (ImGui::CollapsingHeader("Scenarios")) {
		ImGui::TextWrapped("You have a choice of several built-in scenarios (initial conditions) to attempt soft landing from. "
		                   "Select a scenario by choosing one from the drop-down list in the simulation window and clicking \'Apply\'.");
	}

	if (ImGui::CollapsingHeader("Controls")) {
		ImGui::Columns(2);
		ImGui::SetColumnOffset(1, 120);
		ImGui::TextWrapped("Left/right");
		ImGui::NextColumn();
		ImGui::TextWrapped("Control engine throttle.");
		ImGui::NextColumn();
		ImGui::TextWrapped("Up/down");
		ImGui::NextColumn();
		ImGui::TextWrapped("Control simulation speed.");
		ImGui::NextColumn();
		ImGui::TextWrapped("W/A/S/D/Q/E");
		ImGui::NextColumn();
		ImGui::TextWrapped("Fire reaction control thrusters.");
		ImGui::NextColumn();
		ImGui::TextWrapped("Spacebar");
		ImGui::NextColumn();
		ImGui::TextWrapped("Deploy parachute (surface velocity < 500 m/s).");
		ImGui::Columns(1);
	}

	if (ImGui::CollapsingHeader("Autopilot")) {
		ImGui::TextWrapped("Autopilot programs are Lua scripts that are run once per simulation-tick. "
		                   "Follow these steps to load and run an autopilot program:");
		ImGui::TextWrapped("1. Enter the path to your program (relative to the current working directory) in the autopilot menu and click \'Load\';");
		ImGui::TextWrapped("2. If no error messages appeared, your program is now loaded and ready to be run using the \'Autopilot active\' checkbox;");
		ImGui::TextWrapped("If at any point the autopilot program encounters invalid Lua code, the program is immediately unloaded and an error message will be displayed in the autopilot window. "
		                   "For a detailed autopilot programming reference see doc/autopilot.txt.");
	}

	ImGui::End();
}

static void draw_sim_window() {
	ImGui::SetNextWindowBgAlpha(0.35f);
	ImGui::SetNextWindowPos(ImVec2(200 + 20, 10), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(640, 0));

	ImGuiWindowFlags window_flags = 0;
	window_flags |= ImGuiWindowFlags_NoResize;
	window_flags |= ImGuiWindowFlags_NoSavedSettings;

	ImGui::Begin("Simulation parameters", &guistate.show_sim_window, window_flags);

	ImGui::Text("Simulation timestep: %.1f ms", 1000 * guistate.sim_timestep);
	ImGui::Text("Simulation time: %.2f s", guistate.sim_time);

	static float ratio = guistate.sim_timestep / guistate.frame_duration;
	if (guistate.should_update_indicators) ratio = guistate.sim_timestep / guistate.frame_duration;
	ImGui::Text("Simulation speed vs. real time: %.2fx", ratio * guistate.physics_updates_per_frame);

	ImGui::Text("Simulation state:");
	ImGui::SameLine();
	if (simstate.crashed) {
		ImGui::Text("CRASHED");
	} else if (simstate.landed) {
		ImGui::Text("LANDED");
	} else {
		ImGui::Text("NOT LANDED");
	}

	ImGui::Separator();

	static float lower_update_limit = 0.25f;
	static float upper_update_limit = 2048.0f;
	ImGui::Columns(5, "simulation_speedup", false);
	ImGui::SetColumnOffset(1, 200);
	ImGui::SetColumnOffset(2, 530);
	ImGui::SetColumnOffset(3, 560);
	ImGui::SetColumnOffset(4, 590);
	ImGui::Text("Physics updates per frame:");
	ImGui::NextColumn();
	if (simstate.paused) {
		ImGui::Text("%.2f (PAUSED)", guistate.physics_updates_per_frame);
	} else {
		ImGui::Text("%.2f", guistate.physics_updates_per_frame);
	}
	ImGui::NextColumn();
	if (ImGui::Button("<<")) {
		if (guistate.physics_updates_per_frame <= lower_update_limit) {
			guistate.physics_updates_per_frame = lower_update_limit;
		} else {
			guistate.physics_updates_per_frame /= 2.0f;
		}
	}
	ImGui::NextColumn();
	if (ImGui::Button("II")) {
		simstate.paused = !simstate.paused;
	}
	ImGui::NextColumn();
	if (ImGui::Button(">>")) {
		if (guistate.physics_updates_per_frame >= upper_update_limit) {
			guistate.physics_updates_per_frame = upper_update_limit;
		} else {
			guistate.physics_updates_per_frame *= 2;
		}
	}
	ImGui::Columns(1);

	ImGui::Separator();

	ImGui::Text("Select scenario:");

	static int combo_selection = 0;
	guistate.scenario_changed = false;
	if (ImGui::Button("Apply")) {
		guistate.selected_scenario = combo_selection;
		guistate.scenario_changed = true;
	}

	ImGui::SameLine();

	ImGuiComboFlags combo_flags = ImGuiComboFlags_HeightLargest;
	ImGui::PushItemWidth(-1.0f);
	if (ImGui::BeginCombo("", scenario_info[combo_selection].c_str(), combo_flags)) {
		for (unsigned int i = 0; i < scenario_info.size(); i++) {
			if (ImGui::Selectable(scenario_info[i].c_str(), combo_selection == (int)i)) {
				combo_selection = i;
			}
		}

		ImGui::EndCombo();
	}

	if (combo_selection == 8) {
		ImGui::Columns(4, "custom_scenario", false);

		ImGui::Text("Lander pos. (km): ");
		ImGui::NextColumn();
		ImGui::Text("x = ");
		ImGui::SameLine();
		ImGui::InputDouble("    pos_x", &guistate.custom_scenario_lander_pos.x);
		ImGui::NextColumn();
		ImGui::Text("y = ");
		ImGui::SameLine();
		ImGui::InputDouble("    pos_y", &guistate.custom_scenario_lander_pos.y);
		ImGui::NextColumn();
		ImGui::Text("z = ");
		ImGui::SameLine();
		ImGui::InputDouble("    pos_z", &guistate.custom_scenario_lander_pos.z);
		ImGui::NextColumn();

		ImGui::Text("Lander vel. (km/s): ");
		ImGui::NextColumn();
		ImGui::Text("x = ");
		ImGui::SameLine();
		ImGui::InputDouble("    vel_x", &guistate.custom_scenario_lander_vel.x);
		ImGui::NextColumn();
		ImGui::Text("y = ");
		ImGui::SameLine();
		ImGui::InputDouble("    vel_y", &guistate.custom_scenario_lander_vel.y);
		ImGui::NextColumn();
		ImGui::Text("z = ");
		ImGui::SameLine();
		ImGui::InputDouble("    vel_z", &guistate.custom_scenario_lander_vel.z);
		ImGui::NextColumn();

		ImGui::Columns(1);

		ImGui::TextWrapped("NOTE: Mars' radius is %.2f", MARS_RADIUS);
	}

	ImGui::End();
}

static void draw_lander_window() {
	ImGui::SetNextWindowPos(ImVec2(wstate.window_width - 600 - 10, 10), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(600, 0));
	ImGui::SetNextWindowBgAlpha(0.35f);

	ImGuiWindowFlags window_flags = 0;
	//window_flags |= ImGuiWindowFlags_AlwaysAutoResize;
	//window_flags |= ImGuiWindowFlags_NoResize;
	window_flags |= ImGuiWindowFlags_NoSavedSettings;

	ImGui::Begin("Lander dashboard", &guistate.show_lander_window, window_flags);

	// GENERAL INFO
	ImGui::Columns(2, "lander_general", false);
	ImGui::SetColumnOffset(1, 130);

	ImGui::Text("Throttle");
	ImGui::NextColumn();
	ImGui::ProgressBar(lander->attribute["me_throttle"], ImVec2(-1.0, 0.0));
	ImGui::NextColumn();

	ImGui::Text("Delta-V");
	ImGui::NextColumn();
	double delta_v = lander_delta_v(lander);
	double delta_v_fraction = delta_v / lander_max_delta_v(lander);
	std::string delta_v_str = std::to_string(int(delta_v * 1000)) + " m/s";
	ImGui::ProgressBar(delta_v_fraction, ImVec2(-1.0, 0.0), delta_v_str.c_str());
	ImGui::NextColumn();

	ImGui::Separator();

	ImGui::Text("Altitude:");
	ImGui::NextColumn();
	ImGui::Text("%.2f m", 1000 * (glm::length(lander->position - mars->position) - MARS_RADIUS - mars_surface_height(mars, lander->position - mars->position) - BASE_COM_DIST));
	ImGui::NextColumn();

	ImGui::Text("Velocity:");
	ImGui::NextColumn();
	ImGui::Text("%.2f m/s", 1000 * glm::length(lander->velocity));
	ImGui::NextColumn();

	ImGui::Text("Surf. velocity:");
	ImGui::NextColumn();
	ImGui::Text("%.2f m/s", 1000 * glm::length(lander->velocity - mars_surface_velocity(mars, lander->position)));
	ImGui::NextColumn();

	ImGui::Text("Mass:");
	ImGui::NextColumn();
	ImGui::Text("%.2f kg", lander->mass);
	ImGui::NextColumn();

	ImGui::Text("Parachute:");
	ImGui::NextColumn();
	if (lander->attribute["parachute_status"] == 0.0) {
		ImGui::Text("NOT DEPLOYED");
	} else if (lander->attribute["parachute_status"] == 1.0) {
		ImGui::Text("DEPLOYED");
	} else {
		ImGui::Text("LOST");
	}
	ImGui::NextColumn();

	ImGui::Columns(1);

	// ORBIT INFO
	if (ImGui::CollapsingHeader("Orbit", ImGuiTreeNodeFlags_None)) {
		ImGui::Columns(2, "lander_orbit", false);
		ImGui::SetColumnOffset(1, 130);

		ImGui::Text("Periapsis alt.:");
		ImGui::NextColumn();
		ImGui::Text("%.2f m", 1000 * periapsis_radius(mars, lander) - 1000 * MARS_RADIUS);
		ImGui::NextColumn();

		ImGui::Text("Apoapsis alt.:");
		ImGui::NextColumn();
		ImGui::Text("%.2f m", 1000 * apoapsis_radius(mars, lander) - 1000 * MARS_RADIUS);
		ImGui::NextColumn();

		ImGui::Columns(1);
	}

	// LANDING INFO
	if (ImGui::CollapsingHeader("Landing", ImGuiTreeNodeFlags_None)) {
		ImGui::Columns(2, "lander_landing", false);
		ImGui::SetColumnOffset(1, 130);

		ImGui::Text("Descent rate:");
		ImGui::NextColumn();
		ImGui::Text("%.2f m/s", 1000 * descent_rate(lander, mars));
		ImGui::NextColumn();

		ImGui::Text("Groundspeed:");
		ImGui::NextColumn();
		ImGui::Text("%.2f m/s", 1000 * groundspeed(lander, mars));
		ImGui::NextColumn();

		ImGui::Columns(1);
	}

	if (ImGui::CollapsingHeader("Internals", ImGuiTreeNodeFlags_None)) {
		ImGui::Columns(3, "lander_internals", false);
		ImGui::SetColumnOffset(1, 130);
		ImGui::SetColumnOffset(2, 260);

		ImGui::Text("Position:");
		ImGui::NextColumn();
		ImGui::Text("%.2f m", 1000 * glm::length(lander->position));
		ImGui::NextColumn();
		ImGui::Text("(%.1f m, %.1f m, %.1f m)", 1000 * lander->position.x, 1000 * lander->position.y, 1000 * lander->position.z);
		ImGui::NextColumn();

		ImGui::Text("Velocity:");
		ImGui::NextColumn();
		ImGui::Text("%.2f m/s", 1000 * glm::length(lander->velocity));
		ImGui::NextColumn();
		ImGui::Text("(%.1f m/s, %.1f m/s, %.1f m/s)", 1000 * lander->velocity.x, 1000 * lander->velocity.y, 1000 * lander->velocity.z);
		ImGui::NextColumn();

		ImGui::Text("Acceleration:");
		ImGui::NextColumn();
		ImGui::Text("%.2f m/s^2", 1000 * glm::length(lander->acceleration));
		ImGui::NextColumn();
		ImGui::Text("(%.1f m/s^2, %.1f m/s^2, %.1f m/s^2)", 1000 * lander->acceleration.x, 1000 * lander->acceleration.y, 1000 * lander->acceleration.z);
		ImGui::NextColumn();

		ImGui::Text("Net force:");
		ImGui::NextColumn();
		ImGui::Text("%.2f N", 1000 * glm::length(lander->net_force));
		ImGui::NextColumn();
		ImGui::Text("(%.1f N, %.1f N, %.1f N)", 1000 * lander->net_force.x, 1000 * lander->net_force.y, 1000 * lander->net_force.z);
		ImGui::NextColumn();

		glm::vec3 heading = lander->attitude_matrix * glm::vec3(0.0, 1.0, 0.0);
		ImGui::Text("Lander heading");
		ImGui::NextColumn();
		ImGui::NextColumn();
		ImGui::Text("(%.2f, %.2f, %.2f)", heading.x, heading.y, heading.z);
		ImGui::NextColumn();

		ImGui::Text("Angular vel.:");
		ImGui::NextColumn();
		ImGui::Text("%.2f rad/s", glm::length(lander->ang_velocity));
		ImGui::NextColumn();
		ImGui::Text("(%.1f rad/s, %.1f rad/s, %.1f rad/s)", lander->ang_velocity.x, lander->ang_velocity.y, lander->ang_velocity.z);
		ImGui::NextColumn();

		ImGui::Text("Angular accel.:");
		ImGui::NextColumn();
		ImGui::Text("%.2f rad/s^2", glm::length(lander->ang_acceleration));
		ImGui::NextColumn();
		ImGui::Text("(%.1f rad/s^2, %.1f rad/s^2, %.1f rad/s^2)", lander->ang_acceleration.x, lander->ang_acceleration.y, lander->ang_acceleration.z);
		ImGui::NextColumn();

		ImGui::Text("Net moment:");
		ImGui::NextColumn();
		ImGui::Text("%.2f Nm", 1000 * glm::length(lander->net_moment));
		ImGui::NextColumn();
		ImGui::Text("(%.1f Nm, %.1f Nm, %.1f Nm)", 1000 * lander->net_moment.x, 1000 * lander->net_moment.y, 1000 * lander->net_moment.z);
		ImGui::NextColumn();

		ImGui::Columns(1);
	}

	if (ImGui::CollapsingHeader("Mars", ImGuiTreeNodeFlags_None)) {
		ImGui::Columns(2, "lander_mars", false);
		ImGui::SetColumnOffset(1, 130);

		ImGui::Text("Atm. density:");
		ImGui::NextColumn();
		ImGui::Text("%.6f kg/m^3", 1e-9 * mars_atm_density(mars, lander->position));
		ImGui::NextColumn();

		ImGui::Columns(1);
	}

	ImGui::End();
}

static void draw_autopilot_window() {
	ImGui::SetNextWindowPos(ImVec2(10, wstate.window_height - 200 - 10), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(480, 200), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowBgAlpha(0.35f);

	ImGuiWindowFlags window_flags = 0;
	//window_flags |= ImGuiWindowFlags_NoResize;
	window_flags |= ImGuiWindowFlags_NoSavedSettings;
	window_flags |= ImGuiWindowFlags_NoScrollbar;

	ImGui::Begin("Autopilot menu", &guistate.show_autopilot_window, window_flags);

	int width = ImGui::GetWindowSize().x;

	// Autopilot loader
	ImGui::Columns(2, "autopilot_program_loader", false);
	ImGui::SetColumnOffset(1, width-50);

	static char program_path[128] = "";
	ImGui::Text("Program path: $PWD/");
	ImGui::SameLine();
	ImGui::InputText("", program_path, IM_ARRAYSIZE(program_path));
	ImGui::NextColumn();
	if (ImGui::Button("Load")) guistate.ap = make_autopilot_program(program_path);
	ImGui::NextColumn();

	ImGui::Columns(1);

	ImGui::Separator();

	if (guistate.ap.loaded) {
		ImGui::Text("Loaded program: $PWD/%s", guistate.ap.path.c_str());
	} else {
		ImGui::Text("Loaded program: none");
	}
	if (ImGui::Button("Clear log")) guistate.autopilot_message_log.clear();
	ImGui::SameLine();
	ImGui::Checkbox("Autopilot active", &guistate.autopilot_active);
	ImGui::SameLine();
	ImGui::Checkbox("Lock manual controls", &guistate.lock_manual_controls);

	ImGui::Separator();

	// Message log
	ImGui::BeginChild("message_log", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

	for (unsigned int i = 0; i < guistate.autopilot_message_log.size(); i++)
		ImGui::Text("%s", guistate.autopilot_message_log[guistate.autopilot_message_log.size() - i - 1].c_str());

	ImGui::EndChild();
	ImGui::Separator();

	ImGui::End();
}

static void draw_debug_window() {
	ImGui::SetNextWindowSize(ImVec2(400, 0), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowBgAlpha(0.35f);

	ImGuiWindowFlags window_flags = 0;
	window_flags |= ImGuiWindowFlags_NoResize;
	window_flags |= ImGuiWindowFlags_NoSavedSettings;

	ImGui::Begin("Debug", &guistate.show_debug_window, window_flags);

	static bool wireframe_selected = false;
	ImGui::Checkbox("Render models as wireframes", &wireframe_selected);
	if (wireframe_selected) wstate.current_scene->render_wireframe = true;
	else wstate.current_scene->render_wireframe = false;

	ImGui::Separator();

	static float shown_frame_duration = guistate.frame_duration;
	static float shown_render_duration = guistate.render_duration;
	static float shown_sim_duration = guistate.sim_duration;
	static float shown_fc_duration = guistate.fc_duration;
	static float shown_other_duration = guistate.other_duration;

	if (guistate.should_update_indicators) {
		shown_frame_duration = guistate.frame_duration;
		shown_render_duration = guistate.render_duration;
		shown_sim_duration = guistate.sim_duration;
		shown_fc_duration = guistate.fc_duration;
		shown_other_duration = guistate.other_duration;
	}

	ImGui::Columns(3, "frame_timings", false);

	ImGui::Text("Phase");
	ImGui::NextColumn();
	ImGui::Text("Time");
	ImGui::NextColumn();
	ImGui::Text("%% of total");
	ImGui::NextColumn();

	ImGui::Separator();

	ImGui::Text("total");
	ImGui::NextColumn();
	ImGui::Text("%.2f ms", shown_frame_duration * 1000);
	ImGui::NextColumn();
	ImGui::Text("%.1f %%", 100 * shown_frame_duration / shown_frame_duration);
	ImGui::NextColumn();

	ImGui::Text("render");
	ImGui::NextColumn();
	ImGui::Text("%.2f ms", shown_render_duration * 1000);
	ImGui::NextColumn();
	ImGui::Text("%.1f %%", 100 * shown_render_duration / shown_frame_duration);
	ImGui::NextColumn();

	ImGui::Text("sim");
	ImGui::NextColumn();
	ImGui::Text("%.2f ms", shown_sim_duration * 1000);
	ImGui::NextColumn();
	ImGui::Text("%.1f %%", 100 * shown_sim_duration / shown_frame_duration);
	ImGui::NextColumn();

	ImGui::Text("other");
	ImGui::NextColumn();
	ImGui::Text("%.2f ms", shown_other_duration * 1000);
	ImGui::NextColumn();
	ImGui::Text("%.1f %%", 100 * shown_other_duration / shown_frame_duration);
	ImGui::NextColumn();

	ImGui::Text("wait");
	ImGui::NextColumn();
	ImGui::Text("%.2f ms", shown_fc_duration * 1000);
	ImGui::NextColumn();
	ImGui::Text("%.1f %%", 100 * shown_fc_duration / shown_frame_duration);
	ImGui::NextColumn();

	ImGui::End();
}

