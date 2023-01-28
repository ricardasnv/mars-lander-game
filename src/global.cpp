// Ricardas Navickas 2020
#include "global.h"

Object* lander = NULL;
Object* lander_parachute = NULL;
Object* lander_exhaust = NULL;
std::vector<Object*> lander_debris;

Object* mars = NULL;
Object* sun = NULL;

Model* lander_default_model = NULL;
Model* lander_crashed_model = NULL;
Model* lander_debris1_model = NULL;
Model* lander_debris2_model = NULL;

Shader* world_shader = NULL;
Shader* world_nofx_shader = NULL;

void init_global_vars() {
	lander = make_lander_object();
	lander_parachute = make_parachute_object();
	lander_exhaust = make_exhaust_object();
	mars = make_mars_object();
	sun = make_sun_object();

	lander_default_model = lander->model;

	Mesh* lander_crashed_mesh = load_stl_mesh("models/lander_crashed.stl", 0.4f, 0.4f, 0.4f);
	if (lander_crashed_mesh != NULL) {
		transform_mesh(lander_crashed_mesh, glm::scale(glm::dmat4(1.0), glm::dvec3(0.001, 0.001, 0.001)));
		lander_crashed_model = new Model(lander_crashed_mesh, GL_TRIANGLES);
	} else {
		lander_crashed_model = lander->model;
	}

	Mesh* lander_debris1_mesh = load_stl_mesh("models/lander_debris1.stl", 0.4f, 0.4f, 0.4f);
	if (lander_debris1_mesh != NULL) {
		transform_mesh(lander_debris1_mesh, glm::scale(glm::dmat4(1.0), glm::dvec3(0.001, 0.001, 0.001)));
		lander_debris1_model = new Model(lander_debris1_mesh, GL_TRIANGLES);

		// Debris objects
		lander_debris.push_back(new Object(lander_debris1_model, glm::dvec3(0.0), 1.0f, 128));
		lander_debris.push_back(new Object(lander_debris1_model, glm::dvec3(0.0), 1.0f, 128));
	} else {
		lander_debris1_model = NULL;
	}

	Mesh* lander_debris2_mesh = load_stl_mesh("models/lander_debris2.stl", 0.4f, 0.4f, 0.4f);
	if (lander_debris2_mesh != NULL) {
		transform_mesh(lander_debris2_mesh, glm::scale(glm::dmat4(1.0), glm::dvec3(0.001, 0.001, 0.001)));
		lander_debris2_model = new Model(lander_debris2_mesh, GL_TRIANGLES);

		// Debris objects
		lander_debris.push_back(new Object(lander_debris2_model, glm::dvec3(0.0), 1.0f, 128));
	} else {
		lander_debris2_model = NULL;
	}

	world_shader = new Shader("shaders/world.v.glsl", "shaders/world.f.glsl");
	world_nofx_shader = new Shader("shaders/world_nofx.v.glsl", "shaders/world_nofx.f.glsl");
}

