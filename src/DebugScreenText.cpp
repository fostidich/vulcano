#include "Utils.hpp"
#include "Vulcano.hpp"

using namespace glm;
using namespace fmt;

std::pair<float, float> getFPSPeak(float deltaT);

void Vulcano::updateDebugScreen(float deltaT) {
    std::ostringstream oss;
    if (player.debugScreen) {
        auto [fps, peak] = getFPSPeak(deltaT);
        oss << "FPS: " << fpsPeak(fps, peak) << "\n";
        oss << "Position: " << pos(player.eyePosition) << "\n";
        oss << "Rotation: " << rot(player.pitch, player.yaw) << "\n";
        oss << "Direction: " << compass(player.yaw) << "\n";
        oss << "Animations: " << world.currentlyAnimating << "\n";
        txt.printTopLeft(oss.str());

        oss.str("");
        oss.clear();
        oss << "[space] Jumping: " << active(player.jumping) << "\n";
        oss << "[shift] Sneaking: " << active(player.slowing) << "\n";
        oss << "[ctrl] Running: " << active(player.running) << "\n";
        oss << "[esc] Focus: " << active(player.cursorCaptured) << "\n";
        oss << "[Z] Zoom: " << active(player.zooming) << "\n";
        oss << "[F] Flight: " << active(player.flightMode) << "\n";
        oss << "[C] Collisions: " << active(player.collisions) << "\n";
        oss << "[H] Hitboxes: " << active(player.showColliders) << "\n";
        oss << "[I] Info text: " << "on" << "\n";
        txt.printBottomLeft(oss.str());
    } else {
        oss << "[I]" << "\n";
        txt.printTopLeft(oss.str());
    }
}

void Vulcano::printShowcaseInfo() {
    txt.printBottomCenter("Showcase world");
}

std::pair<float, float> getFPSPeak(float deltaT) {
    static float elapsedT    = 0.0f;
    static u32 countedFrames = 0;
    static float fps         = 0.0f;
    countedFrames++;
    elapsedT += deltaT;

    static float peak            = 0.0f;
    static float elapsedFromPeak = 0.0f;
    elapsedFromPeak += deltaT;

    // Update peak in range of 3 seconds
    if (deltaT > peak) {
        peak            = deltaT;
        elapsedFromPeak = 0.0f;
    } else if (elapsedFromPeak > 3.0f) {
        peak = 0.0f;
    }

    // If another full second has passed
    if (elapsedT > 1.0f) {
        fps           = (float)countedFrames / (float)elapsedT;
        elapsedT      = 0.0f;
        countedFrames = 0;
    }

    return {fps, peak};
}
