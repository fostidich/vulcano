#include "Vulcano.hpp"

using namespace glm;

void Vulcano::processCollisions(vec3 &newPos, const vec3 &displacement) {
    // Early exit if collisions are disabled
    if (!state.collisions) {
        newPos += displacement;
        return;
    }

    if (!checkCollision(newPos + vec3(displacement.x, 0.0f, 0.0f))) { // Try moving in X
        newPos.x += displacement.x;
    }
    if (!checkCollision(newPos + vec3(0.0f, displacement.y, 0.0f))) { // Try moving in Y
        newPos.y += displacement.y;
    }
    if (!checkCollision(newPos + vec3(0.0f, 0.0f, displacement.z))) { // Try moving in Z
        newPos.z += displacement.z;
    }
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
