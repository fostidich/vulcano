#include "Vulcano.hpp"

void Vulcano::pipelinesAndRenderPassesInit() {
    // Initialize render pass
    this->RP.init(this);
    this->RP.properties[0].clearValue = {0.0f, 0.0f, 0.2f, 1.0f}; // Set background

    // Initialize pipeline
    this->P.init(this,
                 &this->VD,                          // Vertex descriptor: specify pipeline's vertex descriptor layout (locations)
                 "shaders/SimplePos.vert.spv",       // Vertex shader: compiled shader file path
                 "shaders/BlinnFromPos.frag.spv",    // Fragment shader: compiled shader file path
                 {&this->DSLglobal, &this->DSLlocal} // Descriptor set layouts: ordered by set ID (set 0, set 1...)
    );
    this->P.setCullMode(VK_CULL_MODE_NONE); // Enable double side rendering
}

void Vulcano::populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage) {
    // Populate command buffer with draw commands for the main on-screen render
    // pass: `SC.populateCommandBuffer` registers the commands for rending each
    // object in the scene.
    this->RP.begin(commandBuffer, currentImage);
    this->SC.populateCommandBuffer(commandBuffer, 0, currentImage);
    this->RP.end(commandBuffer);
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
