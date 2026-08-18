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

    const float minDistanceForMessage   = 15.0f;
    const float minDistanceForAnimation = 5.0f;

    // Deer's bridge
    static constexpr std::string_view deerBridgesID[] = {"castle-bridge/1", "castle-bridge/2"};
    const glm::vec3 deerPosition                      = glm::vec3(7.0f, 0.0f, -3.0f);
    bool deerBridgeRaised                             = false;
    bool deerBridgeMoving                             = false;
    float deerDistance; // Keep track of how far the deer is
};
