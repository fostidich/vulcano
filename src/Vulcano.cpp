#include "Vulcano.hpp"
#include "Camera.hpp"
#include "Descriptors.hpp"
#include "Logs.hpp"
#include "modules/Starter.hpp"

void Vulcano::frameLogic(float deltaT) {
    this->cameraUpdate(deltaT);
    this->keypressCallbacks(deltaT);
}

void Vulcano::keypressCallbacks(float deltaT) {
    // Handle the ESC and Q key to exit the app
    if (glfwGetKey(this->window, GLFW_KEY_ESCAPE))
        glfwSetWindowShouldClose(this->window, GL_TRUE);
    if (glfwGetKey(this->window, GLFW_KEY_Q))
        glfwSetWindowShouldClose(this->window, GL_TRUE);
}

void Vulcano::computeViewProj() {
    const float FOVy      = glm::radians(45.0f);
    const float nearPlane = 0.1f;
    const float farPlane  = 100.f;

    // Projection matrix
    this->Prj = glm::perspective(FOVy, this->Ar, nearPlane, farPlane);
    this->Prj[1][1] *= -1;

    // View matrix
    this->View = glm::lookAt(this->camera.eyePosition,
                             this->camera.lookAtPoint,
                             this->camera.upVector);

    // View projection matrix
    this->ViewPrj = this->Prj * this->View;
}

float Vulcano::getDeltaT() {
    static auto startTime = std::chrono::high_resolution_clock::now();
    static float lastTime = 0.0f;

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time       = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
    float deltaT     = time - lastTime;
    lastTime         = time;
    return deltaT;
}

void Vulcano::updateUniformBuffer(u32 currentImage) {
    float deltaT = this->getDeltaT();
    this->frameLogic(deltaT);
    this->computeViewProj();
    GlobalUniformBufferObject gubo;
    this->updateGlobalLight(gubo, deltaT, currentImage);
    this->updateSceneInstances(gubo, currentImage);
    this->updateFPS(deltaT);
}

void Vulcano::updateGlobalLight(GlobalUniformBufferObject &gubo, float deltaT, int currentImage) {
    static float lightRotationAngle = 0.0f;

    // Define parameters for the uniform.
    // Keep track of rotation and increment rotation angle based on time.
    lightRotationAngle += -0.5f * deltaT;
    const glm::mat4 lightView =
        glm::rotate(glm::mat4(1),
                    glm::radians(lightRotationAngle),
                    glm::vec3(0.0f, 1.0f, 0.0f)) *
        glm::rotate(glm::mat4(1),
                    glm::radians(-45.0f),
                    glm::vec3(1.0f, 0.0f, 0.0f));

    // Construct GUBO and send it to GPU
    gubo.lightDir   = glm::vec3(lightView * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f));
    gubo.lightColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f) * 5.0f;
    gubo.eyePos     = glm::vec3(glm::inverse(this->View)[3]);
    this->DSglobal.map(currentImage, &gubo, 0);
}

void Vulcano::updateSceneInstances(GlobalUniformBufferObject gubo, int currentImage) {
    UniformBufferObject ubo;
    for (int instanceId = 0; instanceId < this->SC.TI[0].InstanceCount; instanceId++) {
        ubo.mMat   = this->SC.TI[0].I[instanceId].Wm;
        ubo.mvpMat = this->ViewPrj * ubo.mMat;
        this->SC.TI[0].I[instanceId].DS[0][0]->map(currentImage, &gubo, 0); // Global lighting
        this->SC.TI[0].I[instanceId].DS[0][1]->map(currentImage, &ubo, 0);  // Camera MVP matrix
    }
}

