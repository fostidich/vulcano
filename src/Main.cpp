#include "Vulcano.hpp"

void switchScene(Vulcano &app, str scene) {
    if (scene == "showcase")
        app.state.scenePath = "assets/scenes/Showcase.json";
    else
        logs::error("Unknown scene identifier: ", scene);
}

int main(int argc, char *argv[]) {
    auto app = std::make_unique<Vulcano>();
    if (argc > 1) switchScene(*app, argv[1]);

    try {
        app->run(false);
    } catch (const std::exception &e) {
        logs::error(e.what());
        return 1;
    }

    app.reset();
    return 0;
}
