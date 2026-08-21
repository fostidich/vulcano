#include "Types.hpp"
#include "Vulcano.hpp"

void switchScene(Vulcano &app, str scene) {
    if (scene == "default") {
        app.world.sceneID        = SCENE_DEFAULT;
        app.world.scenePath      = "assets/scenes/Scene.json";
        const string terrainPath = "assets/models/Terrain.obj";
        err err                  = app.terrain.load(terrainPath);
        if (err) {
            logs::error("Unable to load terrain: ", terrainPath);
            exit(err);
        }
        const float spawnX = 666.0f, spawnZ = 1050.0f;
        const float spawnY     = app.terrain.getHeight(spawnX, spawnZ);
        app.player.eyePosition = glm::vec3(spawnX, spawnY + 2.0f, spawnZ);
        app.player.lookAtPoint = glm::vec3(spawnX, spawnY + 2.0f, spawnZ + 1.0f);
    } else if (scene == "showcase") {
        app.world.sceneID   = SCENE_SHOWCASE;
        app.world.scenePath = "assets/scenes/Showcase.json";
    } else if (scene == "terrain") {
        app.world.sceneID        = SCENE_TERRAIN;
        app.world.scenePath      = "assets/scenes/Terrain.json";
        const string terrainPath = "assets/models/Terrain.obj";
        err err                  = app.terrain.load(terrainPath);
        if (err) {
            logs::error("Unable to load terrain: ", terrainPath);
            exit(err);
        }
        const float spawnX = 0.0f, spawnZ = 0.0f;
        const float spawnY     = app.terrain.getHeight(spawnX, spawnZ);
        app.player.eyePosition = glm::vec3(spawnX, spawnY + 2.0f, spawnZ);
        app.player.lookAtPoint = glm::vec3(spawnX, spawnY + 2.0f, spawnZ + 1.0f);
    } else
        logs::error("Unknown scene identifier: ", scene);
}

int main(int argc, char *argv[]) {
    auto app = std::make_unique<Vulcano>();
    switchScene(*app, argc > 1 ? argv[1] : "default");

    try {
        app->run(false);
    } catch (const std::exception &e) {
        logs::error(e.what());
        return 1;
    }

    app.reset();
    return 0;
}
