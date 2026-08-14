#include "Vulcano.hpp"

int main() {
    auto app = std::make_unique<Vulcano>();

    try {
        app->run(false);
    } catch (const std::exception &e) {
        logs::error(e.what());
        return 1;
    }

    app.reset();
    return 0;
}
