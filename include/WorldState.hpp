#pragma once
#include <glm/glm.hpp>
#include <string>

enum SceneID {
    SCENE_DEFAULT,
    SCENE_SHOWCASE,
};

struct WorldState {
    // Loaded scene
    std::string scenePath;
    SceneID sceneID;

    // Shared distances for standardizing interaction behaviors
    const float ring3 = 40.0f;
    const float ring2 = 15.0f;
    const float ring1 = 5.0f;

    // Deer's bridge
    static constexpr std::string_view deerBridgesID[] = {"castle-bridge/1", "castle-bridge/2"};
    const glm::vec3 deerPosition                      = glm::vec3(7.0f, 0.0f, -3.0f);
    bool deerBridgeRaised                             = false;
    bool deerBridgeMoving                             = false;
    float deerDistance; // Keep track of how far the deer is

    // Lifting sword from stone
    static constexpr std::string_view swordID = "sword/1";
    const glm::vec3 swordPosition             = glm::vec3(-15.15f, 0.8f, 50.9f);
    float swordDistance;

    // Candle in the tent
    const glm::vec3 candlePos   = glm::vec3(8.0f, 1.7f, 46.0f);
    const glm::vec3 entrancePos = glm::vec3(7.95f, 1.51f, 40.05f);
    const float candleAreaOn    = 3.0f;
    float candleDistance;

    // Optimization: don't refresh colliders if nothing is moving.
    inline void animating() { currentlyAnimating++; }

    unsigned int currentlyAnimating = 0;
};
