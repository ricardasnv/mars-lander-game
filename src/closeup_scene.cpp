// Ricardas Navickas 2020
#include "closeup_scene.h"
#include "core/graphics.h"
#include "core/camera.h"
#include "core/scene.h"
#include "core/error.h"
#include "global.h"
#include "simulation.h"
#include "gui.h"
#include <iostream>

// Scene and camera objects not accessible outside of comp. unit
static Scene* closeup_scene;
static Camera* closeup_camera;

// Near mars object
static Object* near_mars = NULL;
static const double mars_transition_height = 10.0;
static const unsigned int near_mars_update_period = 60; // number of calls to update_closeup_scene() between near mars model updates
static bool near_mars_model_active = false;

static double dist_to_lander = 0.015; // Distance from camera to lander

static void closeup_mouse_callback(GLFWwindow* w, double x, double y);
static void closeup_scroll_callback(GLFWwindow* w, double x, double y);

void init_closeup_scene(Shader* world_shader, Shader* light_shader) {
	closeup_camera = new Camera(glm::dvec3(0.0, 0.0, 0.0), glm::dvec3(0.0, 0.0, -0.5), glm::dvec3(0.0, 1.0, 0.0), 45.0, 0.01);
	closeup_scene = new Scene(closeup_camera, world_shader, light_shader);

	closeup_scene->add_object(mars);
	closeup_scene->add_object(lander);
	closeup_scene->add_object(lander_parachute);
	closeup_scene->add_nofx_object(lander_exhaust);

	for (unsigned int i = 0; i < lander_debris.size(); i++) {
		closeup_scene->add_object(lander_debris[i]);
	}

	closeup_scene->add_light(sun, glm::vec3(1.0f, 1.0f, 1.0f));
	closeup_scene->render_wireframe = false;

	near_mars = make_mars_near_object(mars, lander);
}

void activate_closeup_scene() {
	wstate.current_scene = closeup_scene;
	wstate.mouse_callback = closeup_mouse_callback;
	wstate.scroll_callback = closeup_scroll_callback;
}

void update_closeup_scene() {
	static unsigned int update_count = 0;

	// Make the closeup camera follow the lander
	glm::dvec3 dist = lander->position - mars->position;
	closeup_camera->right = glm::normalize(-glm::cross(dist, closeup_camera->facing));
	closeup_camera->position = lander->position - dist_to_lander * closeup_camera->facing;
	closeup_camera->up = glm::normalize(glm::cross(closeup_camera->right, closeup_camera->facing));

	if (lander->attribute["me_throttle"] > 0.0 && lander->attribute["fuel_level"] > 0.0) {
		lander_exhaust->position = lander->position + lander->attitude_matrix * glm::dvec3(0.0, -0.0004 - EXHAUST_MAX_LENGTH * lander->attribute["me_throttle"], 0.0);
		lander_exhaust->attitude_matrix = lander->attitude_matrix;
		update_exhaust_model(lander, lander_exhaust);
	} else {
		lander_exhaust->position = glm::dvec3(0.0);
	}

	if (lander->attribute["parachute_status"] != 1.0) {
		lander_parachute->position = glm::dvec3(0.0, 0.0, 0.0);
	} else {
		lander_parachute->position = lander->position;
		lander_parachute->orient_towards(-(lander->velocity - mars_surface_velocity(mars, lander->position)));
	}

	// Transition to Mars near model
	if (glm::length(lander->position - mars->position) - MARS_RADIUS < mars_transition_height && near_mars_model_active == false) {
		update_mars_near_object(near_mars, mars, lander);
		closeup_scene->add_object(near_mars);
		closeup_scene->remove_object(mars);
		add_sim_object(near_mars);
		near_mars_model_active = true;
	}

	// Transition to Mars far model
	if (glm::length(lander->position - mars->position) - MARS_RADIUS > mars_transition_height && near_mars_model_active == true) {
		closeup_scene->add_object(mars);
		closeup_scene->remove_object(near_mars);
		remove_sim_object(near_mars);
		near_mars_model_active = false;
	}

	// Update
	if (near_mars_model_active == true) {
		near_mars->orient_towards(lander->position - mars->position);
		near_mars->velocity = mars_surface_velocity(mars, lander->position - mars->position);
		near_mars->ang_velocity = mars->ang_velocity;
		near_mars->reset_integrator(); // effectively does Euler integration

		if (update_count % near_mars_update_period == 0) {
			update_mars_near_object(near_mars, mars, lander);
		}
	}

	update_count++;
}

static void closeup_mouse_callback(GLFWwindow* w, double xpos, double ypos) {
	static float lastX = DEFAULT_WINDOW_WIDTH / 2;
	static float lastY = DEFAULT_WINDOW_HEIGHT / 2;

	static const double x_sens = 0.01;
	static const double y_sens = 0.01;

	double xoffset = xpos - lastX;
	double yoffset = lastY - ypos;

	if (glfwGetMouseButton(w, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
		glm::dvec3 radial = glm::normalize(lander->position - mars->position);
		glm::dvec3 cross = glm::cross(radial, glm::normalize(closeup_camera->facing));
		double sin_theta = glm::length(cross);

		glm::dvec3 diff_x = xoffset * sin_theta * closeup_camera->right;
		closeup_camera->facing += x_sens * diff_x;

		glm::dvec3 diff_y = yoffset * closeup_camera->up;
		glm::dvec3 new_cross = glm::cross(radial, glm::normalize(closeup_camera->facing + y_sens * diff_y));

		if (glm::dot(cross, new_cross) > 0.0)
			closeup_camera->facing += y_sens * diff_y;

		closeup_camera->facing = glm::normalize(closeup_camera->facing);
	}

	lastX = xpos;
	lastY = ypos;
}

static void closeup_scroll_callback(GLFWwindow* w, double xoffset, double yoffset) {
	const double dist_multiplier = 1.05;
	const double lower_limit = 0.01;
	const double upper_limit = 2.0;

	if (yoffset > 0.0) {
		dist_to_lander /= dist_multiplier;
	} else if (yoffset < 0.0) {
		dist_to_lander *= dist_multiplier;
	}

	// Clamp
	if (dist_to_lander < lower_limit)
		dist_to_lander = lower_limit;
	else if (dist_to_lander > upper_limit)
		dist_to_lander = upper_limit;
}

