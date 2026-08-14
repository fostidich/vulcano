#include "Vulcano.hpp"

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
    glm::vec3 mv = glm::vec3(0.0f);
    if (glfwGetKey(this->window, GLFW_KEY_D)) mv.x += 1.0f;
    if (glfwGetKey(this->window, GLFW_KEY_A)) mv.x -= 1.0f;
    if (glfwGetKey(this->window, GLFW_KEY_S)) mv.z += 1.0f;
    if (glfwGetKey(this->window, GLFW_KEY_W)) mv.z -= 1.0f;
    if (glfwGetKey(this->window, GLFW_KEY_SPACE)) mv.y += 1.0f;
    if (glfwGetKey(this->window, GLFW_KEY_LEFT_SHIFT)) mv.y -= 1.0f;

    // Speed multiplier (running player with CTRL)
    const bool speeding = glfwGetKey(this->window, GLFW_KEY_LEFT_CONTROL);

    // Compute current yaw and pitch
    const glm::vec3 dir = glm::normalize(state.lookAtPoint - state.eyePosition);
    float yaw           = std::atan2(dir.x, -dir.z);
    float pitch         = std::asin(dir.y);

    // Accumulate rotation (clamp pitch to prevent camera flipping)
    yaw += m_dx * state.mouseSensitivity;
    pitch -= m_dy * state.mouseSensitivity;
    pitch = glm::clamp(pitch, glm::radians(-89.0f), glm::radians(89.0f));

    // Compute camera looking direction
    glm::vec3 forward;
    forward.x = cos(pitch) * sin(yaw);
    forward.y = sin(pitch);
    forward.z = -cos(pitch) * cos(yaw);

    // Flat forward vector (ignore pitch: strictly horizontal walking)
    const glm::vec3 flatForward = glm::vec3(sin(yaw), 0.0f, -cos(yaw));
    const glm::vec3 flatRight   = glm::vec3(cos(yaw), 0.0f, sin(yaw));
    const glm::vec3 straightUp  = glm::vec3(0.0f, 1.0f, 0.0f);
    const glm::vec3 moveDir     = (flatForward * -mv.z) + (flatRight * mv.x) + (straightUp * mv.y);

    // Update camera
    this->state.eyePosition += moveDir * state.moveSpeed * (speeding ? state.runMultiplier : 1) * deltaT;
    this->state.lookAtPoint = state.eyePosition + forward;
}
