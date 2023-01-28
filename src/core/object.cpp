// Ricardas Navickas 2020
#include "object.h"
#include "error.h"
#include <cmath>

Object::Object(Model* m, glm::dvec3 pos, float specular_coef, int specular_exp) {
	model = m;

	position = pos;
	velocity = glm::dvec3(0.0, 0.0, 0.0);
	acceleration = glm::dvec3(0.0, 0.0, 0.0);

	attitude_matrix = glm::dmat3(1.0);
	ang_velocity = glm::dvec3(0.0, 0.0, 0.0);
	ang_acceleration = glm::dvec3(0.0, 0.0, 0.0);

	mass = 1.0;
	moment_of_inertia = 1.0;
	net_force = glm::dvec3(0.0);
	net_moment = glm::dvec3(0.0);

	model_matrix = get_model_matrix();

	prev_delta_time = 0.0;
	verlet_first_run = true;

	specular_coefficient = specular_coef;
	specular_exponent = specular_exp;
}

Object::~Object() {}

void Object::update(double delta_time) {
	if (delta_time == 0.0f) return;

	glm::dvec3 starting_position = position;

	acceleration = net_force / mass;
	ang_acceleration = net_moment / moment_of_inertia;

	if (verlet_first_run) {
		// If first run, use the Euler method
		velocity = velocity + acceleration * delta_time;
		position = position + velocity * delta_time;
	} else {
		// Otherwise use the Verlet method
		position = 2.0 * position - prev_position + acceleration * delta_time * delta_time;
		velocity = (position - prev_position) / (2 * delta_time);
	}

	ang_velocity = ang_velocity + ang_acceleration * delta_time;
	if (glm::length(ang_velocity) > 1e-8)
		attitude_matrix = glm::dmat3(glm::rotate(glm::dmat4(attitude_matrix), glm::length(ang_velocity) * delta_time, ang_velocity));

	prev_delta_time = delta_time;
	prev_position = starting_position;
	verlet_first_run = false;
}

void Object::reset_integrator() {
	verlet_first_run = true;
}

void Object::draw_model_solid(Shader* shader) {
	if (model == NULL) return;
	shader->setmat4("model", get_model_matrix());
	shader->setf("specular_coefficient", specular_coefficient);
	shader->seti("specular_exponent", specular_exponent);
	model->draw_solid();
}

void Object::draw_model_solid(Shader* shader, glm::dvec3 origin) {
	if (model == NULL) return;
	shader->setmat4("model", get_relative_model_matrix(origin));
	shader->setf("specular_coefficient", specular_coefficient);
	shader->seti("specular_exponent", specular_exponent);
	model->draw_solid();
}

void Object::draw_model_wire(Shader* shader) {
	if (model == NULL) return;
	shader->setmat4("model", get_model_matrix());
	shader->setf("specular_coefficient", specular_coefficient);
	shader->seti("specular_exponent", specular_exponent);
	model->draw_wire();
}

void Object::draw_model_wire(Shader* shader, glm::dvec3 origin) {
	if (model == NULL) return;
	shader->setmat4("model", get_relative_model_matrix(origin));
	shader->setf("specular_coefficient", specular_coefficient);
	shader->seti("specular_exponent", specular_exponent);
	model->draw_wire();
}

glm::dmat4 Object::get_model_matrix() {
	model_matrix = glm::dmat4(1.0f);
	model_matrix = glm::translate(model_matrix, position);
	model_matrix = model_matrix * glm::dmat4(attitude_matrix);
	return model_matrix;
}

glm::dmat4 Object::get_relative_model_matrix(glm::dvec3 origin) {
	model_matrix = glm::dmat4(1.0f);
	model_matrix = glm::translate(model_matrix, position - origin);
	model_matrix = model_matrix * glm::dmat4(attitude_matrix);
	return model_matrix;
}

void Object::orient_towards(glm::dvec3 v) {
	// if v is a zero vector
	if (glm::length(v) < 1e-8) return;

	v = glm::normalize(v);

	const glm::dvec3 y_vector = glm::dvec3(0.0, 1.0, 0.0);
	double angle = acos(glm::dot(v, y_vector));

	glm::dvec3 axis = glm::cross(v, y_vector);

	if (glm::length(axis) < 1e-8)
		attitude_matrix = glm::dmat4(1.0);
	else
		attitude_matrix = glm::rotate(glm::dmat4(1.0), -angle, axis);
}

