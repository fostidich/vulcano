#include "Vulcano.hpp"
#include "modules/Animations.hpp"

using namespace glm;
using namespace std;

void Vulcano::processInteractions() {
    this->drawDeersBridge();
}

bool contains(const auto &range, const auto &val) {
    return std::find(std::begin(range), std::end(range), val) != std::end(range);
}

void Vulcano::drawDeersBridge() {
    const vec3 deerPosition         = vec3(7.0f, 0.0f, -3.0f);
    const float minDistanceRequired = 5.0f;
    const float playerDistance      = length(deerPosition - state.eyePosition);
    if (playerDistance > minDistanceRequired) return; // Check if player is near enough
    if (state.deersBridgeMoving) return;
    state.deersBridgeMoving = true;
    logs::info("Deer's bridge requested to be raised");
}

void Vulcano::updateDeersBridge(float deltaT) {
    if (!state.deersBridgeMoving) return;
    const str bridgesID[]    = {"castle-bridge/1", "castle-bridge/2"};
    const float duration     = 3.0f;
    const float maxRaise     = 7.5f;
    static float currentTime = 0.0f;
    static unordered_map<Instance *, mat4> initialWms; // World matrix of objects before animation
    static AnimTrack moveTrack;                        // Animation tracker for interpolating position in time

    // Cache the initial transformation matrices on first run
    if (initialWms.empty())
        for (int k = 0; k < SC.TechniqueInstanceCount; k++)
            for (int i = 0; i < SC.TI[k].InstanceCount; i++)
                if (SC.TI[k].I[i].id && contains(bridgesID, *SC.TI[k].I[i].id))
                    initialWms[&SC.TI[k].I[i]] = SC.TI[k].I[i].Wm;

    // Setup animation tracker if it has not been done already
    if (moveTrack.Frames.empty()) {
        moveTrack.nKeyFrames = 2; // Two points to interpolate
        moveTrack.Frames.push_back({
            0.0f,                         // Time
            vec3(0.0f, 0.0f, 0.0f),       // Translation
            quat(1.0f, 0.0f, 0.0f, 0.0f), // Rotation
            vec3(1.0f, 1.0f, 1.0f)        // Scaling
        });
        moveTrack.Frames.push_back({
            duration,                     // Time duration
            vec3(0.0f, maxRaise, 0.0f),   // Translation delta
            quat(1.0f, 0.0f, 0.0f, 0.0f), // No rotation
            vec3(1.0f, 1.0f, 1.0f)        // No scaling
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
    if (state.showColliders) SC.refreshColliderVisualizer();

    // Conclude animation
    if (currentTime >= duration) {
        currentTime             = 0.0f;
        state.deersBridgeMoving = false;
        state.deersBridgeRaised = true;
        logs::info("Deer's bridge raised");
    }
}
