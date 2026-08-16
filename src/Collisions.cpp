#include "Vulcano.hpp"

using namespace glm;

void Vulcano::processCollisions(vec3 &currentPos, const vec3 &displacement) {
    const float distance = length(displacement);
    if (distance < 0.0001f) return;

    const float maxStep = 0.2f;                                            // Max distance computable at a time
    const int steps     = std::max(1, (int)std::ceil(distance / maxStep)); // Number of iterations needed to cover all distance
    const vec3 dispStep = displacement / (float)steps;                     // Horizontal contribution of step displacement
    for (int s = 0; s < steps; ++s) {
        const vec3 target   = currentPos + dispStep;  // Final position to test
        const float topY    = currentPos.y + maxStep; // Max Y reachable in the frame
        const float bottomY = currentPos.y - maxStep; // Min Y reachable in the frame

        // If bottom surface is near, snap to it
        if (!state.flightMode &&
            checkCollision(vec3(target.x, bottomY, target.z)) &&
            !checkCollision(vec3(target.x, currentPos.y - 0.0001f, target.z))) {
            const float snapY = binaryVerticalCollisionSearch(bottomY, currentPos.y, target);
            currentPos        = vec3(target.x, snapY, target.z);
        }
        // If going straight, go straight
        else if (!checkCollision(vec3(target.x, currentPos.y, target.z))) {
            currentPos = vec3(target.x, currentPos.y, target.z);
        }
        // If going up a slope, go up
        else if (!state.flightMode &&
                 !checkCollision(vec3(target.x, topY, target.z))) {
            const float snapY = binaryVerticalCollisionSearch(currentPos.y, topY, target);
            currentPos        = vec3(target.x, snapY, target.z);
        }
        // If colliding with something, slide on it
        else {
            const vec3 dispX = vec3(dispStep.x, 0.0f, 0.0f);
            const vec3 dispZ = vec3(0.0f, 0.0f, dispStep.z);
            if (std::abs(dispX.x) > 0.0001f && !checkCollision(currentPos + dispX)) currentPos += dispX;
            if (std::abs(dispZ.z) > 0.0001f && !checkCollision(currentPos + dispZ)) currentPos += dispZ;
        }

        // Compute gravity pull down
        if (dispStep.y == 0.0f) continue; // Stop here in flight mode
        const vec3 targetV = currentPos + vec3(0.0f, dispStep.y, 0.0f);
        if (!checkCollision(targetV)) currentPos = targetV;
    }
}

float Vulcano::binaryVerticalCollisionSearch(float low, float high, vec3 pos) {
    for (int i = 0; i < 5; ++i) {
        const float mid = (low + high) * 0.5f;
        if (checkCollision(vec3(pos.x, mid, pos.z)))
            low = mid;
        else
            high = mid;
    }
    return high;
}

bool Vulcano::checkCollision(const vec3 &testPos) {
    // Update player collider's world position
    this->playerCollider.setWorldMatrix(translate(mat4(1.0f), testPos));

    // Test against all colliders created by the scene
    for (Collider *objCollider : this->SC.GlobalColliders)
        if (objCollider && this->playerCollider.collidesWith(*objCollider))
            return true; // Collision detected
    return false;
};

void Vulcano::toggleColliders(u32 currentImage) {
    if (this->state.showColliders)
        this->SC.updateColliderVisualizer(currentImage, this->ViewPrj);
    else
        // Feed zero matrix to clip/discard all collider vertices
        this->SC.updateColliderVisualizer(currentImage, glm::mat4(0.0f));
}
