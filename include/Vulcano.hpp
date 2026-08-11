#pragma once
#include "Camera.hpp"
#include "Descriptors.hpp"
#include "modules/Scene.hpp"
#include "modules/Starter.hpp"
#include "modules/TextMaker.hpp"

class Vulcano : public BaseProject {
  protected:
    // Player's camera position and looking direction.
    Camera camera;

    // These objects are used to define which set/binding layouts a shader is
    // able to declare.
    // When they call `init`, they define the bindings, the formats of the
    // layouts, and which shaders can use them.
    // When they call `map`, the object passed (UBO/GUBO) is transferred from
    // the CPU to the corresponding GPU layout, for shaders to use it.
    DescriptorSetLayout DSLlocal, DSLglobal;

    // Vertex descriptor differs from descriptor set layouts because they are
    // used for defining the "in" location layouts of the first vertex shader,
    // instead of sets/bindings.
    // This descriptor will define locations corresponding to attribute offsets
    // of the Vertex struct.
    VertexDescriptor VD;

    // A render pass is a set of pipelines, of which one is chosen for rendering
    // an object depending on its type (e.g. wooden table -> wood pipeline,
    // river -> water pipeline).
    // Multiple render passes can be executed sequentially for the generation
    // of a frame.
    // Regarding input and output of pipelines across render passes:
    //  - the last render pass' pipelines must always have
    //    `layout(location = 0) out vec4` (the pixel on screen);
    //  - in general, the "out layout location" of pipelines in internal
    //    (off-screen) render passes will have to be translated to descriptor
    //    sets (set/binding layouts) that following render passes' pipelines
    //    can access.
    RenderPass RP;

    // A pipeline is a set of two shaders (a vertex and a fragment shader)
    // which are executed on an object.
    // The vertex shader has to define out-locations identical to in-locations
    // of the fragment shader.
    // A pipeline needs to be linked to a render pass, which can contain many
    // and chooses one based on object type.
    // A pipeline requires a descriptor as input for defining in-locations of
    // the vertex shader (e.g. vertex descriptor), and the out-locations of the
    // fragment shaders are called attachments, which can either be used by
    // following render passes or for drawing on screen.
    Pipeline P;

    // `DSglobal` is an instance of `DSLglobal`: since `DSLglobal` maps to a
    // GUBO describing a direct light model shared across all the scene, a
    // single descriptor set is sufficient for mapping it to all the shaders.
    // Side note: there's no `DSlocal` because, unlike `DSglobal`, each object
    // needs a different one; indeed, when scene is initialized, each instance
    // receives its DS object to use.
    DescriptorSet DSglobal;

    // A `Scene` is a helper object which encapsulates logic for loading assets
    // from a JSON scene file.
    // It can load models/meshes (.gltf, .obj, .mgf), textures (.jpg, .png,
    // .tga), and the definition of instances, which are the actual objects
    // found in the world. Each instance is associated with a model, textures
    // and its transformations (for the initial world matrix).
    // After initialization, the scene object grants access to all the loaded
    // content and prepares a descriptor set for each of the instances.
    Scene SC;

    // Under the same technique you can bind more than a single pipeline; indeed
    // you should bind all the pipelines of the render passes for that object.
    // A `TechniqueRef` is a helper object used by a `Scene` to link technique
    // IDs to the instances using it and the pipelines that produce such
    // technique.
    // In the scene JSON, instances are grouped by technique, and such instances
    // should share the same pipelines, which are linked to them thanks to this
    // reference object.
    // Side note: although they also bind pipelines together, `PRs` differs from
    // a render pass for the following reasons.
    //  - Render passes define the global order with which pipelines are
    //    executed (e.g., all shadows first, then all screen colors...).
    //    Not all models require all the render passes, and some render passes
    //    are dependent from ones before (to draw all colors, all shadow
    //    pipelines need to be executed first).
    //  - `PRs` simply tells each object which shader pipeline to pick when
    //    a specific render pass is executing. They do not tell which render
    //    pass the pipelines are in, so they cannot enforce the correct
    //    execution sequence by themselves.
    //  - Pipelines are low-level objects, which generalize a set of Vulkan
    //    directives for drawing on screen, whereas `PRs` bind the generic
    //    pipeline objects to specific instances of the scene.
    std::vector<TechniqueRef> PRs;

    // `VDRs` are used for binding a vertex descriptor to models in scene JSON.
    // They have the same purpose of `TechniqueRef`s (`PRs`) but binding vertex
    // descriptors instead of pipelines.
    // The models sharing the VD ID will have them same vertex descriptor format
    // as input.
    // Since only a single vertex descriptor (model/mesh) can exist per
    // identifier, a `VertexDescriptorRef` binds an ID with a single VD, unlike
    // `PRs` which bind an ID to the multiple pipelines of that instance.
    std::vector<VertexDescriptorRef> VDRs;

