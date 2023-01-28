// Ricardas Navickas 2020
#ifndef AUTOPILOT_H
#define AUTOPILOT_H

#include "core/core.h"
#include "lua.hpp"

struct AutopilotProgram {
	std::string path;
	bool loaded;

	lua_State* L;
};

AutopilotProgram make_autopilot_program(std::string lua_path);
void run_autopilot_program(AutopilotProgram* ap);

void attitude_stabilization_step(Object* lander, double timestep);
void attitude_control_step(Object* lander, glm::dvec3 target, double timestep);
void surface_velocity_control_step(Object* mars, Object* lander, double target, double timestep);

#endif
