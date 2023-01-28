// Ricardas Navickas 2020
#ifndef ORBIT_SCENE_H
#define ORBIT_SCENE_H

#include "core/shader.h"

void init_orbit_scene(Shader* world_shader, Shader* light_shader);

// Sets up callbacks and sets wstate.current_scene to the orbit scene
void activate_orbit_scene();

// Updates the camera
void update_orbit_scene();

#endif
