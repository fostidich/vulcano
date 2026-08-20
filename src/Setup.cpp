#include "Types.hpp"
#include "Vulcano.hpp"

void Vulcano::localInit() {
    this->descriptorsInit();
    this->pipelinesAndRenderPassesInit();
    this->setDSPoolSize();
    this->referencesInit();
    this->loadScene();
    this->cacheSceneColliders();
    this->extractPointLights();
    this->commandBuffersInit();
    this->textMakerInit();
    this->keypressCallbacksInit();
    this->playerInit();
}

void Vulcano::playerInit() {
    const glm::vec3 min = player.colliderAABBmin;
    const glm::vec3 max = player.colliderAABBmax;
    this->playerCollider.initAABB(min.x, min.y, min.z,
                                  max.x, max.y, max.z);
}

void Vulcano::cacheSceneColliders() {
    // Getting extents is a costly task.
    // By caching colliders it is called once at startup, and during animations
    // only, therefore is never called during simple frame loop.
    this->cachedColliders.clear();
    for (Collider *c : this->SC.GlobalColliders)
        this->cachedColliders.push_back({c, c->getExtents()});
}

void Vulcano::loadScene() {
    const string scenePath = this->world.scenePath;
    logs::info("Loading scene ", scenePath);
    const int err = this->SC.init(this,
                                  1,          // Number of render passes (1 for single main pass rendering)
                                  this->VDRs, // Vector of Vertex Descriptor References available for model loading
                                  this->PRs,  // Vector of Technique/Pipeline References available for material binding
                                  scenePath   // Filepath to the scene.json definition file
    );
    if (err) {
        logs::error("Error loading scene ", scenePath);
        exit(err);
    }
}

void Vulcano::textMakerInit() {
    const u32 w = this->swapChainExtent.width;
    const u32 h = this->swapChainExtent.height;
    this->txt.init(this, w, h);
}

void Vulcano::localCleanup() {
    this->DSLlocalPosUV.cleanup();
    this->DSLlocalPos.cleanup();
    this->DSLglobal.cleanup();
    this->PsimpleObject.destroy();
    this->Pterrain.destroy();
    this->RP.destroy();
    this->SC.localCleanup();
    this->txt.localCleanup();
}

void Vulcano::pipelinesAndDescriptorSetsInit() {
    this->RP.create();
    this->PsimpleObject.create(&this->RP);
    this->Pterrain.create(&this->RP);
    this->DSglobal.init(this, &this->DSLglobal, {});
    this->SC.pipelinesAndDescriptorSetsInit();
    this->txt.pipelinesAndDescriptorSetsInit();
    this->commandBuffersInit(); // FIXME: added to avoid crashing when resizing
}

void Vulcano::pipelinesAndDescriptorSetsCleanup() {
    this->clearCommandBuffers(); // FIXME: added to avoid crashing when resizing
    this->PsimpleObject.cleanup();
    this->Pterrain.cleanup();
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
    if (w < 0 || h < 0) return;
    this->Ar        = (float)w / (float)h;
    this->RP.width  = w;
    this->RP.height = h;
    this->txt.resizeScreen(w, h);
    logs::info("Window resized to ", w, "x", h);
}
