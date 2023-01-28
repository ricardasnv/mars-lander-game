// Ricardas Navickas 2020
#include "simulation.h"
#include "physics.h"
#include "mars.h"
#include "lander.h"
#include "gui.h"
#include "core/graphics.h"
#include "core/error.h"
#include "global.h"
#include "autopilot.h"

SimulationState simstate;

// Lander autopilot
static AutopilotProgram* ap;

static void process_lander_control_input();
static bool should_update_scenario();
static void set_scenario(int id);

void init_simulation(double physics_timestep) {
	simstate.time = 0.0;
	simstate.timestep = physics_timestep;
	simstate.scenario_id = guistate.selected_scenario;
	simstate.me_manual_control = false;
	simstate.rcs_manual_control = false;
	simstate.paused = false;
	simstate.landed = false;
	simstate.crashed = false;

	ap = &guistate.ap;

	set_scenario(simstate.scenario_id);
}

void add_sim_object(Object* obj) {
	for (unsigned int i = 0; i < simstate.obj.size(); i++) {
		if (simstate.obj[i] == obj) return;
	}

	simstate.obj.push_back(obj);
}

void remove_sim_object(Object* obj) {
	for (unsigned int i = 0; i < simstate.obj.size(); i++) {
		if (simstate.obj[i] == obj) simstate.obj.erase(simstate.obj.begin() + i);
	}
}

void simulation_step() {
	// Change scenario if requested
	if (should_update_scenario()) set_scenario(guistate.selected_scenario);

	// If paused, do nothing
	if (simstate.paused) return;

	// If below or at Mars' surface
	const double landed_dist = BASE_COM_DIST + MARS_RADIUS + mars_surface_height(mars, lander->position - mars->position); // distance between mars & lander objects when landed
	if (glm::length(lander->position - mars->position) <= landed_dist) {
		lander->position = glm::normalize(lander->position - mars->position) * landed_dist;
		lander->reset_integrator();

		if (soft_landing(lander)) {
			simstate.paused = true;
			simstate.landed = true;
			cut_parachute(lander);
		} else if (simstate.crashed) {
			simstate.paused = true;
			cut_parachute(lander);
		} else {
			lander->model = lander_crashed_model;
			lander->velocity += glm::normalize(lander->position - mars->position) * descent_rate(lander, mars) * 1.05;
			lander->ang_velocity = glm::dvec3(2.0, 0.0, 0.0);
			lander->reset_integrator();
			simstate.crashed = true;

			// Add debris objects
			for (unsigned int i = 0; i < lander_debris.size(); i++) {
				glm::dvec3 velocity_diff = 0.5 * descent_rate(lander, mars) * glm::normalize(glm::dvec3(rand(), rand(), rand()));

				lander_debris[i]->position = lander->position;
				lander_debris[i]->velocity = lander->velocity + velocity_diff;
				lander_debris[i]->ang_velocity = 10.0 * glm::normalize(glm::dvec3(rand(), rand(), rand()));
				lander_debris[i]->net_force = grav_force(mars, lander_debris[i]);
				lander_debris[i]->reset_integrator();

				add_sim_object(lander_debris[i]);
			}
		}
	}

	// Setup forces
	lander->net_force = glm::dvec3(0.0);
	lander->net_moment = glm::dvec3(0.0);
	lander->net_force += grav_force(mars, lander);
	lander->net_force += drag_force(lander, mars_wind_velocity(mars, lander->position, simstate.time), mars_atm_density(mars, lander->position), 1.0, lander->attribute["frontal_area"]);

	// Lander control
	lander->attribute["rcs_throttle"] = 0.0;
	if (!simstate.crashed) {
		if (guistate.autopilot_active && ap->loaded) run_autopilot_program(ap);
		if (!guistate.lock_manual_controls) process_lander_control_input();
		fire_engines(lander, simstate.timestep);
	}

	// Update lander and mars objects
	mars->update(simstate.timestep);
	lander->update(simstate.timestep);

	// Update all other objects
	for (unsigned int i = 0; i < simstate.obj.size(); i++) {
		simstate.obj[i]->update(simstate.timestep);
	}

	simstate.time += simstate.timestep;
}

