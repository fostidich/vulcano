#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

// Main struct containing player specific settings and state.
struct PlayerState {
    // Camera position and rotation state
    glm::vec3 eyePosition = glm::vec3(0.0f, 20.0f, 20.0f);
    glm::vec3 lookAtPoint = glm::vec3(0.0f, 5.0f, 0.0f);
    glm::vec3 upVector    = glm::vec3(0.0f, 1.0f, 0.0f);

    // Looking directions
    float pitch = 0.0f;
    float yaw   = 0.0f;

    // Frustum shape
    const float FOVy      = glm::radians(60.0f);
    const float nearPlane = 0.1f;
    const float farPlane  = 1000.f;

    // Player camera and movement speed
    const float mouseSensitivity = glm::radians(0.1f); // rad / pixel
    const float moveSpeed        = 10.0f;              // meters / second
    const float gravity          = -9.8f / 2.0f;       // meters / second
    const float runMultiplier    = 2.0f;

    bool cursorCaptured = true;  // Whether the window has focus
    bool flightMode     = false; // Whether the player can fly
    bool collisions     = true;  // Collisions with objects
    bool showColliders  = false; // Object hitboxes
    bool debugScreen    = true;  // Show debug screen
    bool running        = false; // Whether player is running


    bool leverTriggered    = false; //hidden mechanism - lower bridge
};
