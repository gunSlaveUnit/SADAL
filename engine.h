//
// Created by gunSlaveUnit on 2/3/22.
//

#pragma once

#ifndef SHATTEREDSOULS_ENGINE_H
#define SHATTEREDSOULS_ENGINE_H


class Engine {
public:
    void run();
private:
    void initVulkan();
    void mainLoop();
    void cleanup();
};


#endif //SHATTEREDSOULS_ENGINE_H
