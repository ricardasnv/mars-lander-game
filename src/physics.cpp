// Ricardas Navickas 2020
#include "physics.h"
#include "mars.h"

glm::dvec3 grav_force(Object const* obj1, Object const* obj2) {
	glm::dvec3 dist = obj1->position - obj2->position;
	double dist_squared = glm::dot(dist, dist);
	return glm::normalize(dist) * GRAVITY * obj1->mass * obj2->mass / dist_squared;
}

glm::dvec3 drag_force(Object const* obj, glm::dvec3 air_vel, double air_density, double drag_coef, double frontal_area) {
	glm::dvec3 relvel = obj->velocity - air_vel; // relative velocity

	if (glm::length(relvel) <= 1e-8)
		return glm::dvec3(0.0f);
	
	glm::dvec3 drag = -0.5 * glm::normalize(relvel) * frontal_area * drag_coef * air_density * glm::dot(relvel, relvel);
	return drag;
}

double orbit_energy(Object const* primary, Object const* secondary) {
	glm::dvec3 dist = primary->position - secondary->position;
	double vel2 = glm::dot(secondary->velocity, secondary->velocity);
	double kinetic = 0.5 * secondary->mass * vel2;
	double potential = -1.0 * GRAVITY * primary->mass * secondary->mass / glm::length(dist);
	return kinetic + potential;
}

double orbit_semi_major_axis(Object const* primary, Object const* secondary) {
	return -1.0 * (GRAVITY * primary->mass * secondary->mass) / (2 * orbit_energy(primary, secondary));
}

double orbit_eccentricity(Object const* primary, Object const* secondary) {
	glm::dvec3 dist = secondary->position - primary->position;
	double specific_angular_momentum = glm::length(glm::cross(dist, secondary->velocity));
	return sqrt(1.0 - specific_angular_momentum * specific_angular_momentum / (GRAVITY * primary->mass * orbit_semi_major_axis(primary, secondary)));
}

double periapsis_radius(Object const* primary, Object const* secondary) {
	return orbit_semi_major_axis(primary, secondary) * (1 - orbit_eccentricity(primary, secondary));
}

double apoapsis_radius(Object const* primary, Object const* secondary) {
	return orbit_semi_major_axis(primary, secondary) * (1 + orbit_eccentricity(primary, secondary));
}

