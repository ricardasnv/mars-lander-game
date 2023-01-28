// Ricardas Navickas 2020
#include "camera.h"
#include "error.h"
#include <iostream>

Camera::Camera(glm::dvec3 initial_pos, glm::dvec3 facing_dir, glm::dvec3 up_dir, double vfov, double sens) {
	position = initial_pos;
	facing = glm::normalize(facing_dir);
	up = glm::normalize(up_dir);
	right = glm::normalize(glm::cross(facing, up));

	fov = vfov;

	sensitivity = sens;
}

Camera::~Camera() {}

glm::dmat4 Camera::view_matrix() {
	return glm::lookAt(position, position + facing, up);
}

glm::dmat4 Camera::origin_view_matrix() {
	return glm::lookAt(glm::dvec3(0.0), facing, up);
}

glm::dmat4 Camera::perspective_matrix(double z_near, double z_far, double aspect_ratio) {
	return glm::perspective(glm::radians(fov), aspect_ratio, z_near, z_far);
}

void Camera::update_orientation_from_mouse(double xoffset, double yoffset) {
	glm::dvec3 diff = xoffset * right + yoffset * up;
	facing += sensitivity * diff;
	facing = glm::normalize(facing);
	up = glm::normalize(glm::cross(right, facing));
}

