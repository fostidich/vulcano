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
    err err = this->SC.init(this,
                            2,          // Number of render passes
                            this->VDRs, // Vector of vertex descriptor references available for model loading
                            this->PRs,  // Vector of technique/pipeline references available for material binding
                            scenePath   // Filepath to the scene JSON definition file
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

void Vulcano::setDSPoolSize() {
    // Sized generously to scale with scene's object count
    // [Inherited from BaseProject: this->DPSZs]
    this->DPSZs.uniformBlocksInPool = 256;
    this->DPSZs.texturesInPool      = 256;
    this->DPSZs.setsInPool          = 256;
}

void Vulcano::referencesInit() {
    // Initialize vertex descriptor references
    this->VDRs.resize(2);
    this->VDRs[0].init(
        "VDposUV",     // VD layout identifier matching in scene JSON
        &this->VDposUV // VD layout to assign to models matching ID
    );
    this->VDRs[1].init("VDpos", &this->VDpos);

    // Texture definition pointing to the off-screen shadow map
    TextureDefs shadowMapTex = {false, 0, this->RPshadow.attachments[0].getViewAndSampler()};
    TextureDefs modelTexture = {true, 0, {}};

    // Register pipeline (technique) references
    this->PRs.resize(3);
    this->PRs[0].init(
        "SimpleObject", // Technique (instances) identifier matching in scene JSON
        {
            {
                // Render pass 0
                &this->PshadowPosUV,             // Pipeline implementing the technique
                {{shadowMapTex}, {modelTexture}} // DSL mapping (set 0, set 1 with texture at index 0)
            },
            {
                // Render pass 1
                &this->PsimpleObject,            // Pipeline implementing the technique
                {{shadowMapTex}, {modelTexture}} // DSL mapping (set 0, set 1 with texture at index 0)
            }},
        1,             // Number of textures required per instance in this technique (1, at index 0)
        &this->VDposUV // Vertex descriptor required by this technique
    );
    this->PRs[1].init(
        "Terrain",
        {
            {&this->PshadowPos, {{shadowMapTex}, {}}}, // Render pass 0
            {&this->Pterrain, {{shadowMapTex}, {}}}    // Render pass 1
        },
        0,
        &this->VDpos);
    this->PRs[2].init(
        "SwordShine",
        {
            {&this->PshadowPosUV, {{shadowMapTex}, {modelTexture}}}, // Render pass 0
            {&this->PswordShine, {{shadowMapTex}, {modelTexture}}}   // Render pass 1
        },
        1,
        &this->VDposUV);
}

void Vulcano::localCleanup() {
    this->DSLlocalPosUV.cleanup();
    this->DSLlocalPos.cleanup();
    this->DSLglobal.cleanup();

    this->PshadowPos.destroy();
    this->PshadowPosUV.destroy();
    this->PsimpleObject.destroy();
    this->Pterrain.destroy();
    this->PswordShine.destroy();

    this->RPmain.destroy();
    this->RPshadow.destroy();

    this->txt.localCleanup();
    this->SC.localCleanup();
}

void Vulcano::pipelinesAndDescriptorSetsInit() {
    this->PshadowPos.create(&this->RPshadow);
    this->PshadowPosUV.create(&this->RPshadow);
    this->PsimpleObject.create(&this->RPmain);
    this->Pterrain.create(&this->RPmain);
    this->PswordShine.create(&this->RPmain);

    this->DSglobal.init(this, &this->DSLglobal,
                        {this->RPshadow.attachments[0].getViewAndSampler()});

    this->txt.pipelinesAndDescriptorSetsInit();
    this->SC.pipelinesAndDescriptorSetsInit();

    this->commandBuffersInit(); // FIXME: added to avoid crashing when resizing
}

void Vulcano::pipelinesAndDescriptorSetsCleanup() {
    this->clearCommandBuffers(); // FIXME: added to avoid crashing when resizing

    this->PshadowPos.cleanup();
    this->PshadowPosUV.cleanup();
    this->PsimpleObject.cleanup();
    this->Pterrain.cleanup();
    this->PswordShine.cleanup();

    this->RPmain.cleanup();
    this->RPshadow.cleanup();

    this->DSglobal.cleanup();

    this->txt.pipelinesAndDescriptorSetsCleanup();
    this->SC.pipelinesAndDescriptorSetsCleanup();
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
    this->Ar            = (float)w / (float)h;
    this->RPmain.width  = w;
    this->RPmain.height = h;
    this->txt.resizeScreen(w, h);
    logs::info("Window resized to ", w, "x", h);
}