void Vulcano::updateFPS(float deltaT) {
    static float elapsedT    = 0.0f;
    static int countedFrames = 0;

    countedFrames++;
    elapsedT += deltaT;

    // If another full second has passed
    if (elapsedT > 1.0f) {
        float fps = (float)countedFrames / (float)elapsedT;

        std::ostringstream oss;
        oss << "FPS: " << fps << "\n";

        // Updates the FPS in top-left corner
        this->txt.print(-1.0f, -1.0f,
                        oss.str(), 1, "CO",
                        false, false, true,
                        TAL_LEFT, TRH_LEFT, TRV_TOP,
                        {1.0f, 0.0f, 0.0f, 1.0f},
                        {0.8f, 0.8f, 0.0f, 1.0f});

        elapsedT      = 0.0f;
        countedFrames = 0;
    }

    this->txt.updateCommandBuffer();
}

void Vulcano::localInit() {
    this->descriptorsInit();
    this->pipelinesAndRenderPassesInit();
    this->setDSPoolSize();
    this->referencesInit();
    this->loadScene();
    this->commandBuffersInit();
    this->textMakerInit();
}

void Vulcano::descriptorsInit() {
    this->DSLlocalInit();
    this->DSLglobalInit();
    this->VDInit();
}

void Vulcano::DSLlocalInit() {
    // Init a set with two bindings: objects' UBO and texture
    this->DSLlocal.init(this,
                        {{
                             // Binding 0: objects' UBO
                             0,                                 // Binding number inside the set
                             VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, // Descriptor type: uniform buffer (what UBO is)
                             VK_SHADER_STAGE_VERTEX_BIT,        // Accessed by vertex shader only
                             sizeof(UniformBufferObject),       // Size of the descriptor's object type
                             1                                  // Count 1: not an array
                         },
                         {
                             // Binding 1: textures (image and sampler)
                             1,                                         // Binding number inside the set
                             VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, // Descriptor type: texture and sampler
                             VK_SHADER_STAGE_FRAGMENT_BIT,              // Accessed by fragment shader only
                             0,                                         // Size 0 is a placeholder: textures are loaded later
                             1                                          // Count 1: not an array
                         }});
}

void Vulcano::DSLglobalInit() {
    // Init a set with one binding: scene's GUBO
    this->DSLglobal.init(this,
                         {{
                             // Binding 0: scene's GUBO (light and camera)
                             0,                                 // Binding number inside the set
                             VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, // Descriptor type: uniform buffer (what GUBO is)
                             VK_SHADER_STAGE_ALL_GRAPHICS,      // Accesses by both vertex and fragment shaders
                             sizeof(GlobalUniformBufferObject), // Size of the descriptor's object type
                             1                                  // Count 1: not an array
                         }});
}

void Vulcano::VDInit() {
    // Init a descriptor with two locations: position and texture coordinates
    this->VD.init(this,
                  {{
                      // Binding 0 (struct 0): input vertices
                      0,                          // Binding number (vertex struct ID)
                      sizeof(Vertex),             // Stride: total size of the struct sent
                      VK_VERTEX_INPUT_RATE_VERTEX // Input rate: step is by vertex (instead of by instance)
                  }},
                  {{
                       // Location 0: vertex position
                       0,                          // Binding number: ID of struct the location comes from
                       0,                          // Location number in layout
                       VK_FORMAT_R32G32B32_SFLOAT, // Descriptor type: `vec3`
                       offsetof(Vertex, pos),      // Offset: attribute location inside binding struct
                       sizeof(glm::vec3),          // Size of the attribute
                       POSITION                    // Usage: semantic attribute type (world coordinates)
                   },
                   {
                       // Location 1: vertex texture coordinates
                       0,                       // Binding number: ID of struct the location comes from
                       1,                       // Location number in layout
                       VK_FORMAT_R32G32_SFLOAT, // Descriptor type: `vec2`
                       offsetof(Vertex, UV),    // Offset: attribute location inside binding struct
                       sizeof(glm::vec2),       // Size of the attribute
                       UV                       // Usage: semantic attribute type (texture coordinates)
                   }});
}

void Vulcano::setDSPoolSize() {
    // [Inherited from BaseProject: this->DPSZs]
    this->DPSZs.uniformBlocksInPool = 2; // GUBO (in set 0) and UBO (in set 1)
    this->DPSZs.texturesInPool      = 1; // The texture in set 0
    this->DPSZs.setsInPool          = 2; // Set 0 (GUBO), set 1 (UBO, texture)
}

