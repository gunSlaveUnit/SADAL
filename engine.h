//
// Created by gunSlaveUnit on 2/3/22.
//

#pragma once

#ifndef SHATTEREDSOULS_ENGINE_H
#define SHATTEREDSOULS_ENGINE_H

#include <cstdint>

class Engine {
public:
    void run();
private:
    void initVulkan();
    void mainLoop();
    void cleanup();

    const uint16_t WINDOW_WIDTH = 800;
    const uint16_t WINDOW_HEIGHT = 600;
};


#endif //SHATTEREDSOULS_ENGINE_H
