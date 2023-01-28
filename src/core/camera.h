// Ricardas Navickas 2020
#ifndef CAMERA_H
#define CAMERA_H

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

class Camera {
public:
	Camera(glm::dvec3 initial_pos, glm::dvec3 facing_dir, glm::dvec3 up_dir, double vfov, double sens);
	~Camera();

	glm::dmat4 view_matrix();
	glm::dmat4 origin_view_matrix(); // view matrix assuming pos=glm:dvec3(0.0, 0.0, 0.0)
	glm::dmat4 perspective_matrix(double z_near, double z_far, double aspect_ratio);

	// Update camera orientation using mouse offsets
	void update_orientation_from_mouse(double xoffset, double yoffset);

	glm::dvec3 position;

	// Camera orientation
	glm::dvec3 facing, up, right;

private:
	// Vertical field of vision
	double fov;

	double sensitivity; // mouse sensitivity
};

#endif
