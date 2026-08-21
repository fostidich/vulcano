#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0, set = 0) uniform GlobalUniformBufferObject {
    mat4 lightSpaceMat;
} gubo;

layout(binding = 0, set = 1) uniform UniformBufferObject {
    mat4 mvpMat;
    mat4 mMat;
} ubo;

layout(location = 0) in vec3 inPosition;

void main() {
    gl_Position = gubo.lightSpaceMat * ubo.mMat * vec4(inPosition, 1.0);
}
