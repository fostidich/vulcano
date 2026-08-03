#include "Vulcano.hpp"

void Vulcano::cameraUpdate(float deltaT) {
    const float MOVE_SPEED = 5.0f;                // 5 meters / second
    const float ROT_SPEED  = glm::radians(90.0f); // 90 degrees / second
    const float MOUSE_RES  = 10.0f;

    static double old_xpos = 0, old_ypos = 0;
    static float yaw   = 0.0f;
    static float pitch = 0.0f;

    glm::vec3 mv  = glm::vec3(0.0f);
    glm::vec3 rot = glm::vec3(0.0f);

    // Speed multiplier (running player with CTRL)
    float speed_mul = 1;
    if (glfwGetKey(this->window, GLFW_KEY_LEFT_CONTROL))
        speed_mul = 5;

    // Compute mouse rotation (hide cursor and lock it inside window)
    double xpos, ypos;
    glfwSetInputMode(this->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwGetCursorPos(this->window, &xpos, &ypos);
    double m_dx = xpos - old_xpos;
    double m_dy = ypos - old_ypos;
    old_xpos = xpos, old_ypos = ypos;
    rot.y = m_dx / MOUSE_RES;
    rot.x = -m_dy / MOUSE_RES;

    // Compute player movement (WASD, SHIFT, SPACE)
    if (glfwGetKey(this->window, GLFW_KEY_D)) mv.x = 1.0f;
    if (glfwGetKey(this->window, GLFW_KEY_A)) mv.x = -1.0f;
    if (glfwGetKey(this->window, GLFW_KEY_S)) mv.z = 1.0f;
    if (glfwGetKey(this->window, GLFW_KEY_W)) mv.z = -1.0f;
    if (glfwGetKey(this->window, GLFW_KEY_SPACE)) mv.y = 1.0f;
    if (glfwGetKey(this->window, GLFW_KEY_LEFT_SHIFT)) mv.y = -1.0f;

    // Accumulate rotation (clamp pitch to prevent camera flipping)
    yaw += rot.y * ROT_SPEED * deltaT;
    pitch += rot.x * ROT_SPEED * deltaT;
    pitch = glm::clamp(pitch, glm::radians(-89.0f), glm::radians(89.0f));

    // Compute camera looking direction
    glm::vec3 forward;
    forward.x = cos(pitch) * sin(yaw);
    forward.y = sin(pitch);
    forward.z = -cos(pitch) * cos(yaw);
    forward   = glm::normalize(forward);

    // Flat forward vector (ignore pitch: strictly horizontal walking)
    glm::vec3 flatForward = glm::normalize(glm::vec3(forward.x, 0.0f, forward.z));
    glm::vec3 flatRight   = glm::normalize(glm::cross(flatForward, glm::vec3(0.0f, 1.0f, 0.0f)));

    // Apply translation relative to camera direction
    glm::vec3 right   = glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f)));
    glm::vec3 up      = glm::normalize(glm::cross(right, forward));
    glm::vec3 moveDir = (flatForward * -mv.z) + (flatRight * mv.x) + (glm::vec3(0.0f, 1.0f, 0.0f) * mv.y);

    // Update camera
    this->camera.eyePosition += moveDir * MOVE_SPEED * speed_mul * deltaT;
    this->camera.lookAtPoint = this->camera.eyePosition + forward;
}
