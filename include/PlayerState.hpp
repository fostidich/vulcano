#pragma once
#include <glm/glm.hpp>

// Main struct containing player specific settings and state.
struct PlayerState {
    // Camera position and rotation state
    // glm::vec3 eyePosition = glm::vec3(-600.0f, 130.0f, 600.0f);
    glm::vec3 eyePosition = glm::vec3(0.0f, 4.0f, 0.0f);
    glm::vec3 lookAtPoint = glm::vec3(0.0f, 4.0f, 1.0f);
    glm::vec3 upVector    = glm::vec3(0.0f, 1.0f, 0.0f);

    // Looking directions
    float pitch = 0.0f;
    float yaw   = 0.0f;

    // Frustum shape
    const float FOVy      = glm::radians(60.0f);
    const float nearPlane = 0.1f;
    const float farPlane  = 2000.f;

    // Player hitbox
    const glm::vec3 colliderAABBmin = glm::vec3(-0.5f, -1.5f, -0.5f);
    const glm::vec3 colliderAABBmax = glm::vec3(0.5f, 0.5f, 0.5f);

    // Player camera and movement speed settings
    const float mouseSensitivity = glm::radians(0.1f); // rad / pixel
    const float moveSpeed        = 10.0f;              // meters / second
    const float gravity          = -9.8f / 2.0f;       // meters / second
    const float runMultiplier    = 2.0f;
    const float sneakMultiplier  = 0.5f;

    bool cursorCaptured = true;  // Whether the window has focus
    bool flightMode     = false; // Whether the player can fly
    bool collisions     = true;  // Collisions with objects
    bool showColliders  = false; // Object hitboxes
    bool debugScreen    = false; // Show debug screen
    bool running        = false; // Whether player is sprinting
    bool slowing        = false; // Whether player is sneaking
    bool zooming        = false; // Camera is zooming

    // Jump: gravity reversed for a while
    bool jumping             = false; // Whether player pressed space
    const float jumpDuration = 0.4f;  // Time between jump start and max height
    float jumpTimer          = 0.0f;  // Time until player stays airborne for jumping
};
