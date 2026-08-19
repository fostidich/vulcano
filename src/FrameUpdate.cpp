#include "Vulcano.hpp"

void Vulcano::updateUniformBuffer(u32 currentImage) {
    const float deltaT = this->getDeltaT();
    this->cameraUpdate(deltaT);
    this->renderColliders(currentImage);
    this->processInteractions(deltaT);
    this->processTextOnScreen(deltaT);
    this->textMakerUpdate();
    this->computeViewProj();
    GlobalUniformBufferObject gubo;
    this->updateGlobalLight(gubo, deltaT, currentImage);
    this->updateSceneInstances(gubo, currentImage);
}

void Vulcano::textMakerUpdate() {
    this->txt.updateCommandBuffer();
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
    this->Prj = glm::perspective(this->player.FOVy * (player.zooming ? 0.1f : 1.0f),
                                 this->Ar,
                                 this->player.nearPlane,
                                 this->player.farPlane);
    this->Prj[1][1] *= -1;

    // View matrix
    this->View = glm::lookAt(this->player.eyePosition,
                             this->player.lookAtPoint,
                             this->player.upVector);

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
    gubo.lightDir         = glm::vec3(lightView * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f));
    gubo.lightColor       = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f) * 5.0f;
    gubo.eyePos           = glm::vec3(glm::inverse(this->View)[3]);

    world.candleDistance = glm::distance(this->player.eyePosition, world.entrancePos);
    const bool candleNear = world.candleDistance < world.candleAreaOn;
    gubo.candleLightPos = world.candlePos;
    gubo.candleLightColor = candleNear ? glm::vec4(1.0f, 0.6f, 0.2f, 1.0f) * 5.0f : glm::vec4(0.0f);
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
