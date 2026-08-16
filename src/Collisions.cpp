#include "Vulcano.hpp"

using namespace glm;

void Vulcano::processCollisions(vec3 &currentPos, const vec3 &displacement) {
    // Early exit if collisions are disabled
    if (!state.collisions) {
        currentPos += displacement;
        return;
    }

    // Horizontal movement and slope snapping
    const float maxStep    = 0.5f;                                                    // Max vertical displacement in a frame
    const float topY       = currentPos.y + maxStep;                                  // Max Y reachable in the frame
    const float bottomY    = currentPos.y - maxStep;                                  // Max Y reachable in the frame
    const vec3 targetHoriz = currentPos + vec3(displacement.x, 0.0f, displacement.z); // Horizontal-only displacement
    const vec2 horizDisp   = vec2(displacement.x, displacement.z);                    // New XZ-only coordinates
    const float horizDist  = length(horizDisp);                                       // Horizontal distanced walked (hypotenuse)

    // Skip slope computation if player is staying still
    if (horizDist < 0.0001f) goto end;

    // Check if there is a surface between top and bottom Y
    // (i.e. if I'm moving up a slope).
    if (!checkCollision(vec3(targetHoriz.x, topY, targetHoriz.z)) &&
        checkCollision(vec3(targetHoriz.x, bottomY, targetHoriz.z))) {

        // Converge to a more precise Y displacement (precision of 1/64 of a meter)
        float low  = bottomY;
        float high = topY;
        for (int i = 0; i < 6; ++i) {
            const float mid = (low + high) * 0.5f;
            if (checkCollision(vec3(targetHoriz.x, mid, targetHoriz.z)))
                low = mid;
            else
                high = mid;
        }

        // On a 45 degrees slope, vertical rise equals horizontal run (`tan(45) = 1 => rise/run = 1`).
        // This means that, on the steepest walkable slope, XZ displacement equals Y displacement (`horizDist` equals `heightDelta`).
        // Therefore, slope is walkable if:
        // - steepness is less than 45*1.05, allowing a 5% margin of error for floating-point inaccuracies.
        // - a +5cm provides threshold for tiny bumps (nearly flat ground).
        // If the elevation change exceeds this, the surface is too steep to walk on.
        float heightDelta = fabsf(high - currentPos.y); // Y displacement delta
        if (heightDelta <= horizDist * 1.05f + 0.05f) {
            const vec3 resolvedPos = vec3(targetHoriz.x, high, targetHoriz.z);
            if (!checkCollision(resolvedPos)) currentPos = resolvedPos;
        }
    }
    // Flat ground or airborne horizontal movement
    else if (!checkCollision(targetHoriz))
        currentPos = targetHoriz;

end: // Vertical movement
    if (displacement.y == 0.0f) return;
    const vec3 targetVert = currentPos + vec3(0.0f, displacement.y, 0.0f);
    if (checkCollision(targetVert)) return;
    currentPos = targetVert;
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
