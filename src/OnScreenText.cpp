#include "Vulcano.hpp"

void Vulcano::updateFPS(float deltaT) {
    static float elapsedT    = 0.0f;
    static int countedFrames = 0;

    countedFrames++;
    elapsedT += deltaT;

    // If another full second has passed
    if (elapsedT > 1.0f) {
        const float fps = (float)countedFrames / (float)elapsedT;

        std::ostringstream oss;
        oss << "FPS: " << fps << "\n";

        // Updates the FPS in top-left corner
        this->txt.print(-1.0f, -1.0f,
                        oss.str(), 1, "CO",
                        false, false, true,
                        TAL_LEFT, TRH_LEFT, TRV_TOP,
                        {1.0f, 0.0f, 0.0f, 1.0f},
                        {0.8f, 0.8f, 0.0f, 1.0f});

        elapsedT      = 0.0f;
        countedFrames = 0;
    }

    this->txt.updateCommandBuffer();
}

void Vulcano::textMakerInit() {
    this->txt.init(this, this->windowWidth, this->windowHeight);
}
