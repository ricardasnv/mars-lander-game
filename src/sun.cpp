// Ricardas Navickas 2020
#include "sun.h"
#include "mars.h"

Object* make_sun_object() {
	Mesh* sun_mesh = new Mesh;
	*sun_mesh = make_uv_sphere_mesh(20, 10, 1.0f, 1.0f, 0.8f);
	transform_mesh(sun_mesh, glm::scale(glm::dmat4(1.0f), glm::dvec3(4.0 * MARS_RADIUS)));
	Model* sun_model = new Model(sun_mesh, GL_TRIANGLES);
	Object* sun = new Object(sun_model, glm::dvec3(500 * MARS_RADIUS, 0.0, 0.0), 0.0f, 1);

	return sun;
}

