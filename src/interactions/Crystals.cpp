#include "Descriptors.hpp"
#include "Vulcano.hpp"

void Vulcano::updateCrystals() {
    world.crystalBonesDistance = length(world.crystalBonesPosition - player.eyePosition);
    if (world.crystalBonesDistance > world.ring2) return;

    static std::unordered_map<std::string_view, PointLight> crystals;

    // Cache the point light information of a crystal, the first time
    if (crystals.empty())
        for (const auto &id : world.crystalsID)
            crystals[id] = world.pointLights[id];

    // Force reset point lights if in ring 2
    if (world.crystalBonesDistance > world.ring1) {
        for (const auto &id : world.crystalsID)
            world.pointLights[id] = crystals[id];
        return;
    }

    // Change light intensity based on distance to bones, in ring 1
    const float intensityFactor = 100.0f - 99.0f * (world.crystalBonesDistance / world.ring1); // Twice as intense when in the center
    const float decayFactor     = 5.0f - 4.0f * (world.crystalBonesDistance / world.ring1);    // Twice as intense when in the center
    for (const auto &id : world.crystalsID) {
        PointLight pl = crystals[id];
        pl.color *= intensityFactor;
        pl.decay *= decayFactor;
        world.pointLights[id] = pl;
    }
}
