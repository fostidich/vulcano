#include "Vulcano.hpp"
#include "modules/Animations.hpp"
#include <glm/glm.hpp>

using namespace glm;
using namespace std;

namespace {
void toggle(bool &b) { b = !b; }

bool contains(const auto &range, const auto &val) {
    return std::find(std::begin(range), std::end(range), val) != std::end(range);
}
} // namespace
void printCenterBottom(TextMaker &txt, int id, const string &s) {
    txt.print(0.0f, 0.9f,
              s, id, "CO",
              false, false, false,
              TAL_CENTER, TRH_CENTER, TRV_BOTTOM,
              {1.0f, 1.0f, 1.0f, 1.0f},
              {0.0f, 0.0f, 0.0f, 1.0f});
}

void Vulcano::drawDeersBridge() {
    // Fired once when pressing E near the deer.
    if (world.deerDistance > world.minDistanceForAnimation) return;
    if (world.deerBridgeMoving) return;
    world.deerBridgeMoving = true;
    logs::info("Deer's bridge requested to be ", world.deerBridgeRaised ? "lowered" : "raised");
}

void Vulcano::updateDeersBridge(float deltaT) {
    // Executed on every frame, to smoothly animate the bridge when requested
    // to be lowered/raised.
    world.deerDistance = length(world.deerPosition - player.eyePosition); // Keep track of how far the deer is
    if (!world.deerBridgeMoving) return;

    const float duration     = 3.0f;
    const float targetHeight = 7.5f * (world.deerBridgeRaised ? -1.0f : 1.0f);
    static float currentTime = 0.0f;
    static vector<pair<Instance *, mat4>> initialWms; // World matrix of objects before animation
    static AnimTrack moveTrack;                       // Animation tracker for interpolating position in time

    if (currentTime == 0.0f) {
        // Cache the initial transformation matrices on first run
        for (int k = 0; k < SC.TechniqueInstanceCount; k++)
            for (int i = 0; i < SC.TI[k].InstanceCount; i++)
                if (SC.TI[k].I[i].id && contains(world.deerBridgesID, *SC.TI[k].I[i].id))
                    initialWms.emplace_back(&SC.TI[k].I[i], SC.TI[k].I[i].Wm);

        // Setup animation tracker at the start of the animation
        moveTrack.Frames.clear();
        moveTrack.nKeyFrames = 2; // Two points to interpolate
        moveTrack.Frames.push_back({
            0.0f,                         // Time
            vec3(0.0f, 0.0f, 0.0f),       // Translation
            quat(1.0f, 0.0f, 0.0f, 0.0f), // Rotation
            vec3(1.0f, 1.0f, 1.0f)        // Scaling
        });
        moveTrack.Frames.push_back({
            duration,                       // Time duration
            vec3(0.0f, targetHeight, 0.0f), // Translation delta
            quat(1.0f, 0.0f, 0.0f, 0.0f),   // No rotation
            vec3(1.0f, 1.0f, 1.0f)          // No scaling
        });
    }

    // Compute new matrix interpolating based on current frame sample time
    currentTime += deltaT;
    const float sampleTime  = std::min(currentTime, duration);
    const mat4 animationMat = moveTrack.Sample(sampleTime, 0, -1, false);

    // Combine current animation transform with initial matrix of object
    for (auto &[inst, baseWm] : initialWms) {
        const mat4 newWm = animationMat * baseWm;
        inst->Wm         = newWm;                    // Update object world matrix
        if (inst->C) inst->C->setWorldMatrix(newWm); // Update physical hitbox as well
    }

    // Refresh colliders guides also
    if (player.showColliders) SC.refreshColliderVisualizer();

    // Conclude animation
    if (currentTime >= duration) {
        currentTime = 0.0f;
        initialWms.clear();
        moveTrack.Frames.clear();
        world.deerBridgeMoving = false;
        toggle(world.deerBridgeRaised);
        logs::info("Deer's bridge ", world.deerBridgeRaised ? "raised" : "lowered");
    }
}

void Vulcano::printDeerText() {
    // Executed on every frame for correctly print messages for interacting with
    // the deer.
    if (!world.deerBridgeMoving) {
        // Check if player is near enough for displaying the message or for
        // requesting raising/lowering the bridge (if it's not moving already).
        if (world.deerDistance > world.minDistanceForMessage) {
            txt.removeText(2);
            return;
        }
        if (world.deerDistance > world.minDistanceForAnimation) {
            const string dist = std::format("[{:.2f}m]", world.deerDistance);
            printCenterBottom(txt, 2, "Talk with the deer " + dist);
            return;
        }
        const string req = world.deerBridgeRaised ? "lower" : "raise";
        printCenterBottom(txt, 2, "Hi wanderer, press [E] to " + req + " the bridge");
    } else {
        const string status = world.deerBridgeRaised ? "down" : "up";
        printCenterBottom(txt, 2, "Bridge is going " + status);
    }
}
