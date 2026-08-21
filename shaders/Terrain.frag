#version 450
#extension GL_ARB_separate_shader_objects : enable

// The terrain model currently used has a vertex height always comprised betwee
// -10 and 100. Colors are computed based on their height and the vertex normal
// vector.
// The idea is that the model is placed at height 0, with a water plane present
// at height 1, and sand covering all heights below 2.
// Side note: the water layer will have a collider placed one meter below it, at
// layer 0.

struct PointLight {
    vec3 position;
    vec3 color;
    float target;
    float decay;
};

layout(binding = 0, set = 0) uniform GlobalUniformBufferObject {
    mat4 lightSpaceMat;
    vec3 lightDir;
    vec4 lightColor;
    vec3 eyePos;
    PointLight pointLights[16];
    int pointLightsCount;
} gubo;

layout(binding = 1, set = 0) uniform sampler2D shadowMap;

layout(location = 0) in vec3 fragPos;
layout(location = 0) out vec4 outColor;

// Lights properties
const float specularStrength = 0.005;
const float shininess = 10.0;
const float ambience = 0.015;

// Procedural albedo palette (converted to linear space)
const vec3 colSand = pow(vec3(0.35, 0.28, 0.18), vec3(2.2));
const vec3 colGrass = pow(vec3(0.05, 0.16, 0.05), vec3(2.2));
const vec3 colRock = pow(vec3(0.20, 0.19, 0.19), vec3(2.2));
const vec3 colSnow = pow(vec3(0.85, 0.88, 0.92), vec3(2.2));

// Terrain peak altitude in meters
// This needs to be changed if terrain max height gets updated.
const float maxHeight = 256.0;

// Normalized height ranges defined on a 0-100 scale
const vec2 rangeSandToGrass = vec2(0.0, 2.5);
const vec2 rangeGrassToRock = vec2(30.0, 50.0);
const vec2 rangeRockToSnow = vec2(80.0, 100.0);

float computeShadow(vec3 worldPos, vec3 normal, vec3 lightDirection);

void main() {
    vec3 X = dFdx(fragPos);
    vec3 Y = dFdy(fragPos);
    vec3 N = -normalize(cross(X, Y));

    // For simplicity, scale normalized 0-100 ranges according to max height
    float hScale = maxHeight / 100.0;
    vec2 transSandToGrass = rangeSandToGrass * hScale;
    vec2 transGrassToRock = rangeGrassToRock * hScale;
    vec2 transRockToSnow = rangeRockToSnow * hScale;

    // Height-based blending: start with base sand and blend upwards
    float h = fragPos.y;
    vec3 albedo = colSand;
    albedo = mix(albedo, colGrass, smoothstep(transSandToGrass.x, transSandToGrass.y, h));
    albedo = mix(albedo, colRock, smoothstep(transGrassToRock.x, transGrassToRock.y, h));
    albedo = mix(albedo, colSnow, smoothstep(transRockToSnow.x, transRockToSnow.y, h));

    // Slope-based blending: very sharp cutoff for steep rock cliffs
    float flatness = clamp(N.y, 0.0, 1.0);
    float slopeRockWeight = 1.0 - smoothstep(0.50, 0.60, flatness);
    albedo = mix(albedo, colRock, slopeRockWeight);

    vec3 V = normalize(gubo.eyePos - fragPos);
    vec3 L = normalize(-gubo.lightDir);
    vec3 H = normalize(V + L);

    float NdotL = max(dot(N, L), 0.0);
    float HdotN = max(dot(H, N), 0.0);

    vec3 ambient = ambience * albedo;
    vec3 radiance = gubo.lightColor.rgb;

    vec3 specular = vec3(specularStrength * pow(HdotN, shininess));
    vec3 diffuse = albedo * NdotL;
    float shadow = computeShadow(fragPos, N, L);
    vec3 directLight = (diffuse + specular) * radiance * shadow;

    vec3 pointLight = vec3(0.0);
    for (int i = 0; i < gubo.pointLightsCount; ++i) {
        vec3 delta = gubo.pointLights[i].position - fragPos;
        float dist = max(length(delta), 0.001);

        vec3 pL = normalize(delta);
        vec3 pH = normalize(V + pL);
        float pNdotL = max(dot(N, pL), 0.0);
        float pNdotH = max(dot(N, pH), 0.0);

        float att = pow(gubo.pointLights[i].target / dist, gubo.pointLights[i].decay);
        vec3 pRadiance = gubo.pointLights[i].color * att;

        vec3 pDiffuse = albedo * pNdotL;
        vec3 pSpecular = vec3(specularStrength * pow(pNdotH, shininess));
        pointLight += (pDiffuse + pSpecular) * pRadiance;
    }

    vec3 color = ambient + directLight + pointLight;
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2));
    outColor = vec4(color, 1.0);
}

float computeShadow(vec3 worldPos, vec3 normal, vec3 lightDirection) {
    vec4 lightSpacePos = gubo.lightSpaceMat * vec4(worldPos, 1.0);
    vec3 projCoords = lightSpacePos.xyz / lightSpacePos.w;

    vec2 shadowUV = projCoords.xy * 0.5 + 0.5;
    float currentDepth = projCoords.z;
    if (shadowUV.x < 0.0 || shadowUV.x > 1.0 || shadowUV.y < 0.0 || shadowUV.y > 1.0 ||
            currentDepth > 1.0 || currentDepth < 0.0)
        return 0.7;

    float bias = max(0.0001 * (1.0 - dot(normal, lightDirection)), 0.0005);

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
