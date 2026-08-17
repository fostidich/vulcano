#include "Vulcano.hpp"

void toggle(bool &b) { b = !b; }

void Vulcano::switchKeys(int key, int action) {
    if (action == GLFW_PRESS)
        switch (key) {
        case GLFW_KEY_ESCAPE: // Defocus window
            state.cursorCaptured = false;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            break;
        case GLFW_KEY_LEFT_CONTROL: // Sprint
            state.running = true;
            break;
        case GLFW_KEY_LEFT_SHIFT: // Sneak
            if (!state.flightMode) state.slowing = true;
            break;
        case GLFW_KEY_SPACE: // Jump
            if (state.jumping || state.flightMode) break;
            state.jumping   = true;
            state.jumpTimer = state.jumpDuration;
            break;
        case GLFW_KEY_Q: // Close application
            glfwSetWindowShouldClose(window, GLFW_TRUE);
            break;
        case GLFW_KEY_I: // Toggle debug screen
            toggle(state.debugScreen);
            break;
        case GLFW_KEY_H: // Toggle colliders
            toggle(state.showColliders);
            break;
        case GLFW_KEY_C: // Toggle collisions
            toggle(state.collisions);
            if (!state.collisions) state.flightMode = true;
            break;
        case GLFW_KEY_F: // Toggle creative
            toggle(state.flightMode);
            if (!state.flightMode) state.collisions = true;
            if (state.flightMode) state.slowing = false;
            break;
        case GLFW_KEY_Z: // Activate zoom
            state.zooming = true;
            break;
        case GLFW_KEY_E: // Interact with world
            this->processInteractions();
            break;
        default:
            logs::debug("Unknown key: ", key);
        }

    else if (action == GLFW_RELEASE)
        switch (key) {
        case GLFW_KEY_LEFT_CONTROL: // Back to normal speed
            state.running = false;
            break;
        case GLFW_KEY_LEFT_SHIFT: // Back to normal speed
            state.slowing = false;
            break;
        case GLFW_KEY_Z: // Deactivate zoom
            state.zooming = false;
            break;
        default:
            logs::debug("Unknown key: ", key);
        }
}

void Vulcano::switchMouseButtons(int btn, int action) {
    if (action == GLFW_PRESS)
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
    glfwSetKeyCallback(this->window, [](GLFWwindow *window, int key, int, int action, int) {
        auto *app = static_cast<Vulcano *>(glfwGetWindowUserPointer(window));
        app->switchKeys(key, action);
    });

    // Register mouse button callback
    glfwSetMouseButtonCallback(this->window, [](GLFWwindow *window, int button, int action, int) {
        auto *app = static_cast<Vulcano *>(glfwGetWindowUserPointer(window));
        app->switchMouseButtons(button, action);
    });
}
