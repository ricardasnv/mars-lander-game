// Ricardas Navickas 2020
#ifndef GLOBAL_H
#define GLOBAL_H

#include "core/object.h"
#include "core/shader.h"
#include "lander.h"
#include "mars.h"
#include "sun.h"

// Objects
extern Object* lander;
extern Object* lander_exhaust;
extern Object* lander_parachute;
extern std::vector<Object*> lander_debris;

extern Object* mars;
extern Object* sun;

// Models
extern Model* lander_default_model;
extern Model* lander_crashed_model;
extern Model* lander_debris1_model;
extern Model* lander_debris2_model;

// Shader programs
extern Shader* world_shader;
extern Shader* world_nofx_shader;

void init_global_vars();

#endif
