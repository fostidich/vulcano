#version 450
#extension GL_ARB_separate_shader_objects : enable

// Humans are more sensible to dark colors. Therefore displays, which use sRGB
// (standard RGB), compress color in the high brightness range with a power
// function with 2.2 as exponent.
// Computations on lights require instead linear colors.

layout(binding = 0, set = 0) uniform GlobalUniformBufferObject {
    vec3 lightDir; // World space
    vec4 lightColor; // Linear color space
    vec3 eyePos; // World space
} gubo;

layout(binding = 1, set = 1) uniform sampler2D albedoMap; // Display color space

layout(location = 0) in vec3 fragPos; // World space
layout(location = 1) in vec2 fragUV; // Texture space
layout(location = 0) out vec4 outColor; // Display color space

const float specularStrength = 0.005;
const float shininess = 10;
const float ambience = 0.015;

void main() {
    vec3 X = dFdx(fragPos); // Coordinates difference between this pixel and the one on its right
    vec3 Y = dFdy(fragPos); // Coordinates difference between this pixel and the one below
    vec3 N = -normalize(cross(X, Y)); // Normal vector of the geometric face

    vec3 albedo = pow(texture(albedoMap, fragUV).rgb, vec3(2.2)); // Gamma expansion: convert sRGB to linear space

    vec3 V = normalize(gubo.eyePos - fragPos); // Vector from fragment to viewer
    vec3 L = normalize(-gubo.lightDir); // Vector from fragment to light
    vec3 H = normalize(V + L); // Half vector
    vec3 radiance = gubo.lightColor.rgb; // Light color

    float NdotL = max(dot(N, L), 0.0); // Clipped cosine of angle between vectors
    float HdotN = max(dot(H, N), 0.0); // Clipped cosine of angle between vectors

    vec3 specular = vec3(specularStrength * pow(HdotN, shininess)); // Intensity of reflected light
    vec3 diffuse = albedo * NdotL; // Intensity of object reflection
    vec3 Lo = (diffuse + specular) * radiance; // Fragment direct color lights

    vec3 ambient = ambience * albedo; // Base ambience lower threshold light
    vec3 color = ambient + Lo; // Total light (ambience + direct)
    color = color / (color + vec3(1.0)); // Compression of high dynamic range to (0, 1)
    color = pow(color, vec3(1.0 / 2.2)); // Gamma correction: conversion of linear colors back to display space

    outColor = vec4(color, 1.0); // Output final fragment color
}
