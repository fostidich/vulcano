#include "Descriptors.hpp"
#include "Logs.hpp"
#include "Types.hpp"
#include "Vulcano.hpp"
#include <optional>

using namespace glm;
using namespace std;

// This is the function in which to define all the possible identifier for point
// light sources, for mapping those instances to their corresponding light model.
std::optional<PointLight> initPointLight(const Instance &instance) {
    PointLight pl;
    vec3 pos = glm::vec3(instance.Wm[3]);

    if (instance.id->starts_with("candle/")) {
        // World position: column 3 of the world matrix
        // Add a small Y offset so the light emits from the wick
        pl.position = pos + vec3(0.0f, 0.35f, 0.0f);
        pl.color    = vec3(1.0f, 0.6f, 0.2f) * 5.0f; // Linear color and intensity (warm candle flame)
        pl.target   = 3.0f;                          // Reference target distance (g)
        pl.decay    = 2.0f;                          // Distance Decay Exponent (beta)
    }
    if (instance.id->starts_with("crystal/")) {
        pl.position = pos;
        pl.color    = vec3(0.6f, 0.1f, 0.9f) * 2.5f;
        pl.target   = 2.0f;
        pl.decay    = 3.0f;
    }

    else {
        return std::nullopt;
    }
    return pl;
}

void Vulcano::extractPointLights() {
    for (int t = 0; t < this->SC.TechniqueInstanceCount; t++) {
        for (int i = 0; i < this->SC.TI[t].InstanceCount; i++) {
            const auto &instance = this->SC.TI[t].I[i];
            if (world.pointLights.size() > MAX_POINT_LIGHTS) {
                logs::error("Number of point lights exceeded: ", MAX_POINT_LIGHTS);
                exit(1);
            }
            const auto pl = initPointLight(instance);
            if (pl.has_value()) world.pointLights[*instance.id] = pl.value();
        }
    }
}

void Vulcano::updateSceneLights(GlobalUniformBufferObject &gubo, float deltaT) {
    gubo.eyePos = player.eyePosition;

    // Construct GUBO with direct light model
    static float dayTime    = 0.0f;
    const float dayDuration = 5.0f * 60.0f; // 5 minutes
    dayTime                 = mod(dayTime + deltaT, dayDuration);
    const float rot         = radians(360.0f) * dayTime / dayDuration;
    gubo.lightDir           = vec3(cos(rot), -1.0f, sin(rot));
    gubo.lightColor         = vec4(1.0f, 1.0f, 0.5f, 1.0f) * 6.0f; // Warm light

    // Copy point lights with GUBO
    gubo.pointLightsCount = this->world.pointLights.size();
    usize i               = 0;
    for (const auto &[key, val] : this->world.pointLights)
        gubo.pointLights[i++] = val;

    // full shine when the player enters ring1 around stone
    gubo.swordShine = 1.0f - glm::clamp(this->world.swordStoneDistance / this->world.ring1, 0.0f, 1.0f);
}
