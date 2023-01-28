// Ricardas Navickas 2020
#ifndef MARS_H
#define MARS_H

#include "core/object.h"

// Unit system: kilometer/kilogram/second
// All forces are in kilonewtons (kilogram * kilometer / second^2)
#define MARS_RADIUS 3386.0f // kilometers
#define MARS_MASS 6.42e23 // kilograms
#define MARS_DAY 88642.65f // seconds

Object* make_mars_object();                                  // Spherical low-detail mars object
Object* make_mars_near_object(Object* mars, Object* lander); // Flat high-detail mars object

// Move the near-object under the lander
void update_mars_near_object(Object* near_mars, Object* mars, Object* lander);

double mars_surface_height(Object* mars, glm::dvec3 direction); // Radius in the given direction from the center
glm::vec3 mars_surface_color(glm::dvec3 direction);  // Color in the given direction from the center
glm::dvec3 mars_surface_velocity(Object const* mars, glm::dvec3 pos); // surface velocity under pos
glm::dvec3 mars_wind_velocity(Object const* mars, glm::dvec3 pos, double time); // wind velocity at pos

// Returns atmosphere density at pos (in kg/km^3)
double mars_atm_density(Object const* mars, glm::dvec3 pos);

#endif
