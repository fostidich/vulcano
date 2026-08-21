#include "Types.hpp"
#include "Vulcano.hpp"

using namespace glm;
using namespace std;

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
    this->updateSceneLights(gubo, deltaT);
    this->updateSceneShadows(gubo);
    this->DSglobal.map(currentImage, &gubo, 0);
    this->updateSceneInstances(gubo, currentImage);
}

float Vulcano::getDeltaT() {
    static auto startTime = chrono::high_resolution_clock::now();
    static float lastTime = 0.0f;

    const auto currentTime = chrono::high_resolution_clock::now();
    const float time       = chrono::duration<float, chrono::seconds::period>(currentTime - startTime).count();
    const float deltaT     = time - lastTime;
    lastTime               = time;
    return deltaT;
}

void Vulcano::computeViewProj() {
    // Projection matrix
    this->Prj = perspective(this->player.FOVy * (player.zooming ? 0.1f : 1.0f),
                            this->Ar,
                            this->player.nearPlane,
                            this->player.farPlane);
    this->Prj[1][1] *= -1;

    // View matrix
    this->View = lookAt(this->player.eyePosition,
                        this->player.lookAtPoint,
                        this->player.upVector);

    // View projection matrix
    this->ViewPrj = this->Prj * this->View;
}

void Vulcano::updateSceneInstances(const GlobalUniformBufferObject &gubo, int currentImage) {
    UniformBufferObject ubo;
    for (int t = 0; t < this->SC.TechniqueInstanceCount; t++) {
        for (int instanceId = 0; instanceId < this->SC.TI[t].InstanceCount; instanceId++) {
            ubo.mMat   = this->SC.TI[t].I[instanceId].Wm;
            ubo.mvpMat = this->ViewPrj * ubo.mMat;

            // Pass 0
            this->SC.TI[t].I[instanceId].DS[0][0]->map(currentImage, (void *)&gubo, 0); // Global lighting
            this->SC.TI[t].I[instanceId].DS[0][1]->map(currentImage, &ubo, 0);          // Camera MVP matrix
                                                                                        //
            // Pass 1
            this->SC.TI[t].I[instanceId].DS[1][0]->map(currentImage, (void *)&gubo, 0); // Global lighting
            this->SC.TI[t].I[instanceId].DS[1][1]->map(currentImage, &ubo, 0);          // Camera MVP matrix
        }
    }
}
