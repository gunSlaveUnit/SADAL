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
#include <fstream>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "vertex.h"


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
    void createRenderPass();
    void createImageViews();
    void createGraphicsPipeline();
    static std::vector<char> readFile(const std::string& filename);
    VkShaderModule createShaderModule(const std::vector<char>& code);
    VkViewport createViewport();
    VkRect2D createScissor();
    void createFrameBuffers();
    void createCommandPool();
    void createVertexBuffer();
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                      VkMemoryPropertyFlags properties,
                      VkBuffer& buffer, VkDeviceMemory& bufferMemory);
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
    void createCommandBuffers();
    void createSemaphores();
    void createFences();
    void mainLoop();
    void drawFrame();
    void recreateSwapChain();
    void cleanup();
    void cleanupSwapChain();

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
            "VK_LAYER_KHRONOS_validation"
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
    VkRenderPass renderPass;
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;
    std::vector<VkFramebuffer> swapChainFrameBuffers;
    VkCommandPool commandPool;
    std::vector<VkCommandBuffer> commandBuffers;
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> flightFences;
    std::vector<VkFence> flightImages;
    const size_t MAX_FRAMES_IN_FLIGHT = 2;
    size_t currentFrame = 0;
    bool isFramebufferResized = false;
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;

    const std::string SHADER_COMPILED_DIRECTORY = R"(../shaders/cmpld/)";
};


#endif //SHATTEREDSOULS_ENGINE_H
