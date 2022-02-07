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
        throw std::runtime_error("ERROR: GLFW can't create window");
}

void Engine::initVulkan() {
    createInstance();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
    createSwapChain();
    createImageViews();
    createGraphicsPipeline();
}

void Engine::createInstance() {
    if(enableValidationLayers && !checkValidationLayerSupport())
        throw std::runtime_error("ERROR: Vulkan validation layers requested, but not available");

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pEngineName = "No engine";
    appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 1);
    appInfo.pApplicationName = WINDOW_TITLE;
    appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
    appInfo.apiVersion = VK_API_VERSION_1_2;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensionsNames = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    createInfo.enabledExtensionCount = glfwExtensionCount;
    createInfo.ppEnabledExtensionNames = glfwExtensionsNames;

    if(enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    } else
        createInfo.enabledLayerCount = 0;

    if(vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
        throw std::runtime_error("ERROR: Vulkan can't create instance");
}

bool Engine::checkValidationLayerSupport() {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for(const char* layerName : validationLayers) {
        bool layerFound = false;

        for(const auto& availableLayer : availableLayers)
            if(!strcmp(layerName, availableLayer.layerName)) {
                layerFound = true;
                break;
            }

        if(!layerFound) return false;
    }

    return true;
}

void Engine::createSurface() {
    if(glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
        throw std::runtime_error("ERROR: GLFW failed to create a surface");
}

void Engine::pickPhysicalDevice() {
    uint32_t physicalDeviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr);

    if(!physicalDeviceCount)
        throw std::runtime_error("ERROR: Failed find a physical GPU device with Vulkan support");

    std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
    vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices.data());

    for(const auto& device : physicalDevices)
        if(isPhysicalDeviceSuitable(device)) {
            physicalDevice = device;
            break;
        }

    if(physicalDevice == VK_NULL_HANDLE)
        throw std::runtime_error("ERROR: Failed to find a suitable GPU");
}

bool Engine::isPhysicalDeviceSuitable(VkPhysicalDevice const &device) {
    QueueFamilyIndices indices = findQueueFamilies(device);

    bool areExtensionsSupported = checkDeviceExtensionSupport(device);
    bool isSwapChainAdequate = false;
    if (areExtensionsSupported) {
        SwapChainSupportDetails swapChainSupportDetails = queryDetailsSwapChainSupport(device);
        isSwapChainAdequate = !swapChainSupportDetails.formats.empty() && !swapChainSupportDetails.presentModes.empty();
    }

    return indices.isQueueFamilyAvailable() && areExtensionsSupported && isSwapChainAdequate;
}

bool Engine::checkDeviceExtensionSupport(VkPhysicalDevice const &device) {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    for (const auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

void Engine::createLogicalDevice() {
    QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {
            indices.graphicsFamily.value(),
            indices.presentSurfaceFamily.value()
    };

    float queuePriority = 1.0f;
    for(const auto& queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;

        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{};

    VkDeviceCreateInfo deviceCreateInfo{};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
    deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
    deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();

    if (enableValidationLayers) {
        deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        deviceCreateInfo.ppEnabledLayerNames = validationLayers.data();
    } else
        deviceCreateInfo.enabledLayerCount = 0;

    if(vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &logicalDevice) != VK_SUCCESS)
        throw std::runtime_error("ERROR: Failed to create logical device");

    vkGetDeviceQueue(logicalDevice, indices.graphicsFamily.value(), 0, &graphicsQueue);
    vkGetDeviceQueue(logicalDevice, indices.presentSurfaceFamily.value(), 0, &presentSurfaceQueue);
}

Engine::QueueFamilyIndices Engine::findQueueFamilies(VkPhysicalDevice const &device) {
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    int graphicsFamilyIndex = 0;
    for(const auto& queueFamily : queueFamilies) {
        if(queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            indices.graphicsFamily = graphicsFamilyIndex;

        VkBool32 isDevicePresentImagesSurface = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, graphicsFamilyIndex, surface, &isDevicePresentImagesSurface);

        if(isDevicePresentImagesSurface) indices.presentSurfaceFamily = graphicsFamilyIndex;

        if(indices.isQueueFamilyAvailable()) break;

        ++graphicsFamilyIndex;
    }

    return indices;
}

void Engine::createSwapChain() {
    SwapChainSupportDetails swapChainSupportDetails = queryDetailsSwapChainSupport(physicalDevice);

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupportDetails.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupportDetails.presentModes);
    VkExtent2D extent = chooseSwapExtent(swapChainSupportDetails.capabilities);

    /* +1, since at the bare minimum, we might have to wait for the driver
     * to complete internal operations before we can get another image to render. */
    const uint32_t additionalImagesNumber = 1;
    uint32_t imageCount = swapChainSupportDetails.capabilities.minImageCount + additionalImagesNumber;

    const uint32_t maxImageCountCapabilities = swapChainSupportDetails.capabilities.maxImageCount;
    if(maxImageCountCapabilities > 0 && imageCount > maxImageCountCapabilities)
        imageCount = maxImageCountCapabilities;

    VkSwapchainCreateInfoKHR createSwapChainInfo{};
    createSwapChainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createSwapChainInfo.surface = surface;
    createSwapChainInfo.minImageCount = imageCount;
    createSwapChainInfo.imageFormat = surfaceFormat.format;
    createSwapChainInfo.imageColorSpace = surfaceFormat.colorSpace;
    createSwapChainInfo.imageExtent = extent;
    createSwapChainInfo.imageArrayLayers = 1;
    createSwapChainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentSurfaceFamily.value()};

    if (indices.graphicsFamily != indices.presentSurfaceFamily) {
        createSwapChainInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createSwapChainInfo.queueFamilyIndexCount = 2;
        createSwapChainInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createSwapChainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createSwapChainInfo.queueFamilyIndexCount = 0; // Optional
        createSwapChainInfo.pQueueFamilyIndices = nullptr; // Optional
    }

    createSwapChainInfo.preTransform = swapChainSupportDetails.capabilities.currentTransform;
    createSwapChainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createSwapChainInfo.presentMode = presentMode;
    createSwapChainInfo.clipped = VK_TRUE;
    createSwapChainInfo.oldSwapchain = VK_NULL_HANDLE;

    if(vkCreateSwapchainKHR(logicalDevice, &createSwapChainInfo, nullptr, &swapChain) != VK_SUCCESS)
        throw std::runtime_error("ERROR: Vulkan failed to create swap chain");

    uint32_t swapChainImagesCount = 0;
    vkGetSwapchainImagesKHR(logicalDevice, swapChain, &swapChainImagesCount, nullptr);
    swapChainImages.resize(swapChainImagesCount);
    vkGetSwapchainImagesKHR(logicalDevice, swapChain, &swapChainImagesCount, swapChainImages.data());

    swapChainFormat = surfaceFormat.format;
    swapChainExtent = extent;
}

