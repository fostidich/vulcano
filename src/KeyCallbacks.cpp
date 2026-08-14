#include "Vulcano.hpp"

void Vulcano::switchKeys(int key) {
    switch (key) {
    case GLFW_KEY_Q: // Close application
        glfwSetWindowShouldClose(window, GLFW_TRUE);
        break;
    case GLFW_KEY_ESCAPE: // Defocus window
        state.cursorCaptured = false;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        break;
    case GLFW_KEY_H: // Toggle colliders
        state.showColliders = !state.showColliders;
        break;
    case GLFW_KEY_G: // Toggle collisions
        state.collisions = !state.collisions;
        if (!state.collisions) state.flightMode = true;
        break;
    case GLFW_KEY_F: // Toggle creative
        state.flightMode = !state.flightMode;
        break;
    default:
        logs::debug("Unknown key: ", key);
    }
}

void Vulcano::switchMouseButtons(int btn) {
    switch (btn) {
    case GLFW_MOUSE_BUTTON_LEFT: // Refocus window
        if (state.cursorCaptured) break;
        state.cursorCaptured = true;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        break;
    default:
        logs::debug("Unknown mouse button: ", btn);
    }
}

void Vulcano::keypressCallbacksInit() {
    glfwSetInputMode(this->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // Hide cursor and lock it inside window
    glfwSetWindowUserPointer(this->window, this);                      // Store instance pointer on window

    // Register keys callback
    glfwSetKeyCallback(this->window, [](GLFWwindow *window, int key, int scancode, int action, int mods) {
        auto *app = static_cast<Vulcano *>(glfwGetWindowUserPointer(window));
        if (action != GLFW_PRESS) return;
        app->switchKeys(key);
    });

    // Register mouse button callback
    glfwSetMouseButtonCallback(this->window, [](GLFWwindow *window, int button, int action, int mods) {
        auto *app = static_cast<Vulcano *>(glfwGetWindowUserPointer(window));
        if (action != GLFW_PRESS) return;
        app->switchMouseButtons(button);
    });
}
