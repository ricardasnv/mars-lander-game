// Ricardas Navickas 2020
#ifndef PHYSICS_H
#define PHYSICS_H

#include "core/object.h"

#define GRAVITY 6.67e-20 // km^3/kg/s^2

glm::dvec3 grav_force(Object const* primary, Object const* secondary); // grav. force acting on secondary
glm::dvec3 drag_force(Object const* obj, glm::dvec3 air_vel, double air_density, double drag_coef, double frontal_area); // air_vel is relative to origin

double orbit_energy(Object const* primary, Object const* secondary);
double orbit_semi_major_axis(Object const* primary, Object const* secondary);
double orbit_eccentricity(Object const* primary, Object const* secondary);
double periapsis_radius(Object const* primary, Object const* secondary);
double apoapsis_radius(Object const* primary, Object const* secondary);

#endif
