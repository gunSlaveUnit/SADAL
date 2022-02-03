//
// Created by gunSlaveUnit on 2/3/22.
//

#include "engine.h"

void Engine::run() {
    initWindow();
    initVulkan();
    mainLoop();
    cleanup();
}

void Engine::initWindow() {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, nullptr, nullptr);
    if (!window)
        std::cerr<<"ERROR: GLFW can't create window"<<std::endl;
}

void Engine::initVulkan() {
    createInstance();
}

void Engine::createInstance() {
    VkApplicationInfo appInfo;
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pEngineName = "No engine";
    appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 1);
    appInfo.pApplicationName = WINDOW_TITLE;
    appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
    appInfo.apiVersion = VK_API_VERSION_1_2;
}

void Engine::mainLoop() {
    while(!glfwWindowShouldClose(window))
        glfwPollEvents();
}

void Engine::cleanup() {
    glfwDestroyWindow(window);
    glfwTerminate();
}
