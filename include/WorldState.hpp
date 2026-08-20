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
    const glm::vec3 deerPosition                      = glm::vec3(7.0f, 0.0f, -3.0f);
    bool deerBridgeRaised                             = false;
    bool deerBridgeMoving                             = false;
    float deerDistance; // Keep track of how far the deer is

    // Lifting sword from stone
    static constexpr std::string_view swordStoneID = "sword/1";
    const glm::vec3 swordStonePosition             = glm::vec3(-15.15f, 0.8f, 50.9f);
    float swordStoneDistance;

    // Candle in the skeleton's tent
    static constexpr std::string_view tentCandleID = "candle/1";
    const glm::vec3 tentCandlePosition             = glm::vec3(8.0f, 1.7f, 46.0f);
    const glm::vec3 tentCandleEntrancePos          = glm::vec3(7.95f, 1.51f, 40.05f);
    float tentCandleDistance;

    // Crystal circle with bones in the middle
    static constexpr std::string_view crystalsID[] = {"crystal/left", "crystal/right", "crystal/front", "crystal/back"};
    const glm::vec3 crystalBonesPosition           = glm::vec3(0.0f, 0.0f, -30.0f);
    float crystalBonesDistance;

    // Optimization: don't refresh colliders if nothing is moving.
    inline void animating() { currentlyAnimating++; }

    unsigned int currentlyAnimating = 0;
};
