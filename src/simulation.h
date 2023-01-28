// Ricardas Navickas 2020
#ifndef SIMULATION_H
#define SIMULATION_H

#include "core/object.h"

extern struct SimulationState {
	double time;
	double timestep;
	bool paused;
	bool landed, crashed;

	int scenario_id;

	bool me_manual_control;
	bool rcs_manual_control;

	// Objects to simulate (other than lander & mars)
	std::vector<Object*> obj;
} simstate;

void init_simulation(double physics_timestep);

void add_sim_object(Object* obj);
void remove_sim_object(Object* obj);

// Do n physics updates
void simulation_step();

#endif
