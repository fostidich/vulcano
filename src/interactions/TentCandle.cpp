#include "Descriptors.hpp"
#include "Vulcano.hpp"

void Vulcano::updateTentCandle() {
    world.tentCandleDistance = length(world.tentCandleEntrancePos - player.eyePosition);

    static std::optional<PointLight> candleLight = std::nullopt;
    static bool active                           = false;

    // Cache point light struct at start
    if (!candleLight.has_value()) {
        candleLight = world.pointLights[world.tentCandleID];
        world.pointLights.erase(world.tentCandleID);
    }
    if (world.tentCandleDistance > world.ring2) return;

    // If in ring 2, remove candle's point light, if present still
    if (world.tentCandleDistance > world.ring1) {
        if (!active) return; // Early exit if removed already
        world.pointLights.erase(world.tentCandleID);
        active = false;
    }
    // If in ring 1, re-add point light
    else {
        if (active) return; // Early exit if present already
        world.pointLights[world.tentCandleID] = candleLight.value();
        active                                = true;
    }
}

void Vulcano::printCandleText() {
    if (world.tentCandleDistance > world.ring1) return;
    txt.printBottomCenter("Be careful where you look...");
}
