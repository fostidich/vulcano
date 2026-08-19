#include "Vulcano.hpp"

void Vulcano::updateSword(float deltaT) {
    static int index = -1;
    if (index < 0) {
        for (int i = 0; i < SC.TI[0].InstanceCount; i++) {
            if (SC.TI[0].I[i].id && *SC.TI[0].I[i].id == "sword/1") {
                index = i;
                break;
            }
        }
    }
    world.swordDistance = length(world.swordPosition - player.eyePosition);
    const bool swordNear = world.swordDistance < world.swordArea;
    const float duration     = 2.0f;
    static float currentTime = 0.0f;
    currentTime = currentTime + (swordNear ? 1.0f : -1.0f) * duration * deltaT;
    currentTime = glm::clamp(currentTime, 0.0f, 1.0f);

    if (index >= 0) {
        const glm::vec3 pos = world.swordPosition + glm::vec3(0.0f, world.swordLift * currentTime, 0.0f);
        SC.TI[0].I[index].Wm = glm::translate(glm::mat4(1.0f), pos) *
                                glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f)) *
                                glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)) *
                                    glm::scale(glm::mat4(1.0f), glm::vec3(1.6f));
    }

}


