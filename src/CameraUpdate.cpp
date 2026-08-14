#include "Vulcano.hpp"
using namespace glm;

void Vulcano::cameraUpdate(float deltaT) {
    // When first focusing window, delay camera change to avoid flickers
    static u8 stillFrames = 5;
    if (!state.cursorCaptured) {
        stillFrames = 5;
        return;
    }

    // Compute mouse rotation difference from last frame
    double xpos, ypos;
    glfwGetCursorPos(this->window, &xpos, &ypos);
    static double old_xpos = xpos, old_ypos = ypos;
    const double m_dx = xpos - old_xpos;
    const double m_dy = ypos - old_ypos;
    old_xpos          = xpos;
    old_ypos          = ypos;
    if (stillFrames != 0) {
        stillFrames--;
        return;
    }

    // Compute player movement (WASD, SHIFT, SPACE)
    vec3 mv = vec3(0.0f);
    if (glfwGetKey(this->window, GLFW_KEY_D)) mv.x += 1.0f;
    if (glfwGetKey(this->window, GLFW_KEY_A)) mv.x -= 1.0f;
    if (glfwGetKey(this->window, GLFW_KEY_S)) mv.z += 1.0f;
    if (glfwGetKey(this->window, GLFW_KEY_W)) mv.z -= 1.0f;
    if (state.flightMode) {
        if (glfwGetKey(this->window, GLFW_KEY_SPACE)) mv.y += 2.0f;
        if (glfwGetKey(this->window, GLFW_KEY_LEFT_SHIFT)) mv.y -= 2.0f;
    } else {
        mv.y -= 1.0f;
    }

    // Speed multiplier (running player with CTRL)
    const bool speeding = glfwGetKey(this->window, GLFW_KEY_LEFT_CONTROL);

    // Compute current yaw and pitch
    const vec3 dir = normalize(state.lookAtPoint - state.eyePosition);
    float yaw      = std::atan2(dir.x, -dir.z);
    float pitch    = std::asin(dir.y);

    // Accumulate rotation (clamp pitch to prevent camera flipping)
    yaw += m_dx * state.mouseSensitivity;
    pitch -= m_dy * state.mouseSensitivity;
    pitch = clamp(pitch, radians(-89.0f), radians(89.0f));

    // Compute camera looking direction
    vec3 forward;
    forward.x = cos(pitch) * sin(yaw);
    forward.y = sin(pitch);
    forward.z = -cos(pitch) * cos(yaw);

    // Flat forward vector (ignore pitch: strictly horizontal walking)
    const vec3 flatForward = vec3(sin(yaw), 0.0f, -cos(yaw));
    const vec3 flatRight   = vec3(cos(yaw), 0.0f, sin(yaw));
    const vec3 straightUp  = vec3(0.0f, 1.0f, 0.0f);
    const vec3 moveDirZ    = flatForward * -mv.z * (speeding ? state.runMultiplier : 1);
    const vec3 moveDirX    = flatRight * mv.x * (speeding ? state.runMultiplier : 1);
    const vec3 moveDirY    = straightUp * mv.y;
    const vec3 moveDir     = moveDirZ + moveDirX + moveDirY;

    // Compute translation delta
    const vec3 displacement = moveDir * state.moveSpeed * deltaT;
    vec3 newPos             = this->state.eyePosition;
    slideAgainsWall(newPos, displacement);

    // Update camera
    this->state.eyePosition = newPos;
    this->state.lookAtPoint = state.eyePosition + forward;
}

void Vulcano::slideAgainsWall(vec3 &newPos, const vec3 &displacement) {
    if (!checkCollision(newPos + vec3(displacement.x, 0.0f, 0.0f))) { // Try moving in X
        newPos.x += displacement.x;
    }
    if (!checkCollision(newPos + vec3(0.0f, displacement.y, 0.0f))) { // Try moving in Y
        newPos.y += displacement.y;
    }
    if (!checkCollision(newPos + vec3(0.0f, 0.0f, displacement.z))) { // Try moving in Z
        newPos.z += displacement.z;
    }
}

bool Vulcano::checkCollision(const vec3 &testPos) {
    // Update player collider's world position
    this->playerCollider.setWorldMatrix(translate(mat4(1.0f), testPos));

    // Test against all colliders created by the scene
    for (Collider *objCollider : this->SC.GlobalColliders) {
        if (objCollider && this->playerCollider.collidesWith(*objCollider)) {
            return true; // Collision detected
        }
    }
    return false;
};
