#include "Vulcano.hpp"

void Vulcano::descriptorsInit() {
    this->DSLlocalInit();
    this->DSLglobalInit();
    this->VDInit();
}

void Vulcano::DSLlocalInit() {
    // Init a set with two bindings: objects' UBO and texture
    this->DSLlocalPosUV.init(this,
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

    // Set with only UBO and no texture (terrain)
    this->DSLlocalPos.init(this,
                           {{
                               // Binding 0: objects' UBO
                               0,                                 // Binding 0
                               VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, // UBO descriptor
                               VK_SHADER_STAGE_VERTEX_BIT,        // Vertex shader
                               sizeof(UniformBufferObject),       // Size of descriptor
                               1                                  // Not an array
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
    this->VDposUV.init(this,
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

    // Position only VD (for terrain)
    this->VDpos.init(this,
                     {{
                         // Binding 0: input vertices
                         0,                          // Binding 0
                         sizeof(glm::vec3),          // Stride: only a `vec3` size for position
                         VK_VERTEX_INPUT_RATE_VERTEX // Step by vertex
                     }},
                     {{
                         // Location 0: position in vertex shader
                         0,                          // Binding 0
                         0,                          // Location 0
                         VK_FORMAT_R32G32B32_SFLOAT, // Type is `vec3`
                         0,                          // Offset 0: one attribute
                         sizeof(glm::vec3),          // Size of the only attribute,
                         POSITION                    // World coordinates
                     }});
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
    this->VDRs[1].init(
        "VDpos",     // VD layout identifier matching in scene JSON
        &this->VDpos // VD layout to assign to models matching ID
    );

    // Register pipeline (technique) references
    this->PRs.resize(2);
    this->PRs[0].init(
        "SimpleObject", // Technique (instances) identifier matching in scene JSON
        {{
            &this->PsimpleObject, // Pipeline implementing the technique
            {{}, {{true, 0, {}}}} // DSL mapping (set 0, set 1 with texture at index 0)
        }},
        1,             // Number of textures required per instance in this technique (1, at index 0)
        &this->VDposUV // Vertex descriptor required by this technique
    );
    this->PRs[1].init(
        "Terrain", // Technique (instances) identifier matching in scene JSON
        {{
            &this->Pterrain, // Pipeline implementing the technique
            {{}, {}}         // DSL mapping (set 0, set 1), no textures
        }},
        0,           // Number of textures required per instance in this technique (none)
        &this->VDpos // Vertex descriptor required by this technique
    );
}
