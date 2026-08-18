#include "Vulcano.hpp"

namespace {
void toggle(bool &b) { b = !b; }
} // namespace

void Vulcano::switchKeys(int key, int action) {
    if (action == GLFW_PRESS)
        switch (key) {
        case GLFW_KEY_ESCAPE: // Defocus window
            player.cursorCaptured = false;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            break;
        case GLFW_KEY_LEFT_CONTROL: // Sprint
            player.running = true;
            break;
        case GLFW_KEY_LEFT_SHIFT: // Sneak
            if (!player.flightMode) player.slowing = true;
            break;
        case GLFW_KEY_SPACE: // Jump
            if (player.jumping || player.flightMode) break;
            player.jumping   = true;
            player.jumpTimer = player.jumpDuration;
            break;
        case GLFW_KEY_Q: // Close application
            glfwSetWindowShouldClose(window, GLFW_TRUE);
            break;
        case GLFW_KEY_I: // Toggle debug screen
            toggle(player.debugScreen);
            break;
        case GLFW_KEY_H: // Toggle colliders
            toggle(player.showColliders);
            toggleColliders();
            break;
        case GLFW_KEY_C: // Toggle collisions
            toggle(player.collisions);
            if (!player.collisions) player.flightMode = true;
            break;
        case GLFW_KEY_F: // Toggle creative
            toggle(player.flightMode);
            if (!player.flightMode) player.collisions = true;
            if (player.flightMode) player.slowing = false;
            break;
        case GLFW_KEY_Z: // Activate zoom
            player.zooming = true;
            break;
        case GLFW_KEY_E: // Interact with world
            this->fireInteractions();
            break;
        default:
            logs::debug("Unknown key: ", key);
        }

    else if (action == GLFW_RELEASE)
        switch (key) {
        case GLFW_KEY_LEFT_CONTROL: // Back to normal speed
            player.running = false;
            break;
        case GLFW_KEY_LEFT_SHIFT: // Back to normal speed
            player.slowing = false;
            break;
        case GLFW_KEY_Z: // Deactivate zoom
            player.zooming = false;
            break;
        default:
            logs::debug("Unknown key: ", key);
        }
}

void Vulcano::switchMouseButtons(int btn, int action) {
    if (action == GLFW_PRESS)
        switch (btn) {
        case GLFW_MOUSE_BUTTON_LEFT: // Refocus window
            if (player.cursorCaptured) break;
            player.cursorCaptured = true;
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
