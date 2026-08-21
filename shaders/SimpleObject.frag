#version 450
#extension GL_ARB_separate_shader_objects : enable

// Humans are more sensible to dark colors. Therefore displays, which use sRGB
// (standard RGB), compress color in the high brightness range with a power
// function with 2.2 as exponent.
// Computations on lights require instead linear colors.

struct PointLight {
    vec3 position; // World space
    vec3 color; // Linear space
    float target;
    float decay;
};

layout(binding = 0, set = 0) uniform GlobalUniformBufferObject {
    mat4 lightSpaceMat;
    vec3 lightDir; // World space
    vec4 lightColor; // Linear color space
    vec3 eyePos; // World space
    PointLight pointLights[16];
    int pointLightsCount;
} gubo;

layout(binding = 1, set = 0) uniform sampler2D shadowMap; // Shadow off-screen attachment
layout(binding = 1, set = 1) uniform sampler2D albedoMap; // Display color space

layout(location = 0) in vec3 fragPos; // World space
layout(location = 1) in vec2 fragUV; // Texture space
layout(location = 0) out vec4 outColor; // Display color space

const float specularStrength = 0.005;
const float shininess = 10.0;
const float ambience = 0.015;

float computeShadow(vec3 worldPos, vec3 normal, vec3 lightDirection);

void main() {
    vec3 X = dFdx(fragPos); // Coordinates difference between this pixel and the one on its right
    vec3 Y = dFdy(fragPos); // Coordinates difference between this pixel and the one below
    vec3 N = -normalize(cross(X, Y)); // Normal vector of the geometric face

    vec3 albedo = pow(texture(albedoMap, fragUV).rgb, vec3(2.2)); // Gamma expansion: convert sRGB to linear space
    vec3 ambient = ambience * albedo; // Base ambience lower threshold light
    vec3 radiance = gubo.lightColor.rgb; // Light color

    vec3 V = normalize(gubo.eyePos - fragPos); // Vector from fragment to viewer
    vec3 L = normalize(-gubo.lightDir); // Vector from fragment to light
    vec3 H = normalize(V + L); // Half vector

    float NdotL = max(dot(N, L), 0.0); // Clipped cosine of angle between vectors
    float HdotN = max(dot(H, N), 0.0); // Clipped cosine of angle between vectors

    vec3 specular = vec3(specularStrength * pow(HdotN, shininess)); // Intensity of reflected light
    vec3 diffuse = albedo * NdotL; // Intensity of object reflection
    float shadow = computeShadow(fragPos, N, L); // Compute shadow at fragment world coordinates
    vec3 directLight = (diffuse + specular) * radiance * shadow; // Fragment direct color lights

    vec3 pointLight = vec3(0.0);
    for (int i = 0; i < gubo.pointLightsCount; ++i) {
        vec3 delta = gubo.pointLights[i].position - fragPos;
        float dist = max(length(delta), 0.001);

        vec3 pL = normalize(delta);
        vec3 pH = normalize(V + pL);
        float pNdotL = max(dot(N, pL), 0.0);
        float pNdotH = max(dot(N, pH), 0.0);

        // Distance attenuation: (g/d)^beta
        float att = pow(gubo.pointLights[i].target / dist, gubo.pointLights[i].decay);
        vec3 pRadiance = gubo.pointLights[i].color * att;

        vec3 pDiffuse = albedo * pNdotL;
        vec3 pSpecular = vec3(specularStrength * pow(pNdotH, shininess));
        pointLight += (pDiffuse + pSpecular) * pRadiance;
    }

    vec3 color = ambient + directLight + pointLight; // Total light on fragment
    color = color / (color + vec3(1.0)); // Compression of high dynamic range to (0, 1)
    color = pow(color, vec3(1.0 / 2.2)); // Gamma correction: conversion of linear colors back to display space
    outColor = vec4(color, 1.0); // Output final fragment color
}

float computeShadow(vec3 worldPos, vec3 normal, vec3 lightDirection) {
    // Transform world position to light clip space
    vec4 lightSpacePos = gubo.lightSpaceMat * vec4(worldPos, 1.0);

    // Perspective divide to NDC space
    vec3 projCoords = lightSpacePos.xyz / lightSpacePos.w;

    // Transform XY from (-1, 1) to (0, 1) for texture coordinates (Z is already (0, 1))
    vec2 shadowUV = projCoords.xy * 0.5 + 0.5;
    float currentDepth = projCoords.z;

    // If fragment is outside light frustum, we don't know if it's in shadow
    if (shadowUV.x < 0.0 || shadowUV.x > 1.0 || shadowUV.y < 0.0 || shadowUV.y > 1.0 ||
            currentDepth > 1.0 || currentDepth < 0.0)
        return 0.2;

    // Slope-scale depth bias to prevent shadow acne: fragment distance from
    // light is slightly reduced so depth testing on shadow map doesn't fail
    // for small floating point precision errors.
    // Bias depends on the steepness of the surface with regards to the light
    // source: if light is perpendicular, no bias; if surface is extra steep,
    // max bias is required.
    float bias = max(0.0001 * (1.0 - dot(normal, lightDirection)), 0.0005);

    // Percentage-closer filtering (PCF) 3x3 kernel for soft shadow edges
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            float closestDepth = texture(shadowMap, shadowUV + vec2(x, y) * texelSize).r;
            shadow += (currentDepth - bias > closestDepth) ? 0.0 : 1.0;
        }
    }
    return shadow / 9.0;
}
