#include "Vulcano.hpp"
#include "modules/Animations.hpp"

using namespace glm;
using namespace std;

void Vulcano::processInteractions() {
    this->drawDeersBridge();
}

namespace {
void toggle(bool &b) { b = !b; }

bool contains(const auto &range, const auto &val) {
    return std::find(std::begin(range), std::end(range), val) != std::end(range);
}

void printCenterBottom(TextMaker &txt, int id, const string &s) {
    txt.print(0.0f, 0.9f,
              s, id, "CO",
              false, false, false,
              TAL_CENTER, TRH_CENTER, TRV_BOTTOM,
              {1.0f, 1.0f, 1.0f, 1.0f},
              {0.0f, 0.0f, 0.0f, 1.0f});
}
} // namespace

void Vulcano::drawDeersBridge() {
    const vec3 deerPosition             = vec3(7.0f, 0.0f, -3.0f);
    const float minDistanceForAnimation = 5.0f;
    const float playerDistance          = length(deerPosition - state.eyePosition);
    if (playerDistance > minDistanceForAnimation) return;

    if (state.deersBridgeMoving) return;
    state.deersBridgeMoving = true;
    logs::info("Deer's bridge requested to be ", state.deersBridgeRaised ? "lowered" : "raised");
}

void Vulcano::updateDeersBridge(float deltaT) {
    if (!state.deersBridgeMoving) {
        const vec3 deerPosition             = vec3(7.0f, 0.0f, -3.0f);
        const float minDistanceForMessage   = 15.0f;
        const float minDistanceForAnimation = 5.0f;
        const float playerDistance          = length(deerPosition - state.eyePosition);

        // Check if player is near enough for displaying the message or for
        // requesting raising/lowering the bridge (if it's not moving already).
        if (playerDistance > minDistanceForMessage) {
            txt.removeText(2);
            return;
        }
        if (playerDistance > minDistanceForAnimation) {
            printCenterBottom(txt, 2, "Talk with the deer");
            return;
        }
        const string req = state.deersBridgeRaised ? "lower" : "raise";
        printCenterBottom(txt, 2, "Hi wanderer, press [E] to " + req + " the bridge");
        return;
    } else {
        const string status = state.deersBridgeRaised ? "down" : "up";
        printCenterBottom(txt, 2, "Bridge is going " + status);
    }

    const str bridgesID[]    = {"castle-bridge/1", "castle-bridge/2"};
    const float duration     = 3.0f;
    const float targetHeight = 7.5f * (state.deersBridgeRaised ? -1.0f : 1.0f);
    static float currentTime = 0.0f;
    static vector<pair<Instance *, mat4>> initialWms; // World matrix of objects before animation
    static AnimTrack moveTrack;                       // Animation tracker for interpolating position in time

    if (currentTime == 0.0f) {
        // Cache the initial transformation matrices on first run
        for (int k = 0; k < SC.TechniqueInstanceCount; k++)
            for (int i = 0; i < SC.TI[k].InstanceCount; i++)
                if (SC.TI[k].I[i].id && contains(bridgesID, *SC.TI[k].I[i].id))
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
    if (state.showColliders) SC.refreshColliderVisualizer();

    // Conclude animation
    if (currentTime >= duration) {
        currentTime = 0.0f;
        initialWms.clear();
        moveTrack.Frames.clear();
        state.deersBridgeMoving = false;
        toggle(state.deersBridgeRaised);
        logs::info("Deer's bridge ", state.deersBridgeRaised ? "raised" : "lowered");
    }
}
