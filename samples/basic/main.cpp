//
// Created by theo on 02/07/23.
//

#include <Engine.h>

int main() {
    Engine engine{};

    engine.init();

    engine.run();

    engine.cleanup();

    return 0;
}