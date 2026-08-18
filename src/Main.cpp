#include "Vulcano.hpp"

void switchScene(Vulcano &app, str scene) {
    if (scene == "default") {
        app.world.scenePath = "assets/scenes/Scene.json";
        app.world.sceneID   = SCENE_DEFAULT;
    } else if (scene == "showcase") {
        app.world.scenePath = "assets/scenes/Showcase.json";
        app.world.sceneID   = SCENE_SHOWCASE;
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
