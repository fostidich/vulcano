#include "Vulcano.hpp"
#include "modules/Animations.hpp"
#include <glm/glm.hpp>

using namespace glm;
using namespace std;

void Vulcano::updateSword(float deltaT) {
    world.swordDistance = length(world.swordPosition - player.eyePosition);
    if (world.swordDistance > world.ring3) return;

    const float duration     = 1.0f;
    const float targetHeight = 1.0f;
    static float currentTime = 0.0f;                  // Animation progress: current time / duration
    static vector<pair<Instance *, mat4>> initialWms; // World matrix of objects before animation
    static AnimTrack moveTrack;                       // Animation tracker for interpolating position in time

    // Manage sword animation state for this frame
    if (world.swordDistance > world.ring2) {
        if (currentTime == 0.0f) return; // In ring 3 and sword fully down
        currentTime = 0.0f;              // Force sword to be fully down (reset) when too far
    } else if (world.swordDistance > world.ring1) {
        if (currentTime <= 0.0f) return; // In ring 2 and sword fully down
        currentTime -= deltaT;           // Lower the sword
    } else {
        if (currentTime >= duration) return; // In ring 1 and sword fully up
        currentTime += deltaT;               // Raise the sword
    }
    currentTime = std::clamp(currentTime, 0.0f, duration);

    world.animating();
    if (initialWms.empty()) {
        // Cache the initial transformation matrices on first run
        for (int k = 0; k < SC.TechniqueInstanceCount; k++)
            for (int i = 0; i < SC.TI[k].InstanceCount; i++)
                if (*SC.TI[k].I[i].id == world.swordID)
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
    const mat4 animationMat = moveTrack.Sample(currentTime, 0, -1, false);

    // Combine current animation transform with initial matrix of object
    for (auto &[inst, baseWm] : initialWms) {
        const mat4 newWm = animationMat * baseWm;
        inst->Wm         = newWm;                    // Update object world matrix
        if (inst->C) inst->C->setWorldMatrix(newWm); // Update physical hitbox as well
    }
}
