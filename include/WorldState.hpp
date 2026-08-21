#pragma once
#include "Descriptors.hpp"
#include <glm/glm.hpp>
#include <string>
#include <unordered_map>

enum SceneID {
    SCENE_DEFAULT,
    SCENE_SHOWCASE,
    SCENE_TERRAIN,
};

struct WorldState {
    // Loaded scene
    std::string scenePath;
    SceneID sceneID;

    // Upon loading scene, all point lights will be extracted from it, based
    // on recognised point light model IDs (e.g. "candle/*").
    // To interact with lights, modify this point lights array or the point
    // light struct of the point light in question.
    // This map can only accept string views, meaning it enforces that the
    // string used as key must always be specifically IDs of instances declared
    // in this struct.
    std::unordered_map<std::string_view, PointLight> pointLights;

    // Shared distances for standardizing interaction behaviors
    const float ring3 = 40.0f;
    const float ring2 = 15.0f;
    const float ring1 = 5.0f;

    // Deer's bridge
    static constexpr std::string_view deerBridgesID[] = {"castle-bridge/1", "castle-bridge/2"};
    const glm::vec3 deerPosition                      = glm::vec3(682, 22.5, 1081);
    bool deerBridgeRaised                             = false;
    bool deerBridgeMoving                             = false;
    float deerDistance; // Keep track of how far the deer is

    // Lifting sword from stone
    static constexpr std::string_view swordStoneID = "sword/1";
    const glm::vec3 swordStonePosition             = glm::vec3(659.85, 23.3, 1134.9);
    float swordStoneDistance;

    // Candle in the skeleton's tent
    static constexpr std::string_view tentCandleID = "candle/1";
    const glm::vec3 tentCandleEntrancePos          = glm::vec3(683, 24.1, 1124);
    float tentCandleDistance;

    // Crystal circle with bones in the middle
    static constexpr std::string_view crystalsID[] = {"crystal/left", "crystal/right", "crystal/front", "crystal/back"};
    const glm::vec3 crystalBonesPosition           = glm::vec3(675, 22.5, 1054);
    float crystalBonesDistance;

    // Optimization: don't refresh colliders if nothing is moving.
    inline void animating() { currentlyAnimating++; }

    unsigned int currentlyAnimating = 0;
};
