#include "Vulcano.hpp"

void Vulcano::pipelinesAndRenderPassesInit() {
    // Initialize off-screen shadow render pass
    this->RPshadow.init(this,
                        pow(2, 11), pow(2, 11),                                           // 2048x2048 shadow map
                        -1,                                                               // Framebuffers match swap chain image count
                        RenderPass::getStandardAttchmentsProperties(AT_DEPTH_ONLY, this), // No color: just 2048x2048 floats
                        RenderPass::getStandardDependencies(ATDEP_DEPTH_TRANS),           // Enforce RP finish before starting the following
                        true                                                              // Generates sampler for sampling depth in pass 1
    );
    this->RPshadow.create();

    // Initialize render pass
    this->RPmain.init(this);
    this->RPmain.properties[0].clearValue = {0.2f, 0.2f, 0.35f, 1.0f}; // Set background
    this->RPmain.create();

    // Initialize pipelines
    this->PshadowPosUV.init(this, &this->VDposUV,
                            "shaders/Shadow.vert.spv", "shaders/Empty.frag.spv",
                            {&this->DSLglobal, &this->DSLlocalPosUV});
    this->PshadowPosUV.setCullMode(VK_CULL_MODE_NONE); // Enable double side rendering
    this->PshadowPos.init(this, &this->VDpos,
                          "shaders/Shadow.vert.spv", "shaders/Empty.frag.spv",
                          {&this->DSLglobal, &this->DSLlocalPos});
    this->PshadowPos.setCullMode(VK_CULL_MODE_NONE); // Enable double side rendering
    this->PsimpleObject.init(this,
                             &this->VDposUV,                          // Pipeline's vertex descriptor layout (locations)
                             "shaders/SimplePosUV.vert.spv",          // Vertex shader: compiled shader file path
                             "shaders/SimpleObject.frag.spv",         // Fragment shader: compiled shader file path
                             {&this->DSLglobal, &this->DSLlocalPosUV} // Descriptor set layouts: ordered by set ID (set 0, set 1...)
    );
    this->PsimpleObject.setCullMode(VK_CULL_MODE_NONE); // Enable double side rendering
    this->Pterrain.init(this, &this->VDpos,
                        "shaders/SimplePos.vert.spv", "shaders/Terrain.frag.spv",
                        {&this->DSLglobal, &this->DSLlocalPos});
    this->Pterrain.setCullMode(VK_CULL_MODE_NONE); // Enable double side rendering
    this->PswordShine.init(this, &this->VDposUV,
                           "shaders/SimplePosUV.vert.spv", "shaders/SwordShine.frag.spv",
                           {&this->DSLglobal, &this->DSLlocalPosUV});
    this->PswordShine.setCullMode(VK_CULL_MODE_NONE); // Enable double side rendering
}

void Vulcano::populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage) {
    // Populate command buffer with draw commands for the main on-screen render
    // pass: `SC.populateCommandBuffer` registers the commands for rending each
    // object in the scene.

    // Pass 0
    this->RPshadow.begin(commandBuffer, currentImage);
    this->SC.populateCommandBuffer(commandBuffer, 0, currentImage);
    this->RPshadow.end(commandBuffer);

    // Pass 1
    this->RPmain.begin(commandBuffer, currentImage);
    this->SC.populateCommandBuffer(commandBuffer, 1, currentImage);
    this->RPmain.end(commandBuffer);
}

void Vulcano::commandBuffersInit() {
    auto cb = [](VkCommandBuffer commandBuffer, int currentImage, void *Params) {
        Vulcano *T = (Vulcano *)Params;
        T->populateCommandBuffer(commandBuffer, currentImage);
    };
    this->submitCommandBuffer("main", // Command buffer identifier
                              0,      // Execution step: lower IDs execute first (0 executes first)
                              cb,     // Callback populating GPU command buffer
                              this);
}