void Vulcano::referencesInit() {
    // Initialize vertex descriptor reference
    this->VDRs.resize(1);
    this->VDRs[0].init(
        "VDposUV", // VD layout identifier matching in scene JSON
        &this->VD  // VD layout to assign to models matching ID
    );

    // Register pipeline (technique) reference
    this->PRs.resize(1);
    this->PRs[0].init(
        "BlinnPos", // Technique (instances) identifier matching in scene JSON
        {{
            &this->P,             // Pipeline implementing the technique
            {{}, {{true, 0, {}}}} // DSL mapping (set 0, set 1 with texture at index 0)
        }},
        1,        // Number of textures required per instance in this technique (1, at index 0)
        &this->VD // Vertex descriptor required by this technique
    );
}

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
}

void Vulcano::loadScene() {
    std::string scenePath = "assets/scenes/Scene.json";
    logs::info("Loading scene ", scenePath);
    int err = this->SC.init(this,
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

void Vulcano::commandBuffersInit() {
    this->submitCommandBuffer("main",                               // Command buffer identifier
                              0,                                    // Execution step: lower IDs execute first (0 executes first)
                              Vulcano::populateCommandBufferAccess, // Callback populating GPU command buffer
                              this);
}

void Vulcano::textMakerInit() {
    this->txt.init(this, this->windowWidth, this->windowHeight);
    this->txt.print(-1.0f, -1.0f,
                    "FPS:", 1, "CO",
                    false, false, true,
                    TAL_LEFT, TRH_LEFT, TRV_TOP,
                    {1.0f, 0.0f, 0.0f, 1.0f},
                    {0.8f, 0.8f, 0.0f, 1.0f});
}

void Vulcano::populateCommandBufferAccess(VkCommandBuffer commandBuffer, int currentImage, void *Params) {
    // Simple trick to avoid having always 'T->' in the code that populates the
    // command buffer.
    Vulcano *T = (Vulcano *)Params;
    T->populateCommandBuffer(commandBuffer, currentImage);
}

void Vulcano::populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage) {
    // Populate command buffer with draw commands for the main on-screen render
    // pass: `SC.populateCommandBuffer` registers the commands for rending each
    // object in the scene.
    this->RP.begin(commandBuffer, currentImage);
    this->SC.populateCommandBuffer(commandBuffer, 0, currentImage);
    this->RP.end(commandBuffer);
}

void Vulcano::localCleanup() {
    this->DSLlocal.cleanup();
    this->DSLglobal.cleanup();
    this->P.destroy();
    this->RP.destroy();
    this->SC.localCleanup();
    this->txt.localCleanup();
}

void Vulcano::pipelinesAndDescriptorSetsInit() {
    // Create render pass and create pipeline linked to it
    this->RP.create();
    this->P.create(&this->RP);

    // Initialize GUBO DS instance
    this->DSglobal.init(this,
                        &this->DSLglobal, // Descriptor set layout for GUBO (set 0)
                        {}                // Textures: set 0 uses none
    );

    // Initialize pipelines and DSs for each instance in the loaded 3D scene
    this->SC.pipelinesAndDescriptorSetsInit();

    this->txt.pipelinesAndDescriptorSetsInit();
}

void Vulcano::pipelinesAndDescriptorSetsCleanup() {
    this->P.cleanup();
    this->RP.cleanup();
    this->DSglobal.cleanup();
    this->SC.pipelinesAndDescriptorSetsCleanup();
    this->txt.pipelinesAndDescriptorSetsCleanup();
}

void Vulcano::setWindowParameters() {
    this->windowWidth     = 1000;
    this->windowHeight    = 750;
    this->windowTitle     = "Campfire City";
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

int main() {
    auto app = std::make_unique<Vulcano>();

    try {
        app->run(false);
    } catch (const std::exception &e) {
        logs::error(e.what());
        return 1;
    }

    app.reset();
    return 0;
}
