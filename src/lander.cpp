// Ricardas Navickas 2020
#include "lander.h"
#include "noise.h"
#include "global.h"

static const glm::vec3 exhaust_color(0.8f, 0.8f, 0.1f);

Object* make_lander_object() {
	Mesh* lander_mesh = load_stl_mesh("models/lander.stl", 0.4f, 0.4f, 0.4f);

	// If failed to load from file, use simple cone mesh
	if (lander_mesh == NULL) {
		lander_mesh = new Mesh;
		*lander_mesh = make_truncated_cone_mesh(30, 0.5f, 0.4f, 0.4f, 0.4f);
		transform_mesh(lander_mesh, glm::scale(glm::dmat4(1.0), glm::dvec3(1.0, 0.5, 1.0)));
	}

	transform_mesh(lander_mesh, glm::scale(glm::dmat4(1.0), glm::dvec3(0.001, 0.001, 0.001))); // from meters to kilometers

	// Add color noise
	Noise3d noise(0.0001f);
	for (int i = 0; i < num_of_vertices(lander_mesh); i++) {
		float val = noise.get_value(get_vertex_coords(lander_mesh, i)) / 10.0f;
		glm::vec3 color = get_vertex_color(lander_mesh, i);

		color.x = glm::clamp(color.x + val, 0.0f, 1.0f);
		color.y = glm::clamp(color.y + val, 0.0f, 1.0f);
		color.z = glm::clamp(color.z + val, 0.0f, 1.0f);

		set_vertex_color(lander_mesh, i, color);
	}

	Model* lander_model = new Model(lander_mesh, GL_TRIANGLES);
	Object* lander = new Object(lander_model, glm::dvec3(0.0), 1.0f, 128);

	reset_lander_attributes(lander);

	return lander;
}

Object* make_parachute_object() {
	Mesh* parachute_mesh = load_stl_mesh("models/lander_parachute.stl", 0.3f, 0.3f, 0.6f);

	if (parachute_mesh == NULL) {
		parachute_mesh = new Mesh;
		*parachute_mesh = make_truncated_cone_mesh(30, 0.01f, 0.3f, 0.3f, 0.6f);
		transform_mesh(parachute_mesh, glm::translate(glm::dmat4(1.0), glm::dvec3(0.0, 0.003, 0.0)));
		transform_mesh(parachute_mesh, glm::scale(glm::dmat4(1.0), glm::dvec3(2.0, 0.2, 2.0)));
	}

	transform_mesh(parachute_mesh, glm::scale(glm::dmat4(1.0), glm::dvec3(0.001, 0.001, 0.001)));

	Model* parachute_model = new Model(parachute_mesh, GL_TRIANGLES);
	Object* parachute = new Object(parachute_model, glm::dvec3(0.0), 1.0f, 8);

	return parachute;
}

Object* make_exhaust_object() {
	Mesh* exhaust_mesh = new Mesh;
	*exhaust_mesh = make_truncated_cone_mesh(15, 0.0f, 0.8f, 0.8f, 0.1f);
	transform_mesh(exhaust_mesh, glm::scale(glm::dmat4(1.0), glm::dvec3(0.00025, -EXHAUST_MAX_LENGTH, 0.00025)));

	Model* exhaust_model = new Model(exhaust_mesh, GL_TRIANGLES);
	Object* exhaust = new Object(exhaust_model, glm::dvec3(0.0), 1.0f, 8);

	return exhaust;
}

void reset_lander_attributes(Object* lander) {
	lander->attribute["dry_mass"] = 150.0;
	lander->attribute["fuel_level"] = 1.0; // 0.0 - empty, 1.0 - full
	lander->attribute["fuel_density"] = 1.0;
	lander->attribute["fuel_capacity"] = 50.0;

	lander->attribute["me_max_fuel_rate"] = 0.5;
	lander->attribute["me_exhaust_vel"] = 3.0; // km/s
	lander->attribute["me_throttle"] = 0.0;

	lander->attribute["rcs_max_fuel_rate"] = 0.1;
	lander->attribute["rcs_exhaust_vel"] = 2.0; // km/s
	lander->attribute["rcs_throttle"] = 0.0;
	lander->attribute["rcs_axis_x"] = 0.0;
	lander->attribute["rcs_axis_y"] = 1.0;
	lander->attribute["rcs_axis_z"] = 0.0;

	lander->attribute["frontal_area"] = M_PI * 0.001 * 0.001;
	lander->attribute["parachute_status"] = 0.0; // 0.0 - not deployed, 0.0-1.0 - deployed, >1.0 - destroyed

	double fuel_mass = lander->attribute["fuel_level"] * lander->attribute["fuel_capacity"] * lander->attribute["fuel_density"];
	lander->mass = lander->attribute["dry_mass"] + fuel_mass;
	lander->moment_of_inertia = lander->mass * 1e-6; // approximate, assuming radius is about 1e-3 km
}