static void process_lander_control_input() {
	if (ImGui::GetIO().WantCaptureKeyboard == 1) return;

	// RCS CONTROLS
	simstate.rcs_manual_control = false;
	glm::dvec3 rcs_axis(0.0);

	if (glfwGetKey(wstate.window, GLFW_KEY_W) == GLFW_PRESS)
		rcs_axis += glm::dvec3(-1.0, 0.0, 0.0);
	if (glfwGetKey(wstate.window, GLFW_KEY_A) == GLFW_PRESS)
		rcs_axis += glm::dvec3(0.0, 0.0, 1.0);
	if (glfwGetKey(wstate.window, GLFW_KEY_S) == GLFW_PRESS)
		rcs_axis += glm::dvec3(1.0, 0.0, 0.0);
	if (glfwGetKey(wstate.window, GLFW_KEY_D) == GLFW_PRESS)
		rcs_axis += glm::dvec3(0.0, 0.0, -1.0);
	if (glfwGetKey(wstate.window, GLFW_KEY_Q) == GLFW_PRESS)
		rcs_axis += glm::dvec3(0.0, -1.0, 0.0);
	if (glfwGetKey(wstate.window, GLFW_KEY_E) == GLFW_PRESS)
		rcs_axis += glm::dvec3(0.0, 1.0, 0.0);
	
	if (glm::length(rcs_axis) > 1e-8) {
		setup_rcs(lander, rcs_axis, 1.0);
		simstate.rcs_manual_control = true;
	}

	// MAIN ENGINE CONTROLS
	simstate.me_manual_control = false;

	if (glfwGetKey(wstate.window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
		setup_main_engine(lander, lander->attribute["me_throttle"] + 0.01);
		simstate.me_manual_control = true;
	} if (glfwGetKey(wstate.window, GLFW_KEY_LEFT) == GLFW_PRESS) {
		setup_main_engine(lander, lander->attribute["me_throttle"] - 0.01);
		simstate.me_manual_control = true;
	}

	// PARACHUTE CONTROLS
	if (glfwGetKey(wstate.window, GLFW_KEY_SPACE) == GLFW_PRESS && lander->attribute["parachute_status"] == 0.0) {
		deploy_parachute(lander);
	}
}

static bool should_update_scenario() {
	//return simstate.scenario_id != guistate.selected_scenario;
	return guistate.scenario_changed;
}

static void set_scenario(int id) {
	reset_lander_attributes(lander);

	lander->model = lander_default_model;

	simstate.time = 0.0;
	//simstate.paused = false;
	simstate.landed = false;
	simstate.crashed = false;
	simstate.scenario_id = id;
	mars->reset_integrator();
	lander->reset_integrator();

	debug("set_scenario()", "Switching to scenario " + std::to_string(id));

	// Remove lander debris objects
	for (unsigned int i = 0; i < lander_debris.size(); i++) {
		remove_sim_object(lander_debris[i]);
	}

	// Reset all objects' Verlet integrators
	for (unsigned int i = 0; i < simstate.obj.size(); i++) {
		simstate.obj[i]->reset_integrator();
	}

	// Position of Mars is same in all scenarios
	mars->position = glm::dvec3(0.0, 0.0, 0.0);
	mars->velocity = glm::dvec3(0.0, 0.0, 0.0);
	mars->attitude_matrix = glm::dmat4(1.0);
	mars->ang_velocity = glm::dvec3(0.0, 2 * M_PI / MARS_DAY, 0.0);

	switch (simstate.scenario_id) {
	case 0:
		// Circular orbit
		lander->position = glm::dvec3(1.2 * MARS_RADIUS, 0.0, 0.0);
		lander->velocity = glm::dvec3(0.0, 0.0, -3.2470874);
		lander->orient_towards(-lander->velocity);
		lander->ang_velocity = glm::dvec3(0.0, 0.0, 0.0);
		break;
	case 1:
		// 10km fall
		lander->position = glm::dvec3(MARS_RADIUS + 10.0, 0.0, 0.0);
		lander->velocity = glm::dvec3(0.0, 0.0, 0.0);
		lander->orient_towards(lander->position - mars->position);
		lander->ang_velocity = glm::dvec3(0.0, 0.0, 0.0);
		break;
	case 2:
		// Elliptical polar orbit
		lander->position = glm::dvec3(0.0, 1.2 * MARS_RADIUS, 0.0);
		lander->velocity = glm::dvec3(3.5, 0.0, 0.0);
		lander->orient_towards(-lander->velocity);
		lander->ang_velocity = glm::dvec3(0.0, 0.0, 0.0);
		break;
	case 3:
		// Launch from pole at escape velocity
		lander->position = glm::dvec3(0.0, MARS_RADIUS + 0.001, 0.0);
		lander->velocity = glm::dvec3(0.0, 5.027, 0.0);
		lander->orient_towards(lander->position - mars->position);
		lander->ang_velocity = glm::dvec3(0.0, 0.0, 0.0);
		break;
	case 4:
		// Elliptical orbit that clips the atmosphere
		lander->position = glm::dvec3(0.0, 0.0, MARS_RADIUS + 100.0);
		lander->velocity = glm::dvec3(4.0, 0.0, 0.0);
		lander->orient_towards(-lander->velocity);
		lander->ang_velocity = glm::dvec3(0.0, 0.0, 0.0);
		break;
	case 5:
		// Descent from 200km
		lander->position = glm::dvec3(MARS_RADIUS + 200.0, 0.0, 0.0);
		lander->velocity = glm::dvec3(0.0, 0.0, 0.0);
		lander->orient_towards(lander->position - mars->position);
		lander->ang_velocity = glm::dvec3(0.0, 0.0, 0.0);
		break;
	case 6:
		// Areostationary orbit
		lander->position = glm::dvec3(20426.976, 0.0, 0.0);
		lander->velocity = glm::dvec3(0.0, 0.0, -1.447866);
		lander->orient_towards(-lander->velocity);
		lander->ang_velocity = glm::dvec3(0.0, 0.0, 0.0);
		break;
	case 7:
		// Inclined elliptical orbit
		lander->position = glm::dvec3(MARS_RADIUS * 1.2, 0.0, 0.0);
		lander->velocity = glm::dvec3(0.0, 2.5, -2.5);
		lander->orient_towards(-lander->velocity);
		lander->ang_velocity = glm::dvec3(0.0, 0.0, 0.0);
		break;
	case 8:
		// Custom scenario
		if (glm::length(guistate.custom_scenario_lander_pos) > 1e-8) {
			lander->position = guistate.custom_scenario_lander_pos;
		} else {
			// if position is zero, replace it with (0, 0, 0)
			lander->position = glm::dvec3(1.0);
		}

		lander->velocity = guistate.custom_scenario_lander_vel;
		//lander->orient_towards(-lander->velocity);
		lander->ang_velocity = glm::dvec3(0.0, 0.0, 0.0);
		break;
	default:
		break;
	}
}

