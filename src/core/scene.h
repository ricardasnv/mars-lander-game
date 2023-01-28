// Ricardas Navickas 2020
#ifndef SCENE_H
#define SCENE_H

#include "camera.h"
#include "object.h"
#include "shader.h"
#include "model.h"
#include "glm/glm.hpp"

/*
 * Scenes are rendered in two passes using different perspective matrices to provide
 * sufficient depth buffer precision both for objects close to the camera and objects
 * ~10^5 km away from the camera. Some overlap is needed to avoid artifacts at the
 * transition boundary.
 * First pass: Z_TRANSITION / Z_OVERLAP_FACTOR to Z_FAR
 * Second pass: Z_NEAR to Z_TRANSITION * Z_OVERLAP_FACTOR
 */
#define Z_NEAR           0.005f
#define Z_TRANSITION     1000.0f
#define Z_FAR            1000.0f * 3386.0f //1000*MARS_RADIUS
#define Z_OVERLAP_FACTOR 1.05f

class Scene {
public:
	Scene(Camera* c, Shader* ws, Shader* ls);
	~Scene();

	void add_object(Object* obj);
	void add_nofx_object(Object* obj);
	void add_light(Object* obj, glm::vec3 color);

	void remove_object(Object* obj);
	void remove_nofx_object(Object* obj);

	void render();

	// If true, every object is rendered as a wireframe
	bool render_wireframe;

	Camera* camera;

private:
	void render_zrange(float z_near, float z_far);

	// Contents of scene
	std::vector<Object*> objects; // objects to draw (using world shader)
	std::vector<Object*> nofx_objects; // objects to draw (using world_nofx shader)
	std::vector<Object*> lights;  // light objects to draw (using world_nofx shader)

	std::vector<glm::vec3> light_pos;
	std::vector<glm::vec3> light_color;

	Shader* world_shader;
	Shader* world_nofx_shader;
	float time_taken;
};

#endif