Engine::SwapChainSupportDetails Engine::queryDetailsSwapChainSupport(VkPhysicalDevice device) {
    SwapChainSupportDetails swapChainDetails;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &swapChainDetails.capabilities);

    uint32_t formatsCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatsCount, nullptr);
    if(formatsCount) {
        swapChainDetails.formats.resize(formatsCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatsCount, swapChainDetails.formats.data());
    }

    uint32_t presentModsCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModsCount, nullptr);
    if(presentModsCount) {
        swapChainDetails.presentModes.resize(presentModsCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModsCount, swapChainDetails.presentModes.data());
    }

    return swapChainDetails;
}

VkSurfaceFormatKHR Engine::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats) {
    for(const auto& availableFormat : availableFormats)
        if(availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR)
            return availableFormat;
    return availableFormats[0];
}

VkPresentModeKHR Engine::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
    for(const auto& availablePresentMode : availablePresentModes)
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
            return availablePresentMode;
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D Engine::chooseSwapExtent(VkSurfaceCapabilitiesKHR& capabilities) {
    if(capabilities.currentExtent.width != UINT32_MAX)
        return capabilities.currentExtent;
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    VkExtent2D actualExtent {
        static_cast<uint32_t>(width),
        static_cast<uint32_t>(height),
    };

    actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
    actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

    return actualExtent;
}

void Engine::createImageViews() {
    swapChainImageViews.resize(swapChainImages.size());
    for(size_t i = 0; i < swapChainImages.size(); ++i) {
        VkImageViewCreateInfo imageViewCreateInfo{};
        imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCreateInfo.image = swapChainImages[i];
        imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewCreateInfo.format = swapChainFormat;

        imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
        imageViewCreateInfo.subresourceRange.levelCount = 1;
        imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        imageViewCreateInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(logicalDevice, &imageViewCreateInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS)
            throw std::runtime_error("ERROR: Vulkan failed to create image views");
    }
}

void Engine::createGraphicsPipeline() {
    const auto vertexShaderCode = readFile(SHADER_COMPILED_DIRECTORY + "vertex.spv");
    const auto fragmentShaderCode = readFile(SHADER_COMPILED_DIRECTORY + "fragment.spv");
}

std::vector<char> Engine::readFile(const std::string &filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if(!file.is_open())
        throw std::runtime_error("ERROR: Failed to open file: " + filename);

    long fileSize = static_cast<long>(file.tellg());
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    return buffer;
}

VkShaderModule Engine::createShaderModule(const std::vector<char>& code) {
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shaderModule;
    if(vkCreateShaderModule(logicalDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
        throw std::runtime_error("ERROR: Vulkan failed to create shader module");

    return shaderModule;
}

void Engine::mainLoop() {
    while(!glfwWindowShouldClose(window))
        glfwPollEvents();
}

void Engine::cleanup() {
    for (const auto& imageView : swapChainImageViews) {
        vkDestroyImageView(logicalDevice, imageView, nullptr);
    }
    vkDestroySwapchainKHR(logicalDevice, swapChain, nullptr);
    vkDestroyDevice(logicalDevice, nullptr);
    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);
    glfwDestroyWindow(window);
    glfwTerminate();
}
