#pragma once
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

enum Movement {
	FORWARD,
	BACKWARD,
	RIGHT,
	LEFT
};

class Camera
{
public:
	// Position attributes
	glm::vec3 position;
	glm::vec3 front;
	glm::vec3 up;
	glm::vec3 right;

	// Euler angles
	float yaw;
	float pitch;

	// Options
	float speed;
	float zoom;
	float sensitivity;


	Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3 front = glm::vec3(0.0f, 0.0f, -1.0f));

	glm::mat4 getViewMatrix();

	void processMovement(Movement direction, float delta_time);
	void processPan(float delta_x, float delta_y);

private:
	void update_vectors();
};

