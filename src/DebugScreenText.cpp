#include "Vulcano.hpp"

using namespace glm;

namespace {
string vec3str(vec3 v) { return std::format("({:.2f}, {:.2f}, {:.2f})", v.x, v.y, v.z); }

string ftoistr(float f) { return std::to_string(static_cast<int>(f)); }

string boolstr(bool b) { return b ? "on" : "off"; }

string jumpstr(bool j, float jt, float jd) {
    if (!j) return "-";
    return std::format("{:.2f}", jt + jd);
}

string radtodegstr(float f) {
    float deg = std::fmod(glm::degrees(f) + 180.0f, 360.0f);
    if (deg < 0.0f) deg += 360.0f;
    deg -= 180.0f;
    return std::to_string(static_cast<int>(std::round(deg)));
}

string compass(float yaw) {
    float deg                 = std::fmod(glm::degrees(yaw), 360.0f);
    deg                       = deg < 0.0f ? deg + 360.0f : deg;
    const string directions[] = {"North", "North-East", "East", "South-East", "South", "South-West", "West", "North-West"};
    const usize index         = static_cast<usize>((deg + 22.5f) / 45.0f) % 8;
    return directions[index];
}
} // namespace

float getFPS(float deltaT) {
    static float elapsedT    = 0.0f;
    static u32 countedFrames = 0;
    static float fps         = 0.0f;

    countedFrames++;
    elapsedT += deltaT;

    // If another full second has passed
    if (elapsedT > 1.0f) {
        fps           = (float)countedFrames / (float)elapsedT;
        elapsedT      = 0.0f;
        countedFrames = 0;
    }

    return fps;
}

void Vulcano::updateDebugScreen(float deltaT) {
    std::ostringstream oss;
    if (player.debugScreen) {
        oss << "FPS: " << ftoistr(getFPS(deltaT)) << "\n";
        oss << "Position: " << vec3str(player.eyePosition) << "\n";
        oss << "Pitch: " << radtodegstr(player.pitch) << "\n";
        oss << "Yaw: " << radtodegstr(player.yaw) << "\n";
        oss << "Direction: " << compass(player.yaw) << "\n";
        oss << "[space] Jumping: " << jumpstr(player.jumping, player.jumpTimer, player.jumpDuration) << "\n";
        oss << "[shift] Sneaking: " << boolstr(player.slowing) << "\n";
        oss << "[ctrl] Running: " << boolstr(player.running) << "\n";
        oss << "[esc] Focus: " << boolstr(player.cursorCaptured) << "\n";
        oss << "[Z] Zoom: " << boolstr(player.zooming) << "\n";
        oss << "[F] Flight: " << boolstr(player.flightMode) << "\n";
        oss << "[C] Collisions: " << boolstr(player.collisions) << "\n";
        oss << "[H] Hitboxes: " << boolstr(player.showColliders) << "\n";
        oss << "[I] Info text: " << "on" << "\n";
    } else {
        oss << "[I]" << "\n";
    }

    // Updates summary in top-left corner
    this->txt.print(-1.0f, -1.0f,
                    oss.str(), 1, "CO",
                    false, false, true,
                    TAL_LEFT, TRH_LEFT, TRV_TOP,
                    {1.0f, 1.0f, 1.0f, 1.0f},
                    {0.0f, 0.0f, 0.0f, 0.0f});
}
