#version 450
#extension GL_ARB_separate_shader_objects : enable

//  - Model space: loaded model unbounded XYZ coordinates (e.g. (3.4, 5, -9))
//  - World space: scene instance unbound XYZ coordinates (e.g. (3.4, 5, -9))
//  - Clip space: scene XYZW coordinates (e.g. (0.9, -0.01, 2, 3))
//      - Can be transformed in normalized device coordinates (NDC: (x/w, y/w, z/w))
//      - Coordinates outside frustum will be clipped
//  - Texture space: image XY coordinates (e.g. (0.1, 1.1))
//      - Coordinates outside (0, 1) range require adaptations

layout(binding = 0, set = 1) uniform UniformBufferObject {
    mat4 mvpMat;
    mat4 mMat;
} ubo;

layout(location = 0) in vec3 inPosition; // Model space
layout(location = 1) in vec2 inUV; // Texture space
layout(location = 0) out vec3 fragPos; // World space
layout(location = 1) out vec2 fragUV; // Texture space

void main() {
    gl_Position = ubo.mvpMat * vec4(inPosition, 1.0); // Clip space
    fragPos = (ubo.mMat * vec4(inPosition, 1.0)).xyz; // World space
    fragUV = inUV; // Texture space
}

// Vertex shader is generally required to compute `gl_Position`.
// Between vertex and fragment shaders, `gl_Position` clip space coordinates
// will be converted to NDC, points outside frustum are discarded, and if the
// fragment shader does not use fragment depth, a early z-test is done to
// discard covered points.
// Fragment shader runs only for surviving pixels.
