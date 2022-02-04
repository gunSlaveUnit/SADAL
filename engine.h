//
// Created by gunSlaveUnit on 2/3/22.
//

#pragma once

#ifndef SHATTEREDSOULS_ENGINE_H
#define SHATTEREDSOULS_ENGINE_H

#include <iostream>
#include <cstdint>
#include <vector>
#include <cstring>
#include <optional>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class Engine {
public:
    void run();
private:
    void initWindow();
    void initVulkan();
    void createInstance();
    bool checkValidationLayerSupport();
    void pickPhysicalDevice();
    bool isPhysicalDeviceSuitable(const VkPhysicalDevice& physicalDevice);
    void createLogicalDevice();
    struct QueueFamilyIndices {
        [[nodiscard]] bool isQueueFamilyAvailable() const {
            return graphicsFamily.has_value();
        }

        std::optional<uint32_t> graphicsFamily;
    };
    QueueFamilyIndices findQueueFamilies(const VkPhysicalDevice& physicalDevice);
    void mainLoop();
    void cleanup();

    const uint16_t WINDOW_WIDTH = 800;
    const uint16_t WINDOW_HEIGHT = 600;
    const char* WINDOW_TITLE = "Shattered Souls";

    GLFWwindow* window;

    VkInstance instance;

    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice logicalDevice;

#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif
    const std::vector<const char*> validationLayers = {
            "VK_LAYER_LUNARG_standard_validation"
    };
};


#endif //SHATTEREDSOULS_ENGINE_H
