#pragma once
#include "modules/Colliders.hpp"
#include <format>
#include <glm/glm.hpp>
#include <string>

struct CachedCollider {
    Collider *ptr;
    AABBextents ext;
};

inline void toggle(bool &b) { b = !b; }

inline bool contains(const auto &list, const auto &value) {
    return std::find(std::begin(list), std::end(list), value) != std::end(list);
}

namespace fmt {
inline std::string pos(glm::vec3 v) {
    return std::format("({:.2f}, {:.2f}, {:.2f})", v.x, v.y, v.z);
}

inline std::string rot(float p, float y) {
    float pitchDeg = std::fmod(glm::degrees(p) + 180.0f, 360.0f);
    float yawDeg   = std::fmod(glm::degrees(y) + 180.0f, 360.0f);
    if (pitchDeg < 0.0f) pitchDeg += 360.0f;
    if (yawDeg < 0.0f) yawDeg += 360.0f;
    pitchDeg -= 180.0f;
    yawDeg -= 180.0f;
    return std::format("({:.2f}, {:.2f})", pitchDeg, yawDeg);
}

inline std::string active(bool b) { return b ? "on" : "off"; }

inline std::string compass(float yaw) {
    float deg                      = std::fmod(glm::degrees(yaw), 360.0f);
    deg                            = deg < 0.0f ? deg + 360.0f : deg;
    const std::string directions[] = {"North", "North-East", "East", "South-East", "South", "South-West", "West", "North-West"};
    const unsigned int index       = (unsigned int)((deg + 22.5f) / 45.0f) % 8;
    return directions[index];
}

inline std::string fpsPeak(float fps, float peak) {
    return std::format("{:d} ({:d}ms)", (int)fps, (int)(peak * 1000));
}
} // namespace fmt
