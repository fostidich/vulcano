#include "Types.hpp"
#include "Vulcano.hpp"

void Vulcano::updateUniformBuffer(u32 currentImage) {
    const float deltaT             = this->getDeltaT();
    this->world.currentlyAnimating = 0;
    this->txt.removeAllText();
    this->processInteractions(deltaT);
    this->processTextOnScreen(deltaT);
    this->txt.updateCommandBuffer();
    this->renderColliders(currentImage);
    this->cameraUpdate(deltaT);
    this->computeViewProj();
    GlobalUniformBufferObject gubo;
    this->updateSceneLights(gubo, deltaT, currentImage);
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

void Vulcano::updateSceneLights(GlobalUniformBufferObject &gubo, float deltaT, int currentImage) {
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

    // Construct GUBO for drift light model
    gubo.lightDir   = glm::vec3(lightView * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f));
    gubo.lightColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f) * 5.0f;
    gubo.eyePos     = glm::vec3(glm::inverse(this->View)[3]);

    // Copy point lights with GUBO
    gubo.pointLightsCount = this->world.pointLights.size();
    usize i               = 0;
    for (const auto &[key, val] : this->world.pointLights)
        gubo.pointLights[i++] = val;

    // Transfer GUBO data to GPU
    this->DSglobal.map(currentImage, &gubo, 0);
}

void Vulcano::updateSceneInstances(const GlobalUniformBufferObject &gubo, int currentImage) {
    UniformBufferObject ubo;
    for (int t = 0; t < this->SC.TechniqueInstanceCount; t++) {
        for (int instanceId = 0; instanceId < this->SC.TI[t].InstanceCount; instanceId++) {
            ubo.mMat   = this->SC.TI[t].I[instanceId].Wm;
            ubo.mvpMat = this->ViewPrj * ubo.mMat;
            this->SC.TI[t].I[instanceId].DS[0][0]->map(currentImage, (void *)&gubo, 0); // Global lighting
            this->SC.TI[t].I[instanceId].DS[0][1]->map(currentImage, &ubo, 0);          // Camera MVP matrix
        }
    }
}
