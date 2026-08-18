#include "Vulcano.hpp"

using namespace glm;

void Vulcano::cameraUpdate(float deltaT) {
    // When first focusing window, delay camera change to avoid flickers
    static u8 stillFrames = 5;
    if (!state.cursorCaptured) {
        stillFrames = 5;
        return;
    }

    // Update jump timer
    if (state.jumping) {
        state.jumpTimer -= deltaT;
        if (state.jumpTimer <= -state.jumpDuration) {
            state.jumping   = false;
            state.jumpTimer = 0.0f;
        }
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
        if (glfwGetKey(this->window, GLFW_KEY_SPACE)) mv.y += 0.5f;
        if (glfwGetKey(this->window, GLFW_KEY_LEFT_SHIFT)) mv.y -= 0.5f;
    } else {
        mv.y = state.gravity * (state.jumping ? -state.jumpTimer : 1.0f);
    }
    if (state.flightMode) mv *= 5.0f;

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

    // Compute translation delta
    const float speedMul    = (state.running ? state.runMultiplier : 1.0f) *
                              (state.slowing ? state.sneakMultiplier : 1.0f);
    const vec3 moveDirZ     = flatForward * -mv.z * speedMul;
    const vec3 moveDirX     = flatRight * mv.x * speedMul;
    const vec3 moveDirY     = straightUp * mv.y;
    const vec3 moveDir      = moveDirZ + moveDirX + moveDirY;
    const vec3 displacement = moveDir * state.moveSpeed * deltaT;

    // Compute collisions
    vec3 newPos = this->state.eyePosition;
    if (state.collisions)
        processCollisions(newPos, displacement);
    else
        newPos += displacement;

    // Update camera
    this->state.eyePosition = newPos;
    this->state.lookAtPoint = state.eyePosition + forward;
    this->state.pitch       = pitch;
    this->state.yaw         = yaw;
}
