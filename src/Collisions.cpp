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
        if (!player.flightMode && !player.jumping &&
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
        else if (!player.flightMode &&
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
        if (high - low < 0.01f) break; // Within 1cm tolerance
        const float mid = (low + high) * 0.5f;
        if (checkCollision(vec3(pos.x, mid, pos.z)))
            low = mid;
        else
            high = mid;
    }
    return high;
}

bool Vulcano::checkCollision(const vec3 &testPos) {
    // If dynamic objects are animating, refresh colliders cache
    if (this->world.currentlyAnimating > 0) this->cacheSceneColliders();

    // Player collider AABB world extents
    const vec3 pMin = testPos + player.colliderAABBmin;
    const vec3 pMax = testPos + player.colliderAABBmax;

    bool playerMatrixSet = false;
    for (const auto &item : this->cachedColliders) {
        // Broad-phase: instant float AABB overlap test
        const AABBextents &ext = item.ext;
        if (pMax.x < ext.xMin || pMin.x > ext.xMax ||
            pMax.y < ext.yMin || pMin.y > ext.yMax ||
            pMax.z < ext.zMin || pMin.z > ext.zMax) {
            // If player is outside the bounding box, all colliders tests are skipped
            continue;
        }

        // Narrow-phase: matrix update and precise check
        if (!playerMatrixSet) {
            this->playerCollider.setWorldMatrix(translate(mat4(1.0f), testPos));
            playerMatrixSet = true;
        }
        if (this->playerCollider.collidesWith(*item.ptr))
            return true;
    }
    return false;
}

void Vulcano::renderColliders(int currentImage) {
    if (this->player.showColliders) {
        this->SC.updateColliderVisualizer(currentImage, this->ViewPrj);
        if (this->world.currentlyAnimating > 0)
            this->SC.refreshColliderVisualizer();
    }
}

void Vulcano::toggleColliders() {
    // Executed once when pressing H
    if (this->player.showColliders)
        // If activated, collider guides are reset based on objects
        this->SC.refreshColliderVisualizer();
    else
        // If deactivated, all collider guides are filled with empty matrix (hidden)
        for (u32 img = 0; img < this->swapChainImages.size(); ++img)
            this->SC.updateColliderVisualizer(img, glm::mat4(0.0f));
}
