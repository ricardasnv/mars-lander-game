// Ricardas Navickas 2020
#include "core/core.h"
#include "mars.h"
#include "noise.h"

#include <cmath>

static const glm::vec3 mars_base_color(0.63f, 0.33f, 0.22f);

Object* make_mars_object() {
	Mesh* mars_mesh = new Mesh;
	*mars_mesh = make_ico_sphere_mesh(4, mars_base_color.x, mars_base_color.y, mars_base_color.z);
	transform_mesh(mars_mesh, glm::scale(glm::mat4(1.0), glm::vec3(MARS_RADIUS)));

	// Base color
	for (int i = 0; i < num_of_vertices(mars_mesh); i++) {
		set_vertex_color(mars_mesh, i, mars_surface_color(get_vertex_coords(mars_mesh, i)));
	}

	// Add color noise
	Noise3d noise(400.0f); 
	for (int i = 0; i < num_of_vertices(mars_mesh); i++) {
		const float coef = 1 / 20.0f;
		float val = noise.get_value(get_vertex_coords(mars_mesh, i), 2);

		glm::vec3 color = get_vertex_color(mars_mesh, i);

		color.x = glm::clamp(color.x + coef * val, 0.0f, 1.0f);
		color.y = glm::clamp(color.y + coef * val, 0.0f, 1.0f);
		color.z = glm::clamp(color.z + coef * val, 0.0f, 1.0f);

		set_vertex_color(mars_mesh, i, color);
	}

	// Add normal noise
	Noise3d x_noise(400.0f);
	Noise3d y_noise(400.0f);
	Noise3d z_noise(400.0f);
	for (int i = 0; i < num_of_vertices(mars_mesh); i++) {
		const float coef = 1 / 10.0f;
		glm::vec3 normal = get_vertex_normal(mars_mesh, i);
		glm::vec3 pos = get_vertex_coords(mars_mesh, i);

		normal.x = normal.x + coef * x_noise.get_value(pos);
		normal.y = normal.y + coef * y_noise.get_value(pos);
		normal.z = normal.z + coef * z_noise.get_value(pos);
		normal = glm::normalize(normal);

		set_vertex_normal(mars_mesh, i, normal);
	}

	Model* mars_model = new Model(mars_mesh, GL_TRIANGLES);
	Object* mars = new Object(mars_model, glm::dvec3(0.0, 0.0, 0.0), 0.1f, 2);
	mars->mass = MARS_MASS;

	return mars;
}

Object* make_mars_near_object(Object* mars, Object* lander) {
	Mesh* mars_flat_mesh = new Mesh;
	glm::vec3 color = mars_surface_color(lander->position - mars->position);
	*mars_flat_mesh = make_square_mesh(100, color.x, color.y, color.z);
	transform_mesh(mars_flat_mesh, glm::scale(glm::dmat4(1.0), glm::dvec3(8.0, 1.0, 8.0))); // 20km x 20km

	Model* mars_flat_model = new Model(mars_flat_mesh, GL_TRIANGLES);
	Object* mars_flat = new Object(mars_flat_model, glm::dvec3(0.0, 0.0, 0.0), 0.1f, 2);

	//update_mars_near_object(mars_flat, mars, lander);

	return mars_flat;
}

void update_mars_near_object(Object* near_mars, Object* mars, Object* lander) {
	const glm::vec3 base_color = mars_surface_color(lander->position - mars->position);
	Mesh* m = near_mars->model->mesh;

	near_mars->position = glm::normalize(lander->position - mars->position) * double(MARS_RADIUS);
	near_mars->orient_towards(lander->position - mars->position);

	for (int i = 0; i < num_of_vertices(m); i++) {
		glm::dvec3 coords = get_vertex_coords(m, i);
		glm::dvec3 vertex_world_coords = near_mars->position + near_mars->attitude_matrix * coords;

		// Height
		coords.y = mars_surface_height(mars, vertex_world_coords);
		set_vertex_coords(m, i, coords);

		// Color
		glm::vec3 color = base_color + glm::vec3(coords.y);
		set_vertex_color(m, i, color);
	}

	near_mars->model->reload_mesh();
}

double mars_surface_height(Object* mars, glm::dvec3 direction) {
	if (glm::length(direction) < 1e-8) return MARS_RADIUS;
	static Noise3d height_noise(2.0f);
	double noise_amplitude = 0.2;
	glm::dvec3 surface_pos = glm::normalize(glm::inverse(mars->attitude_matrix) * direction) * double(MARS_RADIUS);
	return noise_amplitude * height_noise.get_value(surface_pos, 3);
}

glm::vec3 mars_surface_color(glm::dvec3 direction) {
	glm::vec3 color = mars_base_color;
	const glm::dvec3 surface_pos = glm::normalize(direction) * double(MARS_RADIUS);
	const glm::vec3 ice_color(0.9f, 0.9f, 0.9f);
	const float pole_y_threshold = 0.8 * MARS_RADIUS;

	if (std::abs(surface_pos.y) >= pole_y_threshold) {
		float coef = glm::clamp(10.0 * (std::abs(surface_pos.y) - pole_y_threshold) / pole_y_threshold, 0.0, 1.0);

		color.x = glm::clamp(color.x * (1.0f - coef) + ice_color.x * coef, 0.0f, 1.0f);
		color.y = glm::clamp(color.y * (1.0f - coef) + ice_color.y * coef, 0.0f, 1.0f);
		color.z = glm::clamp(color.z * (1.0f - coef) + ice_color.z * coef, 0.0f, 1.0f);
	}

	return color;
}

glm::dvec3 mars_surface_velocity(Object const* mars, glm::dvec3 pos) {
	glm::dvec3 surface_pos = (double)MARS_RADIUS * glm::normalize(pos);
	return glm::cross(mars->ang_velocity, surface_pos);
}

glm::dvec3 mars_wind_velocity(Object const* mars, glm::dvec3 pos, double time) {
	glm::dvec3 surface_pos = (double)MARS_RADIUS * glm::normalize(pos);

	static Noise1d wind_speed(10.0); // cell size - 10 seconds
	static Noise1d wind_x(100.0);
	static Noise1d wind_z(100.0);

	const double wind_speed_amplitude = 0.05; // 50 m/s
	double wind_speed_val = wind_speed.get_value(time);
	double actual_speed = wind_speed_amplitude * std::abs(wind_speed_val);
	glm::dvec3 wind_dir = glm::dvec3(wind_x.get_value(time), 0.0, wind_z.get_value(time));
	glm::dvec3 wind_velocity(0.0);

	if (glm::length(wind_dir) < 1e-8) return mars_surface_velocity(mars, pos);

	wind_velocity = actual_speed * glm::normalize(wind_dir);

	// Transform wind velocity to make it parallel to mars' surface
	double angle = glm::acos(glm::dot(glm::normalize(surface_pos), glm::dvec3(0.0, 1.0, 0.0)));
	glm::dvec3 axis = glm::cross(surface_pos, glm::dvec3(0.0, 1.0, 0.0));

	if (glm::length(axis) > 1e-8) {
		glm::dmat3 transform = glm::rotate(glm::dmat4(1.0), angle, axis);
		wind_velocity = transform * wind_velocity;
	}

	return mars_surface_velocity(mars, pos) + wind_velocity;
}

double mars_atm_density(Object const* mars, glm::dvec3 pos) {
	double altitude = glm::length(pos - mars->position) - MARS_RADIUS;
	if (altitude > 200.0) return 0.0;
	return 0.017e9 * exp(-altitude / 11.0); // 0.017e9 kg/km^3
}

