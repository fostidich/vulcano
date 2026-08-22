
vec3 computeDirectLight(vec3 N, vec3 L, vec3 H, vec3 albedo, vec3 radiance, float shadow) {
    float NdotL = max(dot(N, L), 0.0); // Clipped cosine of angle between vectors
    float HdotN = max(dot(H, N), 0.0); // Clipped cosine of angle between vectors
    vec3 specular = vec3(specularStrength * pow(HdotN, shininess)); // Intensity of reflected light
    vec3 diffuse = albedo * NdotL; // Intensity of object reflection
    return (diffuse + specular) * radiance * shadow;
}

vec3 computePointLights(vec3 fragPos, vec3 N, vec3 V, vec3 albedo, int plc, PointLight plv[16]) {
    vec3 pointLight = vec3(0.0);
    for (int i = 0; i < plc; ++i) {
        vec3 delta = plv[i].position - fragPos;
        float dist = max(length(delta), 0.001);

        vec3 pL = normalize(delta);
        vec3 pH = normalize(V + pL);
        float pNdotL = max(dot(N, pL), 0.0);
        float pNdotH = max(dot(N, pH), 0.0);

        // Distance attenuation: (g/d)^beta
        float att = pow(plv[i].target / dist, plv[i].decay);
        vec3 pRadiance = plv[i].color * att;

        vec3 pDiffuse = albedo * pNdotL;
        vec3 pSpecular = vec3(specularStrength * pow(pNdotH, shininess));
        pointLight += (pDiffuse + pSpecular) * pRadiance;
    }
    return pointLight;
}

float computeShadow(vec3 fragPos, vec3 N, vec3 L, float farShadow) {
    // Transform world position to light clip space
    vec4 lightSpacePos = gubo.lightSpaceMat * vec4(fragPos, 1.0);

    // Perspective divide to NDC space
    vec3 projCoords = lightSpacePos.xyz / lightSpacePos.w;

    // Transform XY from (-1, 1) to (0, 1) for texture coordinates (Z is already (0, 1))
    vec2 shadowUV = projCoords.xy * 0.5 + 0.5;
    float currentDepth = projCoords.z;

    // If fragment is outside light frustum, we don't know if it's in shadow
    if (shadowUV.x < 0.0 || shadowUV.x > 1.0 || shadowUV.y < 0.0 || shadowUV.y > 1.0 ||
            currentDepth > 1.0 || currentDepth < 0.0)
        return farShadow;

    // Slope-scale depth bias to prevent shadow acne: fragment distance from
    // light is slightly reduced so depth testing on shadow map doesn't fail
    // for small floating point precision errors.
    // Bias depends on the steepness of the surface with regards to the light
    // source: if light is perpendicular, no bias; if surface is extra steep,
    // max bias is required.
    float bias = max(0.0001 * (1.0 - dot(N, L)), 0.0005);

    // Percentage-closer filtering (PCF) 3x3 kernel for soft shadow edges
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            float closestDepth = texture(shadowMap, shadowUV + vec2(x, y) * texelSize).r;
            shadow += (currentDepth - bias > closestDepth) ? 0.0 : 1.0;
        }
    }
    shadow /= 9.0;

    // Smoothly blend towards far shadow in the outer margin of the shadow map
    vec2 borderDist = min(shadowUV, 1.0 - shadowUV);
    float edgeFactor = min(borderDist.x, borderDist.y);
    float blend = smoothstep(0.0, 0.05, edgeFactor);
    return mix(farShadow, shadow, blend);
}
