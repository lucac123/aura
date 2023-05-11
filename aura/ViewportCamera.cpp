#include "ViewportCamera.h"

Camera::Camera(glm::vec3 position, glm::vec3 up, glm::vec3 front)
	: position(position), front(front), up(up), yaw(-135.0f), pitch(-45.0f), speed(2.5f), zoom(45.0f), sensitivity(0.1f) {
	update_vectors();
}

glm::mat4 Camera::getViewMatrix() {
	return glm::lookAt(position, position + front, up);
}

void Camera::processMovement(Movement direction, float delta_time) {
	float velocity = speed * delta_time;
	switch (direction) {
	case (FORWARD):
		position += front * velocity;
		break;
	case (BACKWARD):
		position -= front * velocity;
		break;
	case (RIGHT):
		position += right * velocity;
		break;
	case (LEFT):
		position -= right * velocity;
		break;
	}
}
void Camera::processPan(float delta_x, float delta_y) {
	delta_x *= sensitivity;
	delta_y *= sensitivity;

	yaw += delta_x;
	pitch += delta_y;

	update_vectors();
}


void Camera::update_vectors() {
	glm::vec3 t_front;
	t_front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	t_front.y = sin(glm::radians(pitch));
	t_front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	front = glm::normalize(t_front);
	
	right = glm::normalize(glm::cross(front, glm::vec3(0.0f, 1.0f, 0.0f)));
	up = glm::normalize(glm::cross(right, front));
}