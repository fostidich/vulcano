#include "Vulcano.hpp"

void Vulcano::localInit() {
    this->descriptorsInit();
    this->pipelinesAndRenderPassesInit();
    this->setDSPoolSize();
    this->referencesInit();
    this->loadScene();
    this->commandBuffersInit();
    this->textMakerInit();
    this->keypressCallbacksInit();
    this->playerInit();
}

void Vulcano::playerInit() {
    this->playerCollider.initAABB(
        -0.5f, -1.5f, -0.5f, // min (x1, y1, z1)
        0.5f, 0.5f, 0.5f     // max (x2, y2, z2)
    );
}

void Vulcano::loadScene() {
    const std::string scenePath = "assets/scenes/Scene.json";
    logs::info("Loading scene ", scenePath);
    const int err = this->SC.init(this,
                                  1,          // Number of render passes (1 for single main pass rendering)
                                  this->VDRs, // Vector of Vertex Descriptor References available for model loading
                                  this->PRs,  // Vector of Technique/Pipeline References available for material binding
                                  scenePath   // Filepath to the scene.json definition file
    );
    if (err) {
        logs::error("Error loading scene ", scenePath);
        exit(1);
    }
}

void Vulcano::pipelinesAndDescriptorSetsInit() {
    this->RP.create();
    this->P.create(&this->RP);
    this->DSglobal.init(this, &this->DSLglobal, {});
    this->SC.pipelinesAndDescriptorSetsInit();
    this->txt.pipelinesAndDescriptorSetsInit();
}

void Vulcano::localCleanup() {
    this->DSLlocal.cleanup();
    this->DSLglobal.cleanup();
    this->P.destroy();
    this->RP.destroy();
    this->SC.localCleanup();
    this->txt.localCleanup();
}

void Vulcano::pipelinesAndDescriptorSetsCleanup() {
    this->P.cleanup();
    this->RP.cleanup();
    this->DSglobal.cleanup();
    this->SC.pipelinesAndDescriptorSetsCleanup();
    this->txt.pipelinesAndDescriptorSetsCleanup();
}

void Vulcano::setWindowParameters() {
    this->windowWidth     = 1200;
    this->windowHeight    = 800;
    this->windowTitle     = "Vulcano";
    this->windowResizable = GLFW_TRUE;
    this->Ar              = (float)this->windowWidth / (float)this->windowHeight;
    logs::info("Started window with size ", this->windowWidth, "x", this->windowHeight, " (AR ", this->Ar, ")");
}

void Vulcano::onWindowResize(int w, int h) {
    this->Ar        = (float)w / (float)h;
    this->RP.width  = w;
    this->RP.height = h;
    this->txt.resizeScreen(w, h);
    logs::info("Window resized to ", w, "x", h);
}
