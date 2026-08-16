#include "Vulcano.hpp"

using namespace glm;

void Vulcano::processCollisions(vec3 &currentPos, const vec3 &displacement) {
    // Early exit if collisions are disabled
    if (!state.collisions) {
        currentPos += displacement;
        return;
    }

    // Compute walking player displacement on a slope in steps
    const float totalDist = length(displacement);                             // Total distance to compute
    if (totalDist < 0.0001f) return;                                          // If player is not staying still
    const float maxStep   = 0.2f;                                             // Max distance computable at a time
    const int steps       = std::max(1, (int)std::ceil(totalDist / maxStep)); // Number of iterations needed to cover all distance
    const vec3 stepDisp   = displacement / (float)steps;                      // Distance to compute for each iteration
    const vec3 horizDisp  = vec3(stepDisp.x, 0.0f, stepDisp.z);               // New XZ-only coordinates
    const float horizDist = length(horizDisp);                                // Horizontal distanced walked (hypotenuse)
    for (int s = 0; s < steps; ++s) {                                         // Each step modifies only `currentPos`
        const float topY       = currentPos.y + maxStep;                      // Max Y reachable in the frame
        const float bottomY    = currentPos.y - maxStep;                      // Max Y reachable in the frame
        const vec3 targetHoriz = currentPos + horizDisp;                      // Horizontal-only displacement

        // Slope snapping only when walking on ground (not in flight mode).
        // No collision at new top Y and collision at new bottom Y means we are
        // on a slope (no wall, no ravine).
        // Since max step in all direction is coincides to max step in Y
        // direction, this computation is permitted only on slopes with
        // steepness in (-45, 45) degrees range.
        if (!state.flightMode &&
            !checkCollision(vec3(targetHoriz.x, topY, targetHoriz.z)) &&
            checkCollision(vec3(targetHoriz.x, bottomY, targetHoriz.z))) {

            // Converge to a more precise Y displacement
            const float snapY = binaryVerticalCollisionSearch(bottomY, topY, targetHoriz);
            currentPos        = vec3(targetHoriz.x, snapY, targetHoriz.z);
        }
        // Flat ground, flight mode, or airborne horizontal movement
        else if (!checkCollision(targetHoriz)) {
            currentPos = targetHoriz;
            if (stepDisp.y == 0.0f) continue; // Finish here for flight mode
            const vec3 targetVert = currentPos + vec3(0.0f, stepDisp.y, 0.0f);
            if (!checkCollision(targetVert)) currentPos = targetVert; // Make gravity do its things
        }
        // Wall hit
        else {}
    }
}

float Vulcano::binaryVerticalCollisionSearch(float low, float high, vec3 pos) {
    for (int i = 0; i < 6; ++i) {
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
    for (Collider *objCollider : this->SC.GlobalColliders) {
        if (objCollider && this->playerCollider.collidesWith(*objCollider)) {
            return true; // Collision detected
        }
    }
    return false;
};

void Vulcano::toggleColliders(u32 currentImage) {
    if (this->state.showColliders)
        this->SC.updateColliderVisualizer(currentImage, this->ViewPrj);
    else
        // Feed zero matrix to clip/discard all collider vertices
        this->SC.updateColliderVisualizer(currentImage, glm::mat4(0.0f));
}
