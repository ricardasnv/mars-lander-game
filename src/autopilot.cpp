// Ricardas Navickas 2020
#include "autopilot.h"
#include "lander.h"
#include "gui.h"
#include "pid.h"
#include "global.h"
#include "simulation.h"

// Lua helper functions
int TIME(lua_State* L);

int LANDER_ALT(lua_State* L);
int LANDER_VELOCITY(lua_State* L);
int LANDER_SURFACE_VELOCITY(lua_State* L);
int LANDER_GROUND_SPEED(lua_State* L);
int LANDER_DESCENT_RATE(lua_State* L);
int LANDER_DELTA_V(lua_State* L);
int LANDER_MAX_THRUST(lua_State* L);
int LANDER_WEIGHT(lua_State* L);
int LANDER_MASS(lua_State* L);

int PERIAPSIS_ALT(lua_State* L);
int APOAPSIS_ALT(lua_State* L);

// Lua autopilot actions
int MAINTAIN_ATTITUDE(lua_State* L);
int HOLD_PROGRADE(lua_State* L);
int HOLD_RETROGRADE(lua_State* L);
int HOLD_NORMAL(lua_State* L);
int HOLD_ANTINORMAL(lua_State* L);
int HOLD_RADIAL(lua_State* L);
int HOLD_ANTIRADIAL(lua_State* L);
int HOLD_SURFACE_PROGRADE(lua_State* L);
int HOLD_SURFACE_RETROGRADE(lua_State* L);

int SET_MAIN_ENGINE_THROTTLE(lua_State* L);
int MAINTAIN_SURFACE_VELOCITY(lua_State* L);
int DEPLOY_PARACHUTE(lua_State* L);

// Misc. Lua functions
int INFO(lua_State* L);
int PAUSE(lua_State* L);

// ==== ==== ==== ==== ==== ==== ==== ====

AutopilotProgram make_autopilot_program(std::string lua_path) {
	AutopilotProgram ap;

	ap.path = lua_path;

	ap.L = luaL_newstate();
	luaL_openlibs(ap.L);
	lua_register(ap.L, "TIME", TIME);
	lua_register(ap.L, "LANDER_ALT", LANDER_ALT);
	lua_register(ap.L, "LANDER_VELOCITY", LANDER_VELOCITY);
	lua_register(ap.L, "LANDER_SURFACE_VELOCITY", LANDER_SURFACE_VELOCITY);
	lua_register(ap.L, "LANDER_GROUND_SPEED", LANDER_GROUND_SPEED);
	lua_register(ap.L, "LANDER_DESCENT_RATE", LANDER_DESCENT_RATE);
	lua_register(ap.L, "LANDER_DELTA_V", LANDER_DELTA_V);
	lua_register(ap.L, "LANDER_MAX_THRUST", LANDER_MAX_THRUST);
	lua_register(ap.L, "LANDER_WEIGHT", LANDER_WEIGHT);
	lua_register(ap.L, "LANDER_MASS", LANDER_MASS);
	lua_register(ap.L, "PERIAPSIS_ALT", PERIAPSIS_ALT);
	lua_register(ap.L, "APOAPSIS_ALT", APOAPSIS_ALT);
	lua_register(ap.L, "MAINTAIN_ATTITUDE", MAINTAIN_ATTITUDE);
	lua_register(ap.L, "HOLD_PROGRADE", HOLD_PROGRADE);
	lua_register(ap.L, "HOLD_RETROGRADE", HOLD_RETROGRADE);
	lua_register(ap.L, "HOLD_NORMAL", HOLD_NORMAL);
	lua_register(ap.L, "HOLD_ANTINORMAL", HOLD_ANTINORMAL);
	lua_register(ap.L, "HOLD_RADIAL", HOLD_RADIAL);
	lua_register(ap.L, "HOLD_ANTIRADIAL", HOLD_ANTIRADIAL);
	lua_register(ap.L, "HOLD_SURFACE_PROGRADE", HOLD_SURFACE_PROGRADE);
	lua_register(ap.L, "HOLD_SURFACE_RETROGRADE", HOLD_SURFACE_RETROGRADE);
	lua_register(ap.L, "SET_MAIN_ENGINE_THROTTLE", SET_MAIN_ENGINE_THROTTLE);
	lua_register(ap.L, "MAINTAIN_SURFACE_VELOCITY", MAINTAIN_SURFACE_VELOCITY);
	lua_register(ap.L, "DEPLOY_PARACHUTE", DEPLOY_PARACHUTE);
	lua_register(ap.L, "INFO", INFO);
	lua_register(ap.L, "PAUSE", PAUSE);

	if (luaL_dofile(ap.L, lua_path.c_str()) != LUA_OK) {
		guistate.autopilot_message_log.push_back(std::string("[INFO] Lua error: ") + lua_tostring(ap.L, -1));
		ap.loaded = false;
	} else {
		guistate.autopilot_message_log.push_back("[INFO] Successfully loaded script from " + ap.path);
		ap.loaded = true;
	}

	return ap;
}

