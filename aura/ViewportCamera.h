#pragma once
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>


class Camera
{
public:
	// Position attributes
	glm::vec3 target;
	glm::vec3 position;
	glm::vec3 front;
	glm::vec3 up;
	glm::vec3 right;


	// Spherical angles
	float distance;
	float phi;
	float theta;

	// Options
	float speed;
	float zoom;
	float sensitivity;


	Camera(glm::vec3 target = glm::vec3(0.0f, 0.0f, 0.0f), float distance = 3.0f, float phi = glm::radians(45.0f), float theta = glm::radians(45.0f));

	glm::mat4 getViewMatrix();

	void processMovement(float delta_x, float delta_y);
	void processPan(float delta_x, float delta_y);

private:
	void update_vectors();
};

