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
#include <chrono>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "stb_image.h"

#include "vertex.h"
#include "transformation.h"


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
    void createRenderPass();
    void createDescriptorSetLayout();
    void createGraphicsPipeline();
    static std::vector<char> readFile(const std::string& filename);
    VkShaderModule createShaderModule(const std::vector<char>& code);
    VkViewport createViewport();
    VkRect2D createScissor();
    void createFrameBuffers();
    void createCommandPool();
    void createTexture();
    void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
                     VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
    void createVertexBuffer();
    void createIndexBuffer();
    void createUniformBuffers();
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                      VkMemoryPropertyFlags properties,
                      VkBuffer& buffer, VkDeviceMemory& bufferMemory);
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
    void createDescriptorPool();
    void createDescriptorSets();
    void createCommandBuffers();
    void createSemaphores();
    void createFences();
    void mainLoop();
    void drawFrame();
    void updateUniformBuffer(uint32_t currentImage);
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
    VkDescriptorSetLayout descriptorSetLayout;
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
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;
    std::vector<VkBuffer> uniformBuffers;
    std::vector<VkDeviceMemory> uniformBuffersMemory;
    VkDescriptorPool descriptorPool;
    std::vector<VkDescriptorSet> descriptorSets;

    const std::vector<Vertex> vertices = {
            {{-0.5f, -0.5f, 0.0f}, {0.5f, 0.0f, 0.0f}},
            {{0.5f, -0.5f, 0.0f}, {0.5f, 0.0f, 0.0f}},
            {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 0.5f}},
            {{-0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 0.5f}}
    };
    const std::vector<uint32_t> indices = {
            0, 1, 2, 2, 3, 0
    };

    const std::string SHADER_SOURCE_DIRECTORY = R"(../shaders/src/)";
    const std::string SHADER_COMPILED_DIRECTORY = R"(../shaders/cmpld/)";
    const std::string TEXTURES_SOURCE_DIRECTORY = R"(../textures/)";
};


#endif //SHATTEREDSOULS_ENGINE_H