void run_autopilot_program(AutopilotProgram* ap) {
	lua_getglobal(ap->L, "STEP");

	if (!lua_isfunction(ap->L, 1)) {
		guistate.autopilot_message_log.push_back("[INFO] STEP() not defined. Unloading autopilot program.");
		lua_pop(ap->L, 1);
		ap->loaded = false;
		return;
	}

	if (lua_pcall(ap->L, 0, 0, 0) != LUA_OK) {
		guistate.autopilot_message_log.push_back(std::string("[INFO] Lua error: ") + lua_tostring(ap->L, -1));
		ap->loaded = false;
	}
}

void attitude_stabilization_step(Object* lander, double timestep) {
	static PIDController attitude_stabilizer(5.0, 0.0, 0.0);

	double error = glm::length(lander->ang_velocity);

	if (error < 1e-6) return;
	attitude_stabilizer.update(timestep, error);

	setup_rcs(lander, -lander->ang_velocity, attitude_stabilizer.output());
}

void attitude_control_step(Object* lander, glm::dvec3 target, double timestep) {
	static PIDController attitude_controller(5.0, 0.0, 0.0);
	static PIDController attitude_stabilizer(5.0, 0.0, 0.0);

	if (glm::length(target) == 0) return;
	target = glm::normalize(target);

	// Stabilization
	double stabilizer_error = glm::length(lander->ang_velocity);

	attitude_stabilizer.update(timestep, stabilizer_error);

	double stabilizer_output = attitude_stabilizer.output();

	// Control
	glm::dvec3 lander_y = lander->attitude_matrix * glm::dvec3(0.0, 1.0, 0.0);
	glm::dvec3 controller_axis = glm::inverse(lander->attitude_matrix) * glm::cross(lander_y, target);
	double controller_error = glm::length(glm::cross(lander_y, target));

	attitude_controller.update(timestep, controller_error);

	double controller_output = attitude_controller.output();

	// Average PID outputs and fire RCS
	glm::dvec3 avg_axis = (stabilizer_output * (-lander->ang_velocity) + controller_output * controller_axis) / (stabilizer_output + controller_output);
	double avg_output = (stabilizer_output + controller_output) / 2;

	if (avg_output < 1e-6) return;

	setup_rcs(lander, avg_axis, avg_output);
}

// Assumes that lander is already pointed retrograde
void surface_velocity_control_step(Object* mars, Object* lander, double target, double timestep) {
	static PIDController velocity_controller(1000.0, 100.0, 0.0);

	double surface_velocity_magnitude = glm::length(lander->velocity - mars_surface_velocity(mars, lander->position));
	double error = surface_velocity_magnitude - target;

	velocity_controller.update(timestep, error);

	setup_main_engine(lander, velocity_controller.output());
}

// ==== ==== ==== ==== LUA FUNCTIONS ==== ==== ==== ====
int TIME(lua_State* L) {
	lua_pushnumber(L, simstate.time);
	return 1;
}

int LANDER_ALT(lua_State* L) {
	lua_pushnumber(L, glm::length(lander->position - mars->position) - MARS_RADIUS - mars_surface_height(mars, lander->position - mars->position) - BASE_COM_DIST);
	return 1;
}

int LANDER_VELOCITY(lua_State* L) {
	lua_pushnumber(L, glm::length(lander->velocity));
	return 1;
}

int LANDER_SURFACE_VELOCITY(lua_State* L) {
	lua_pushnumber(L, glm::length(lander->velocity - mars_surface_velocity(mars, lander->position)));
	return 1;
}

