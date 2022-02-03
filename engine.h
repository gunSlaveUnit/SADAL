//
// Created by gunSlaveUnit on 2/3/22.
//

#pragma once

#ifndef SHATTEREDSOULS_ENGINE_H
#define SHATTEREDSOULS_ENGINE_H

#include <iostream>
#include <cstdint>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class Engine {
public:
    void run();
private:
    void initWindow();
    void initVulkan();
    void createInstance();
    void mainLoop();
    void cleanup();

    const uint16_t WINDOW_WIDTH = 800;
    const uint16_t WINDOW_HEIGHT = 600;
    const char* WINDOW_TITLE = "Shattered Souls";

    GLFWwindow* window;

    VkInstance instance;
};


#endif //SHATTEREDSOULS_ENGINE_H
