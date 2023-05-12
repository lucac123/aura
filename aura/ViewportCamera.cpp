#include "ViewportCamera.h"

Camera::Camera(glm::vec3 target, float distance, float phi, float theta)
	: target(target), distance(distance), phi(phi), theta(theta), up(glm::vec3(0,1,0)), speed(2.5f), zoom(45.0f), sensitivity(0.01f) {
	update_vectors();
}

glm::mat4 Camera::getViewMatrix() {
	return glm::lookAt(position, position + front, up);
}

void Camera::processMovement(float delta_x, float delta_y) {
	
}
void Camera::processPan(float delta_x, float delta_y) {
	delta_x *= sensitivity;
	delta_y *= sensitivity;

	theta -= delta_x;
	phi += delta_y;

	update_vectors();
}


void Camera::update_vectors() {
	position.x = distance * sin(phi) * sin(theta);
	position.y = distance * cos(phi);
	position.z = distance * sin(phi) * cos(theta);
	front = glm::normalize(target - position);

	right.x = cos(theta);
	right.z = -sin(theta);
	right.y = 0;

	up = glm::cross(right, front);
}