int LANDER_GROUND_SPEED(lua_State* L) {
	double descent_rate = glm::dot(-lander->velocity, glm::normalize(lander->position - mars->position));
	glm::dvec3 dist = lander->position - mars->position;
	double groundspeed = glm::length(lander->velocity + glm::normalize(dist) * descent_rate - mars_surface_velocity(mars, lander->position));
	lua_pushnumber(L, groundspeed);
	return 1;
}

int LANDER_DESCENT_RATE(lua_State* L) {
	double descent_rate = glm::dot(-lander->velocity, glm::normalize(lander->position - mars->position));
	lua_pushnumber(L, descent_rate);
	return 1;
}

int LANDER_DELTA_V(lua_State* L) {
	lua_pushnumber(L, lander_delta_v(lander));
	return 1;
}

int LANDER_MAX_THRUST(lua_State* L) {
	lua_pushnumber(L, lander->attribute["me_exhaust_vel"] * lander->attribute["me_max_fuel_rate"]);
	return 1;
}

int LANDER_WEIGHT(lua_State* L) {
	lua_pushnumber(L, glm::length(grav_force(mars, lander)));
	return 1;
}

int LANDER_MASS(lua_State* L) {
	lua_pushnumber(L, lander->mass);
	return 1;
}

int PERIAPSIS_ALT(lua_State* L) {
	lua_pushnumber(L, periapsis_radius(mars, lander) - MARS_RADIUS);
	return 1;
}

int APOAPSIS_ALT(lua_State* L) {
	lua_pushnumber(L, apoapsis_radius(mars, lander) - MARS_RADIUS);
	return 1;
}

int MAINTAIN_ATTITUDE(lua_State* L) {
	attitude_stabilization_step(lander, simstate.timestep);
	return 0;
}

int HOLD_PROGRADE(lua_State* L) {
	attitude_control_step(lander, lander->velocity, simstate.timestep);
	return 0;
}

int HOLD_RETROGRADE(lua_State* L) {
	attitude_control_step(lander, -lander->velocity, simstate.timestep);
	return 0;
}

int HOLD_NORMAL(lua_State* L) {
	glm::dvec3 normal = glm::cross(lander->velocity, lander->position - mars->position);
	attitude_control_step(lander, normal, simstate.timestep);
	return 0;
}

int HOLD_ANTINORMAL(lua_State* L) {
	glm::dvec3 normal = glm::cross(lander->velocity, lander->position - mars->position);
	attitude_control_step(lander, -normal, simstate.timestep);
	return 0;
}

int HOLD_RADIAL(lua_State* L) {
	attitude_control_step(lander, lander->position - mars->position, simstate.timestep);
	return 0;
}

int HOLD_ANTIRADIAL(lua_State* L) {
	attitude_control_step(lander, -lander->position + mars->position, simstate.timestep);
	return 0;
}

int HOLD_SURFACE_PROGRADE(lua_State* L) {
	attitude_control_step(lander, lander->velocity - mars_surface_velocity(mars, lander->position), simstate.timestep);
	return 0;
}

int HOLD_SURFACE_RETROGRADE(lua_State* L) {
	attitude_control_step(lander, -lander->velocity + mars_surface_velocity(mars, lander->position), simstate.timestep);
	return 0;
}

int SET_MAIN_ENGINE_THROTTLE(lua_State* L) {
	double throttle = glm::clamp(lua_tonumber(L, 1), 0.0, 1.0);
	setup_main_engine(lander, throttle);
	return 0;
}

int MAINTAIN_SURFACE_VELOCITY(lua_State* L) {
	double velocity_target = lua_tonumber(L, 1);
	surface_velocity_control_step(mars, lander, velocity_target, simstate.timestep);
	return 0;
}

int DEPLOY_PARACHUTE(lua_State* L) {
	deploy_parachute(lander);
	return 0;
}

int INFO(lua_State* L) {
	const char* str = lua_tostring(L, 1);
	std::string message = "[" + std::to_string(simstate.time) + "] " + str;
	guistate.autopilot_message_log.push_back(message);
	return 0;
}

int PAUSE(lua_State* L) {
	simstate.paused = true;
	return 0;
}

