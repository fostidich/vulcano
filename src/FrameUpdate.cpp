#include "Vulcano.hpp"

void Vulcano::updateUniformBuffer(u32 currentImage) {
    const float deltaT = this->getDeltaT();
    this->cameraUpdate(deltaT);
    this->toggleColliders(currentImage);
    this->updateDebugScreen(deltaT);
    this->computeViewProj();
    this->updateBridge(deltaT);
    GlobalUniformBufferObject gubo;
    this->updateGlobalLight(gubo, deltaT, currentImage);
    this->updateSceneInstances(gubo, currentImage);
}

float Vulcano::getDeltaT() {
    static auto startTime = std::chrono::high_resolution_clock::now();
    static float lastTime = 0.0f;

    const auto currentTime = std::chrono::high_resolution_clock::now();
    const float time       = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
    const float deltaT     = time - lastTime;
    lastTime               = time;
    return deltaT;
}

void Vulcano::computeViewProj() {
    // Projection matrix
    this->Prj = glm::perspective(this->state.FOVy * (state.zooming ? 0.1f : 1.0f),
                                 this->Ar,
                                 this->state.nearPlane,
                                 this->state.farPlane);
    this->Prj[1][1] *= -1;

    // View matrix
    this->View = glm::lookAt(this->state.eyePosition,
                             this->state.lookAtPoint,
                             this->state.upVector);

    // View projection matrix
    this->ViewPrj = this->Prj * this->View;
}

void Vulcano::updateGlobalLight(GlobalUniformBufferObject &gubo, float deltaT, int currentImage) {
    static float lightRotationAngle = 0.0f;

    // Define parameters for the uniform.
    // Keep track of rotation and increment rotation angle based on time.
    lightRotationAngle += -0.5f * deltaT;
    const glm::mat4 lightView =
        glm::rotate(glm::mat4(1),
                    glm::radians(lightRotationAngle),
                    glm::vec3(0.0f, 1.0f, 0.0f)) *
        glm::rotate(glm::mat4(1),
                    glm::radians(-45.0f),
                    glm::vec3(1.0f, 0.0f, 0.0f));

    // Construct GUBO and send it to GPU
    gubo.lightDir   = glm::vec3(lightView * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f));
    gubo.lightColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f) * 5.0f;
    gubo.eyePos     = glm::vec3(glm::inverse(this->View)[3]);
    gubo.candleLightPos = glm::vec3(8.0f, 1.7f, 46.0f);
    gubo.candleLightColor = glm::vec4(1.0f, 0.6f, 0.2f, 1.0f) * 5.0f;
    this->DSglobal.map(currentImage, &gubo, 0);
}

void Vulcano::updateSceneInstances(const GlobalUniformBufferObject &gubo, int currentImage) {
    UniformBufferObject ubo;
    for (int instanceId = 0; instanceId < this->SC.TI[0].InstanceCount; instanceId++) {
        ubo.mMat   = this->SC.TI[0].I[instanceId].Wm;
        ubo.mvpMat = this->ViewPrj * ubo.mMat;
        this->SC.TI[0].I[instanceId].DS[0][0]->map(currentImage, (void *)&gubo, 0); // Global lighting
        this->SC.TI[0].I[instanceId].DS[0][1]->map(currentImage, &ubo, 0);          // Camera MVP matrix
    }
}

void Vulcano::updateBridge(float deltaT) {
    static int bridgeIndex = -1;
    if (bridgeIndex < 0) {
        for (int j = 0; j < SC.TI[0].InstanceCount; j++) { // looping instances
            if (*SC.TI[0].I[j].id == "bridge1") {
                bridgeIndex = j;
                break;
            }
        }
    }

    const float raisedY   = -4.0f;
    const float loweredY  = -7.0f;
    const float time      = 2.0f;
    static bool triggered = false;
    static float t        = 0.0f;

    if (state.leverTriggered != triggered) {
        t         = 0.0f;
        triggered = state.leverTriggered;
    }
    t = t + deltaT; // update of how much time it took

    const float alpha = glm::clamp(t / time, 0.0f, 1.0f); // progress -- 0 < alpha < 1

    // if triggered interpolate from raised towards lowered, else from lowered to raised
    const float y = state.leverTriggered ? glm::mix(raisedY, loweredY, alpha) : glm::mix(loweredY, raisedY, alpha);

    const glm::mat4 Wm = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, y, 15.0f)) *
                         glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0, 1, 0)) *
                         glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1, 0, 0));
    if (bridgeIndex >= 0) {
        this->SC.TI[0].I[bridgeIndex].Wm = Wm; // overwrite transformation
    }
}
