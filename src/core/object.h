// Ricardas Navickas 2020
#ifndef OBJECT_H
#define OBJECT_H

#include "model.h"
#include "camera.h"
#include "shader.h"
#include "glm/glm.hpp"

#include <map>
#include <string>

class Object {
public:
	Object(Model* m, glm::dvec3 pos, float specular_coef, int specular_exp);
	~Object();

	void update(double delta_time); // Integrates velocity, acceleration etc. using the Verlet method
	void reset_integrator(); // Resets verlet_first_run to 1

	void draw_model_wire(Shader* shader);
	void draw_model_wire(Shader* shader, glm::dvec3 origin);
	void draw_model_solid(Shader* shader);
	void draw_model_solid(Shader* shader, glm::dvec3 origin);

	glm::dmat4 get_model_matrix();  // Calculate model matrix (transformation matrix from model space to world space)
	glm::dmat4 get_relative_model_matrix(glm::dvec3 origin);  // Calculate model matrix assuming the origin is at "origin"

	void orient_towards(glm::dvec3 v); // Align +Y in model space with v in world space

	// Position & derivatives
	glm::dvec3 position;
	glm::dvec3 velocity;
	glm::dvec3 acceleration;

	// Attitude & derivatives
	glm::dmat3 attitude_matrix;  // Rotation matrix
	glm::dvec3 ang_velocity;     // d/dt of attitude
	glm::dvec3 ang_acceleration; // d/dt of ang_velocity (r cross F / I)

	// Physical quantities
	double mass;
	double moment_of_inertia;
	glm::dvec3 net_force;
	glm::dvec3 net_moment;

	Model* model;

	// Optional extra attributes (for example, lander fuel level)
	std::map<std::string, double> attribute;

private:
	bool verlet_first_run;
	double prev_delta_time;
	glm::dvec3 prev_position;
	glm::dvec3 prev_attitude;

	// Specular lighting model
	float specular_coefficient;
	int specular_exponent;

	// Transform from local model space to world space
	glm::dmat4 model_matrix;
};

#endif
