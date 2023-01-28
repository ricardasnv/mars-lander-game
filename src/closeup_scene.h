// Ricardas Navickas 2020
#ifndef CLOSEUP_SCENE_H
#define CLOSEUP_SCENE_H

#include "core/shader.h"

void init_closeup_scene(Shader* world_shader, Shader* light_shader);

// Sets up callbacks and sets wstate.current_scene to the closeup scene
void activate_closeup_scene();

// Updates the camera
void update_closeup_scene();

#endif
