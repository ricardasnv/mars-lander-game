// Ricardas Navickas 2020
#ifndef LANDER_H
#define LANDER_H

#include "core/object.h"
#include "physics.h"

#define EXHAUST_MAX_LENGTH 0.001
#define PARACHUTE_MAX_VELOCITY 0.5
#define BASE_COM_DIST 0.0007 // distance between lander base and center of mass

Object* make_lander_object();
Object* make_parachute_object();
Object* make_exhaust_object();

void reset_lander_attributes(Object* lander);

// Lander controls
void setup_rcs(Object* lander, glm::dvec3 axis, double throttle); // set RCS rotation axis and throttle
void setup_main_engine(Object* lander, double throttle);          // set main engine throttle
void fire_engines(Object* lander, double timestep);               // apply thrust and torque (as configured by setup_rcs() and setup_main_engine())
void deploy_parachute(Object* lander);
void cut_parachute(Object* lander);

double lander_delta_v(Object* lander);
double lander_max_delta_v(Object* lander);

// Helper functions
double descent_rate(Object* lander, Object* mars);
double groundspeed(Object* lander, Object* mars);
bool soft_landing(Object* lander); // Returns true if soft landing conditions are satisfied

// Exhaust effects
void update_exhaust_model(Object* lander, Object* exhaust);

#endif