    // A `TextMaker` is a useful tool used for drawing text onto the screen.
    // It encapsulates all the logic required for such task, exposing just a
    // simple `print` function with which such text can be positioned and drawn.
    TextMaker txt;

    // Aspect ratio used for computing the correct projection matrix.
    float Ar;

    // The view matrix V is used to describe the camera.
    // It encodes the position, the direction to which it's looking, and the up
    // vector.
    glm::mat4 View;

    // The projection matrix P is used to describe the region of the world that
    // needs to be rendered to the screen.
    // It encodes FOV, aspect ratio, and near and far plane.
    glm::mat4 Prj;

    // The view-projection matrix PV combines the view matrix V with the
    // projection matrix P.
    // The PV matrix is used for computing the screen coordinates for every
    // object and vertex in each frame.
    glm::mat4 ViewPrj;

    // Initial configuration of window dimensions, title, resizability.
    void setWindowParameters() override;

    // Window resize callback.
    void onWindowResize(int w, int h) override;

    // Program initialization: load scene and setup descriptors, pipelines and
    // render passes.
    void localInit() override;

    // Create actual objects for pipelines, render passes, descriptor sets (both
    // global and for each instance).
    // Also, link pipelines to their render passes, and link DS to DSL.
    void pipelinesAndDescriptorSetsInit() override;

    // Destroy objects created with `localInit`.
    void localCleanup() override;

    // Destroy objects created with `pipelinesAndDescriptorSetsInit`.
    void pipelinesAndDescriptorSetsCleanup() override;

    // Per-frame uniform buffer update hook.
    void updateUniformBuffer(u32 currentImage) override;

    // Callback forwarding to `populateCommandBuffer`.
    static void populateCommandBufferAccess(VkCommandBuffer commandBuffer, int currentImage, void *Params);

    // Record GPU draw commands into the command buffer for the current frame,
    // binding render passes, pipelines, scene objects, and descriptor sets.
    void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage);

    // Execute main frame logic (possibly requiring frame time delta seconds
    // between current and previous frame).
    void frameLogic(const float deltaT);

  private:
    // Initialize all required descriptor layouts.
    void descriptorsInit();

    // Initialization of the descriptor set layout for local/per-object
    // uniforms (read-only, per render pass, constant memory).
    // The `DLSlocal` is used to arrange UBOs and textures from each
    // instance.
    // Its initialization defines the binding numbers, but not the set's
    // (defined later when initializing the pipeline).
    void DSLlocalInit();

    // Initialization of the descriptor set layout for the global/per-scene
    // uniform.
    // The `DLSglobal` is used to arrange the GUBO describing the scene's
    // direct light model.
    void DSLglobalInit();

    // Initialization of the vertex descriptor layout to define the input
    // vertex data of objects.
    // Unlike `DSLglobal` and `DSLlocal`, `VD` doesn't bind data to
    // set/bindings but to (in) location layouts.
    // Side note: "bindings" in the context of vertex descriptors are not
    // representing sectors of sets like in descriptor set layouts; instead,
    // they are used for identifying the structs of which offsets
    // (locations) are taken.
    void VDInit();

    // Set the size of the descriptor set pool.
    // The type and the count of different descriptor sets used in the rendering
    // must be known ahead of time to allocate the correct quantity of memory.
    // This must be done before loading the scene.
    void setDSPoolSize();

    // Initialization of the vertex descriptor and the pipeline/technique
    // references.
    void referencesInit();

    // Initialization of pipelines and render passes.
    void pipelinesAndRenderPassesInit();

    // Load 3D scene models, textures and instances/techniques from JSON file.
    void loadScene();

    // Submit command buffers: each subsystem (world render, collisions,
    // text...) can have its own command buffer, for enhanced modularity, and
    // based on given priorities they are executed in order by the GPU.
    void commandBuffersInit();

    // Initialize the textual output and show FPS count in top-left corner.
    void textMakerInit();

    // Set initial window settings, such as hide and lock mouse inside window.
    void windowSettingsInit();

    // Compute time delta between last frame and current.
    float getDeltaT();

    // Map each available key press event to the callback function it activates.
    // Excluded player movements and mouse pointing direction.
    void keypressCallbacks(float deltaT);

    // Map keys and mouse events to movement and rotation vectors.
    void cameraUpdate(float deltaT);

    // Compute projection matrix, view matrix from camera and combine them.
    void computeViewProj();

    // Must be called after updating view matrix.
    // Update GUBO parameters and construct new GUBO (scene's direct light model).
    void updateGlobalLight(GlobalUniformBufferObject &gubo, float deltaT, int currentImage);

    // Update UBO and GUBO for each instance.
    void updateSceneInstances(GlobalUniformBufferObject gubo, int currentImage);

    // Update FPS on-screen counter each second.
    void updateFPS(float deltaT);
};
