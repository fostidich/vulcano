#include "Vulcano.hpp"

void Vulcano::cameraUpdate(float deltaT) {
    const float MOVE_SPEED        = 20.0f;              // meters / second
    const float MOUSE_SENSITIVITY = glm::radians(0.1f); // rad / pixel

    // Compute player movement (WASD, SHIFT, SPACE)
    glm::vec3 mv = glm::vec3(0.0f);
    if (glfwGetKey(this->window, GLFW_KEY_D)) mv.x += 1.0f;
    if (glfwGetKey(this->window, GLFW_KEY_A)) mv.x -= 1.0f;
    if (glfwGetKey(this->window, GLFW_KEY_S)) mv.z += 1.0f;
    if (glfwGetKey(this->window, GLFW_KEY_W)) mv.z -= 1.0f;
    if (glfwGetKey(this->window, GLFW_KEY_SPACE)) mv.y += 1.0f;
    if (glfwGetKey(this->window, GLFW_KEY_LEFT_SHIFT)) mv.y -= 1.0f;

    // Speed multiplier (running player with CTRL)
    float speed_mul = 1;
    if (glfwGetKey(this->window, GLFW_KEY_LEFT_CONTROL))
        speed_mul = 5;

    // Compute mouse rotation difference from last frame
    double xpos, ypos;
    glfwGetCursorPos(this->window, &xpos, &ypos);
    static double old_xpos = xpos, old_ypos = ypos;
    double m_dx = xpos - old_xpos;
    double m_dy = ypos - old_ypos;
    old_xpos    = xpos;
    old_ypos    = ypos;

    // Compute current yaw and pitch
    glm::vec3 dir = glm::normalize(this->camera.lookAtPoint - this->camera.eyePosition);
    float yaw     = std::atan2(dir.x, -dir.z);
    float pitch   = std::asin(dir.y);

    // Accumulate rotation (clamp pitch to prevent camera flipping)
    yaw += m_dx * MOUSE_SENSITIVITY;
    pitch -= m_dy * MOUSE_SENSITIVITY;
    pitch = glm::clamp(pitch, glm::radians(-89.0f), glm::radians(89.0f));

    // Compute camera looking direction
    glm::vec3 forward;
    forward.x = cos(pitch) * sin(yaw);
    forward.y = sin(pitch);
    forward.z = -cos(pitch) * cos(yaw);

    // Flat forward vector (ignore pitch: strictly horizontal walking)
    glm::vec3 flatForward = glm::vec3(sin(yaw), 0.0f, -cos(yaw));
    glm::vec3 flatRight   = glm::vec3(cos(yaw), 0.0f, sin(yaw));
    glm::vec3 moveDir     = (flatForward * -mv.z) + (flatRight * mv.x) + (glm::vec3(0.0f, 1.0f, 0.0f) * mv.y);

    // Update camera
    this->camera.eyePosition += moveDir * MOVE_SPEED * speed_mul * deltaT;
    this->camera.lookAtPoint = this->camera.eyePosition + forward;
}