void setup_rcs(Object* lander, glm::dvec3 axis, double throttle) {
	lander->attribute["rcs_axis_x"] = axis.x;
	lander->attribute["rcs_axis_y"] = axis.y;
	lander->attribute["rcs_axis_z"] = axis.z;
	lander->attribute["rcs_throttle"] = glm::clamp(throttle, -1.0, 1.0);
}

void setup_main_engine(Object* lander, double throttle) {
	lander->attribute["me_throttle"] = glm::clamp(throttle, 0.0, 1.0);
}

void fire_engines(Object* lander, double timestep) {
	// If no fuel left, do nothing
	if (lander->mass <= lander->attribute["dry_mass"]) {
		lander->mass = lander->attribute["dry_mass"];
		return;
	}

	// Fire RCS (assuming thrusters are 1m away from the center of mass)
	double thruster_force = std::abs(lander->attribute["rcs_throttle"]) * lander->attribute["rcs_max_fuel_rate"] * lander->attribute["rcs_exhaust_vel"];
	glm::dvec3 axis(lander->attribute["rcs_axis_x"], lander->attribute["rcs_axis_y"], lander->attribute["rcs_axis_z"]);
	lander->net_moment += 1e-3 * thruster_force * glm::normalize(axis);

	// Fire main engine
	glm::dvec3 up_vector = lander->attitude_matrix * glm::dvec3(0.0, 1.0, 0.0);
	lander->net_force += up_vector * lander->attribute["me_throttle"] * lander->attribute["me_max_fuel_rate"] * lander->attribute["me_exhaust_vel"];

	// Update mass
	lander->mass -= lander->attribute["me_throttle"] * lander->attribute["me_max_fuel_rate"] * timestep;
	lander->mass -= lander->attribute["rcs_throttle"] * lander->attribute["rcs_max_fuel_rate"] * timestep;
	lander->moment_of_inertia = lander->mass * 1e-6; // approximate, assuming radius is about 1e-3 km

	// Update fuel-related attributes
	lander->attribute["fuel_level"] = (lander->mass - lander->attribute["dry_mass"]) / (lander->attribute["fuel_capacity"] * lander->attribute["fuel_density"]);
}

void deploy_parachute(Object* lander) {
	// If parachute is destroyed, do nothing
	if (lander->attribute["parachute_status"] > 1.0)
		return;
	
	// If moving too fast, destroy parachute
	if (glm::length(lander->velocity - mars_surface_velocity(mars, lander->position)) > PARACHUTE_MAX_VELOCITY) {
		cut_parachute(lander);
		return;
	}

	lander->attribute["parachute_status"] = 1.0;
	lander->attribute["frontal_area"] = 5 * M_PI * 0.001 * 0.001;
}

void cut_parachute(Object* lander) {
	lander->attribute["parachute_status"] = 2.0; // lose parachute
	lander->attribute["frontal_area"] = M_PI * 0.001 * 0.001;
}

double descent_rate(Object* lander, Object* mars) {
	return glm::dot(-lander->velocity, glm::normalize(lander->position - mars->position));
}

double groundspeed(Object* lander, Object* mars) {
	return glm::length(lander->velocity + glm::normalize(lander->position - mars->position) * descent_rate(lander, mars) - mars_surface_velocity(mars, lander->position));
}

bool soft_landing(Object* lander) {
	return (descent_rate(lander, mars) <= 0.001) && (groundspeed(lander, mars) <= 0.001);
}

double lander_delta_v(Object* lander) {
	return lander->attribute["me_exhaust_vel"] * log(lander->mass / lander->attribute["dry_mass"]);
}

double lander_max_delta_v(Object* lander) {
	return lander->attribute["me_exhaust_vel"] * log((lander->attribute["dry_mass"] + lander->attribute["fuel_capacity"] * lander->attribute["fuel_density"]) / lander->attribute["dry_mass"]);
}

void update_exhaust_model(Object* lander, Object* exhaust) {
	static const Mesh full_length_exhaust_mesh = *exhaust->model->mesh;
	*exhaust->model->mesh = full_length_exhaust_mesh;
	transform_mesh(exhaust->model->mesh, glm::scale(glm::dmat4(1.0), glm::dvec3(1.0, lander->attribute["me_throttle"], 1.0)));
	exhaust->model->reload_mesh();
}

