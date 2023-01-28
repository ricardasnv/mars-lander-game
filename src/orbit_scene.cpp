// Ricardas Navickas 2020
#include "orbit_scene.h"
#include "core/graphics.h"
#include "core/camera.h"
#include "core/scene.h"
#include "core/error.h"
#include "global.h"
#include "gui.h"
#include "simulation.h"

// Scene and camera objects not accessible outside of comp. unit
static Scene* orbit_scene;
static Camera* orbit_camera;

static Object* lander_track;
static Object* lander_indicator;

static const glm::vec3 lander_track_color = glm::vec3(0.6f, 0.6f, 1.0f);
static const int track_points = 1024; // number of points in track
static const float track_update_period = 16.0f; // number of simulation-seconds between updates
static double dist_to_mars = 10 * MARS_RADIUS;

static double last_update_time;
static int prev_scenario = simstate.scenario_id;

static void reset_lander_track();
static void update_lander_track();
static void orbit_mouse_callback(GLFWwindow* w, double x, double y);
static void orbit_scroll_callback(GLFWwindow* w, double x, double y);

void init_orbit_scene(Shader* world_shader, Shader* light_shader) {
	Mesh* lander_track_mesh = new Mesh;
	lander_track_mesh->vertex_data.resize(VERTEX_DATA_LEN * track_points);
	Model* lander_track_model = new Model(lander_track_mesh, GL_LINES);
	lander_track = new Object(lander_track_model, glm::dvec3(0.0, 0.0, 0.0), 0.0f, 1);
	reset_lander_track();

	Mesh* lander_indicator_mesh = new Mesh;
	*lander_indicator_mesh = make_uv_sphere_mesh(12, 6, lander_track_color.x, lander_track_color.y, lander_track_color.z);
	transform_mesh(lander_indicator_mesh, glm::scale(glm::mat4(1.0f), glm::vec3(MARS_RADIUS / 100.0f)));
	Model* lander_indicator_model = new Model(lander_indicator_mesh, GL_TRIANGLES);
	lander_indicator = new Object(lander_indicator_model, glm::dvec3(0.0, 0.0, 0.0), 0.0f, 1);

	orbit_camera = new Camera(glm::dvec3(0.0, 0.0, 0.0), glm::dvec3(0.0, 0.0, -0.5), glm::dvec3(0.0, 1.0, 0.0), 45.0, 0.01);
	orbit_scene = new Scene(orbit_camera, world_shader, light_shader);
	orbit_scene->add_object(mars);
	orbit_scene->add_nofx_object(lander_track);
	orbit_scene->add_nofx_object(lander_indicator);
	orbit_scene->add_light(sun, glm::vec3(1.0f, 1.0f, 1.0f));
	orbit_scene->render_wireframe = false;
}

void activate_orbit_scene() {
	wstate.current_scene = orbit_scene;
	wstate.mouse_callback = orbit_mouse_callback;
	wstate.scroll_callback = orbit_scroll_callback;
}

void update_orbit_scene() {
	// If scenario changed, reset the track
	if (guistate.scenario_changed) {
		reset_lander_track();
		prev_scenario = simstate.scenario_id;
		last_update_time = simstate.time;
	}

	// One update for each track_update_period that passed since last update
	int track_updates = (simstate.time - last_update_time) / track_update_period;
	for (int i = 0; i < track_updates; i++) update_lander_track();
	set_vertex_coords(lander_track->model->mesh, 0, lander->position);
	lander_track->model->reload_mesh();

	lander_indicator->position = lander->position;

	// Update camera
	orbit_camera->position = mars->position - dist_to_mars * orbit_camera->facing;

	orbit_camera->right.x = orbit_camera->facing.z;
	orbit_camera->right.y = 0;
	orbit_camera->right.z = -1.0 * orbit_camera->facing.x;

	orbit_camera->right = glm::normalize(orbit_camera->right);
	orbit_camera->up = glm::normalize(glm::cross(orbit_camera->right, orbit_camera->facing));
}

static void update_lander_track() {
	Mesh* m = lander_track->model->mesh; // Alias for convenience

	// Shift all point coordinates in track by 1
	for (unsigned int i = num_of_vertices(m) - 1; i > 0; i--)
		set_vertex_coords(m, i, get_vertex_coords(m, i - 1));

	// Insert new point into front of list
	set_vertex_coords(m, 0, lander->position);

	lander_track->model->reload_mesh();

	last_update_time = simstate.time;
	prev_scenario = simstate.scenario_id;
}

static void reset_lander_track() {
	Mesh* m = lander_track->model->mesh;

	for (int i = 0; i < track_points; i++) {
		float alpha_multiplier = 1.0f - 1.0f * float(i) / track_points;

		set_vertex_coords(m, i, lander->position);
		set_vertex_normal(m, i, glm::vec3(alpha_multiplier, 0.0f, 0.0f));
		set_vertex_color(m, i, lander_track_color);

		// Indices
		if (i != track_points - 1) {
			m->indices.push_back(i);
			m->indices.push_back(i + 1);
		}
	}

	lander_track->model->reload_mesh();
}

static void orbit_mouse_callback(GLFWwindow* w, double xpos, double ypos) {
	static float lastX = DEFAULT_WINDOW_WIDTH / 2;
	static float lastY = DEFAULT_WINDOW_HEIGHT / 2;

	static const double x_sens = 0.01;
	static const double y_sens = 0.01;

	double xoffset = xpos - lastX;
	double yoffset = lastY - ypos;

	if (glfwGetMouseButton(w, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
		glm::dvec3 cross = glm::cross(glm::dvec3(0.0, 1.0, 0.0), glm::normalize(orbit_camera->facing));
		double sin_theta = glm::length(cross);

		glm::dvec3 diff_x = xoffset * sin_theta * orbit_camera->right;
		orbit_camera->facing += x_sens * diff_x;

		glm::dvec3 diff_y = yoffset * orbit_camera->up;
		glm::dvec3 new_cross = glm::cross(glm::dvec3(0.0, 1.0, 0.0), glm::normalize(orbit_camera->facing + y_sens * diff_y));

		if (glm::dot(cross, new_cross) > 0.0)
			orbit_camera->facing += y_sens * diff_y;

		orbit_camera->facing = glm::normalize(orbit_camera->facing);
	}

	lastX = xpos;
	lastY = ypos;
}

static void orbit_scroll_callback(GLFWwindow* w, double xoffset, double yoffset) {
	const double dist_multiplier = 1.05;
	const double lower_limit = 2.0 * MARS_RADIUS;
	const double upper_limit = 40.0 * MARS_RADIUS;

	if (yoffset > 0.0)
		dist_to_mars /= dist_multiplier;
	else if (yoffset < 0.0)
		dist_to_mars *= dist_multiplier;

	// Clamp
	if (dist_to_mars < lower_limit)
		dist_to_mars = lower_limit;
	else if (dist_to_mars > upper_limit)
		dist_to_mars = upper_limit;
}

