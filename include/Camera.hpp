#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

// Camera object used to store current player looking direction and world
// position.
// The following default member initializers define the angle and translation
// of the player at startup.
struct Camera {
    // Camera position (eye) in world space.
    glm::vec3 eyePosition = glm::vec3(0.0f, 20.0f, 20.0f);
    // Target point center of screen in world space.
    glm::vec3 lookAtPoint = glm::vec3(0.0f, 5.0f, 0.0f);
    // Camera's vertical orientation (up vector).
    glm::vec3 upVector = glm::vec3(0.0f, 1.0f, 0.0f);
};
