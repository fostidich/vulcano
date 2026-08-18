#include "Vulcano.hpp"

void Vulcano::fireInteractions() {
    // Executed once when pressing E.
    // These are the events/callbacks fired when interacting with the world.
    if (this->world.sceneID == SCENE_DEFAULT)
        this->drawDeersBridge();
}

void Vulcano::processInteractions(float deltaT) {
    // Executed on every frame.
    // These are the callbacks executed after an event has been fired to compute
    // world state changes and animations.
    if (this->world.sceneID == SCENE_DEFAULT)
        this->updateDeersBridge(deltaT);
}

void Vulcano::processTextOnScreen(float deltaT) {
    // Executed on every frame.
    // These are the callbacks processed continuously for printing the correct
    // text on the screen.
    this->updateDebugScreen(deltaT);
    if (this->world.sceneID == SCENE_DEFAULT)
        this->printDeerText();
}
