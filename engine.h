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
#include <set>

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
    void createSurface();
    void pickPhysicalDevice();
    bool isPhysicalDeviceSuitable(const VkPhysicalDevice& device);
    bool checkDeviceExtensionSupport(const VkPhysicalDevice& device);
    void createLogicalDevice();
    struct QueueFamilyIndices {
        [[nodiscard]] bool isQueueFamilyAvailable() const {
            return graphicsFamily.has_value() && presentSurfaceFamily.has_value();
        }

        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentSurfaceFamily;
    };
    QueueFamilyIndices findQueueFamilies(const VkPhysicalDevice& physicalDevice);
    void createSwapChain();
    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };
    SwapChainSupportDetails queryDetailsSwapChainSupport(VkPhysicalDevice device);
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D chooseSwapExtent(VkSurfaceCapabilitiesKHR& capabilities);
    void createImageViews();
    void mainLoop();
    void cleanup();

    const uint16_t WINDOW_WIDTH = 800;
    const uint16_t WINDOW_HEIGHT = 600;
    const char* WINDOW_TITLE = "Shattered Souls";
    GLFWwindow* window;

    VkInstance instance;
    #ifdef NDEBUG
        const bool enableValidationLayers = false;
    #else
        const bool enableValidationLayers = true;
    #endif
    const std::vector<const char*> validationLayers = {
            "VK_LAYER_LUNARG_standard_validation"
    };
    VkSurfaceKHR surface;
    VkQueue graphicsQueue;
    VkQueue presentSurfaceQueue;
    const std::vector<const char*> deviceExtensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice logicalDevice;
    VkSwapchainKHR swapChain;
    std::vector<VkImage> swapChainImages;
    VkFormat swapChainFormat;
    VkExtent2D swapChainExtent;
    std::vector<VkImageView> swapChainImageViews;
};


#endif //SHATTEREDSOULS_ENGINE_H
