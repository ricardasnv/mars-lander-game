// Ricardas Navickas 2020
#include "scene.h"
#include "graphics.h"

#include <iostream>

Scene::Scene(Camera* c, Shader* ws, Shader* ls) {
	camera = c;
	world_shader = ws;
	world_nofx_shader = ls;
	render_wireframe = false;
}

Scene::~Scene() {}

void Scene::add_object(Object* obj) {
	objects.push_back(obj);
}

void Scene::add_nofx_object(Object* obj) {
	nofx_objects.push_back(obj);
}

void Scene::add_light(Object* obj, glm::vec3 color) {
	lights.push_back(obj);

	light_pos.push_back(obj->position);
	light_color.push_back(color);
}

void Scene::remove_object(Object* obj) {
	for (unsigned int i = 0; i < objects.size(); i++) {
		if (objects[i] == obj) objects.erase(objects.begin() + i);
	}
}

void Scene::remove_nofx_object(Object* obj) {
	for (unsigned int i = 0; i < nofx_objects.size(); i++) {
		if (nofx_objects[i] == obj) nofx_objects.erase(nofx_objects.begin() + i);
	}
}

void Scene::render() {
	float frame_start_time = glfwGetTime();

	// Render scene in two passes
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	render_zrange(Z_TRANSITION / Z_OVERLAP_FACTOR, Z_FAR);
	glClear(GL_DEPTH_BUFFER_BIT);
	render_zrange(Z_NEAR, Z_TRANSITION * Z_OVERLAP_FACTOR);
	
	time_taken = glfwGetTime() - frame_start_time;
}

void Scene::render_zrange(float z_near, float z_far) {
	// **** Draw light sources ****
	world_nofx_shader->use();
	world_nofx_shader->setmat4("view", camera->origin_view_matrix()); // Put camera at the origin
	world_nofx_shader->setmat4("projection", camera->perspective_matrix(z_near, z_far, (float)wstate.window_width / wstate.window_height));

	for (unsigned int i = 0; i < lights.size(); i++) {
		if (lights[i] == NULL) continue;

		glm::mat4 light_mmat(1.0f);
		light_mmat = glm::translate(light_mmat, light_pos[i]);
		light_mmat = glm::scale(light_mmat, glm::vec3(0.1f));

		world_nofx_shader->setmat4("model", light_mmat);

		// Draw world with the camera at the origin to increase precision of 32bit floats
		if (render_wireframe)
			lights[i]->draw_model_wire(world_nofx_shader, camera->position);
		else
			lights[i]->draw_model_solid(world_nofx_shader, camera->position);
	}

	// **** Draw world ****
	world_shader->use();
	world_shader->setmat4("view", camera->origin_view_matrix());
	world_shader->setmat4("projection", camera->perspective_matrix(z_near, z_far, (float)wstate.window_width / wstate.window_height));
	world_shader->setb("apply_lighting", true);
	world_shader->setb("apply_fog", true);

	// Set light uniforms
	world_shader->seti("num_of_lights", lights.size());
	world_shader->set3fv("light_pos", light_pos.size(), light_pos);
	world_shader->set3fv("light_color", light_color.size(), light_color);

	// Camera position for specular lighting
	world_shader->set3f("view_pos", camera->position.x, camera->position.y, camera->position.z);

	for (unsigned int i = 0; i < objects.size(); i++) {
		if (objects[i] == NULL) continue;

		if (render_wireframe)
			objects[i]->draw_model_wire(world_shader, camera->position);
		else
			objects[i]->draw_model_solid(world_shader, camera->position);
	}

	// **** Draw nofx objects ****
	world_shader->use();
	world_shader->setb("apply_lighting", false);
	world_shader->setb("apply_fog", false);

	for (unsigned int i = 0; i < nofx_objects.size(); i++) {
		if (nofx_objects[i] == NULL) continue;

		if (render_wireframe)
			nofx_objects[i]->draw_model_wire(world_shader, camera->position);
		else
			nofx_objects[i]->draw_model_solid(world_shader, camera->position);
	}
}

