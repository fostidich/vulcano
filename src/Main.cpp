#include "Types.hpp"
#include "Vulcano.hpp"

void switchScene(Vulcano &app, str scene) {
    if (scene == "default") {
        app.world.scenePath = "assets/scenes/Scene.json";
        app.world.sceneID   = SCENE_DEFAULT;
        app.terrain.load("assets/models/Terrain.obj");
        const float spawnX = -600.0f, spawnZ = 600.0f;
        const float spawnY     = app.terrain.getHeight(spawnX, spawnZ);
        app.player.eyePosition = glm::vec3(spawnX, spawnY + 2.0f, spawnZ);
        app.player.lookAtPoint = glm::vec3(spawnX, spawnY + 2.0f, spawnZ + 1.0f);
    } else if (scene == "showcase") {
        app.world.scenePath = "assets/scenes/Showcase.json";
        app.world.sceneID   = SCENE_SHOWCASE;
    } else if (scene == "terrain") {
        app.world.scenePath = "assets/scenes/Terrain.json";
        app.world.sceneID   = SCENE_TERRAIN;
        app.terrain.load("assets/models/Terrain.obj");
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
