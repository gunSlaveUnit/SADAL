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

    window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, nullptr, nullptr);
    if (!window)
        throw std::runtime_error("ERROR: GLFW can't create window");
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window,
[](GLFWwindow* w, int width, int height){
            auto app = reinterpret_cast<Engine*>(glfwGetWindowUserPointer(w));
            app->isFramebufferResized = true;
        });
}

void Engine::initVulkan() {
    createInstance();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
    createSwapChain();
    createImageViews();
    createRenderPass();
    createGraphicsPipeline();
    createFrameBuffers();
    createCommandPool();
    createVertexBuffer();
    createCommandBuffers();
    createSemaphores();
    createFences();
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

void Engine::createRenderPass() {
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = swapChainFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(logicalDevice, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
        throw std::runtime_error("ERROR: Vulkan failed to create render pass");
}

void Engine::createGraphicsPipeline() {
    const auto vertexShaderCode = readFile(SHADER_COMPILED_DIRECTORY + "vertex.spv");
    const auto fragmentShaderCode = readFile(SHADER_COMPILED_DIRECTORY + "fragment.spv");

    VkShaderModule vertexModule = createShaderModule(vertexShaderCode);
    VkShaderModule fragmentModule = createShaderModule(fragmentShaderCode);

    VkPipelineShaderStageCreateInfo vertexShaderStageCreateInfo{};
    vertexShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertexShaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertexShaderStageCreateInfo.module = vertexModule;
    vertexShaderStageCreateInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragmentShaderStageCreateInfo{};
    fragmentShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragmentShaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragmentShaderStageCreateInfo.module = fragmentModule;
    fragmentShaderStageCreateInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertexShaderStageCreateInfo, fragmentShaderStageCreateInfo};

    auto bindingVertexDescription = Vertex::getBindingDescription();
    auto attributeVertexDescriptions = Vertex::getAttributeDescriptions();

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &bindingVertexDescription;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeVertexDescriptions.size());
    vertexInputInfo.pVertexAttributeDescriptions = attributeVertexDescriptions.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{};
    inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport = createViewport();

    VkRect2D scissor = createScissor();

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizerCreateInfo{};
    rasterizerCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizerCreateInfo.depthClampEnable = VK_FALSE;
    rasterizerCreateInfo.rasterizerDiscardEnable = VK_FALSE;
    rasterizerCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizerCreateInfo.lineWidth = 1.0f;
    rasterizerCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizerCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizerCreateInfo.depthBiasEnable = VK_FALSE;
    rasterizerCreateInfo.depthBiasConstantFactor = 0.0f;
    rasterizerCreateInfo.depthBiasClamp = 0.0f;
    rasterizerCreateInfo.depthBiasSlopeFactor = 0.0f;

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f;
    multisampling.pSampleMask = nullptr;
    multisampling.alphaToCoverageEnable = VK_FALSE;
    multisampling.alphaToOneEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState colorBlendAttachmentInfo{};
    colorBlendAttachmentInfo.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT |
            VK_COLOR_COMPONENT_G_BIT |
            VK_COLOR_COMPONENT_B_BIT |
            VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachmentInfo.blendEnable = VK_FALSE;
    colorBlendAttachmentInfo.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachmentInfo.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachmentInfo.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachmentInfo.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachmentInfo.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachmentInfo.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachmentInfo;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pSetLayouts = nullptr;
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges = nullptr;

    if (vkCreatePipelineLayout(logicalDevice, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
        throw std::runtime_error("ERROR: Vulkan failed to create pipeline layout");

    VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo{};
    graphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    graphicsPipelineCreateInfo.stageCount = 2;
    graphicsPipelineCreateInfo.pStages = shaderStages;
    graphicsPipelineCreateInfo.pVertexInputState = &vertexInputInfo;
    graphicsPipelineCreateInfo.pInputAssemblyState = &inputAssemblyInfo;
    graphicsPipelineCreateInfo.pViewportState = &viewportState;
    graphicsPipelineCreateInfo.pRasterizationState = &rasterizerCreateInfo;
    graphicsPipelineCreateInfo.pMultisampleState = &multisampling;
    graphicsPipelineCreateInfo.pDepthStencilState = nullptr;
    graphicsPipelineCreateInfo.pColorBlendState = &colorBlending;
    graphicsPipelineCreateInfo.pDynamicState = nullptr;
    graphicsPipelineCreateInfo.layout = pipelineLayout;
    graphicsPipelineCreateInfo.renderPass = renderPass;
    graphicsPipelineCreateInfo.subpass = 0;
    graphicsPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
    graphicsPipelineCreateInfo.basePipelineIndex = -1;

    if (vkCreateGraphicsPipelines(logicalDevice, VK_NULL_HANDLE, 1, &graphicsPipelineCreateInfo, nullptr, &graphicsPipeline) != VK_SUCCESS)
        throw std::runtime_error("ERROR: Vulkan failed to create graphics pipeline");

    vkDestroyShaderModule(logicalDevice, vertexModule, nullptr);
    vkDestroyShaderModule(logicalDevice, fragmentModule, nullptr);
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

VkViewport Engine::createViewport() {
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float) swapChainExtent.width;
    viewport.height = (float) swapChainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    return viewport;
}

VkRect2D Engine::createScissor() {
    VkRect2D scissor;
    scissor.offset = {0, 0};
    scissor.extent = swapChainExtent;

    return scissor;
}

void Engine::createFrameBuffers() {
    size_t imagesCount = swapChainImageViews.size();
    swapChainFrameBuffers.resize(swapChainImageViews.size());

    for(size_t i = 0; i < imagesCount; ++i) {
        VkImageView attachments[] = {swapChainImageViews[i] };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = sizeof(attachments) / sizeof(attachments[0]);
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = swapChainExtent.width;
        framebufferInfo.height = swapChainExtent.height;
        framebufferInfo.layers = 1;

        if(vkCreateFramebuffer(logicalDevice, &framebufferInfo, nullptr, &swapChainFrameBuffers[i]) != VK_SUCCESS)
            throw std::runtime_error("ERROR: Vulkan failed to create frame buffer");
    }
}

void Engine::createCommandPool() {
    QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

    VkCommandPoolCreateInfo commandPoolInfo{};
    commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
    commandPoolInfo.flags = 0;

    if (vkCreateCommandPool(logicalDevice, &commandPoolInfo, nullptr, &commandPool) != VK_SUCCESS)
        throw std::runtime_error("ERROR: Vulkan failed to create command pool");
}

void Engine::createVertexBuffer() {
    const std::vector<Vertex> vertices = {
            {{0.0f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}},
            {{0.5f, 0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},
            {{-0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}}
    };

    VkBufferCreateInfo bufferCreateInfo;
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size = sizeof(vertices[0]) * vertices.size();
    bufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(logicalDevice, &bufferCreateInfo, nullptr, &vertexBuffer) != VK_SUCCESS)
        throw std::runtime_error("ERROR: Vulkan failed to create vertex buffer");

    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements(logicalDevice, vertexBuffer, &memoryRequirements);

    VkMemoryAllocateInfo allocateInfo{};
    allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocateInfo.allocationSize = memoryRequirements.size;
    allocateInfo.memoryTypeIndex = findMemoryType(
            memoryRequirements.memoryTypeBits,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
            );

    if (vkAllocateMemory(logicalDevice, &allocateInfo, nullptr, &vertexBufferMemory) != VK_SUCCESS)
        throw std::runtime_error("ERROR: Vulkan failed to allocate vertex buffer memory");

    vkBindBufferMemory(logicalDevice, vertexBuffer, vertexBufferMemory, 0);

    void* data;
    vkMapMemory(logicalDevice, vertexBufferMemory, 0, bufferCreateInfo.size, 0, &data);
    std::memcpy(data, vertices.data(), (size_t) bufferCreateInfo.size);
    vkUnmapMemory(logicalDevice, vertexBufferMemory);
}

uint32_t Engine::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &physicalDeviceMemoryProperties);

    for(uint32_t i = 0; i < physicalDeviceMemoryProperties.memoryTypeCount; ++i)
        if (typeFilter & (1 << i) && (physicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
            return i;

    throw std::runtime_error("ERROR: Failed to find suitable memory type");
}

void Engine::createCommandBuffers() {
    commandBuffers.resize(swapChainFrameBuffers.size());

    VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
    commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateInfo.commandPool = commandPool;
    commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferAllocateInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

    if (vkAllocateCommandBuffers(logicalDevice, &commandBufferAllocateInfo, commandBuffers.data()) != VK_SUCCESS)
        throw std::runtime_error("ERROR: Vulkan failed to allocate command buffers");

    for(size_t i = 0; i < commandBuffers.size(); ++i) {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = 0;
        beginInfo.pInheritanceInfo = nullptr;

        if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS)
            throw std::runtime_error("ERROR: Vulkan failed to begin recording command buffer!");

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = renderPass;
        renderPassInfo.framebuffer = swapChainFrameBuffers[i];
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = swapChainExtent;
        VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;

        vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
        vkCmdDraw(commandBuffers[i], 3, 1, 0, 0);

        vkCmdEndRenderPass(commandBuffers[i]);
        if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS)
            throw std::runtime_error("ERROR: Vulkan failed to record command buffer!");
    }
}

void Engine::createSemaphores() {
    imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        if (vkCreateSemaphore(logicalDevice, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(logicalDevice, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS)
            throw std::runtime_error("ERROR: Vulkan failed to create semaphores!");
    }
}

void Engine::createFences() {
    flightImages.resize(swapChainImages.size(), VK_NULL_HANDLE);
    flightFences.resize(MAX_FRAMES_IN_FLIGHT);

    VkFenceCreateInfo fenceCreateInfo{};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
        if(vkCreateFence(logicalDevice, &fenceCreateInfo, nullptr, &flightFences[i]) != VK_SUCCESS)
            throw std::runtime_error("ERROR: Vulkan failed to create a fence for a frame");
}

void Engine::mainLoop() {
    while(!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        drawFrame();
    }

    vkDeviceWaitIdle(logicalDevice);
}

void Engine::drawFrame() {
    vkWaitForFences(logicalDevice, 1, &flightFences[currentFrame], VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(logicalDevice, swapChain, UINT64_MAX,
                                            imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreateSwapChain();
        return;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        throw std::runtime_error("ERROR: Vulkan failed to acquire swap chain image");

    if (flightImages[imageIndex] != VK_NULL_HANDLE)
        vkWaitForFences(logicalDevice, 1, &flightImages[imageIndex], VK_TRUE, UINT64_MAX);
    flightImages[imageIndex] = flightImages[currentFrame];

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffers[imageIndex];

    VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    vkResetFences(logicalDevice, 1, &flightFences[currentFrame]);

    if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, flightFences[currentFrame]) != VK_SUCCESS)
        throw std::runtime_error("ERROR: Vulkan failed to submit draw command buffer!");

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {swapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr;

    result = vkQueuePresentKHR(presentSurfaceQueue, &presentInfo);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || isFramebufferResized) {
        isFramebufferResized = false;
        recreateSwapChain();
    } else if (result != VK_SUCCESS)
        throw std::runtime_error("ERROR: Vulkan failed to present swap chain image");

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Engine::recreateSwapChain() {
    int width = 0, height = 0;
    glfwGetFramebufferSize(window, &width, &height);
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(window, &width, &height);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(logicalDevice);

    cleanupSwapChain();

    createSwapChain();
    createImageViews();
    createRenderPass();
    createGraphicsPipeline();
    createFrameBuffers();
    createCommandBuffers();
}

void Engine::cleanup() {
    cleanupSwapChain();
    vkDestroyBuffer(logicalDevice, vertexBuffer, nullptr);
    vkFreeMemory(logicalDevice, vertexBufferMemory, nullptr);
    for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        vkDestroySemaphore(logicalDevice, renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(logicalDevice, imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(logicalDevice, flightFences[i], nullptr);
    }
    vkDestroyCommandPool(logicalDevice, commandPool, nullptr);
    vkDestroyDevice(logicalDevice, nullptr);
    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);
    glfwDestroyWindow(window);
    glfwTerminate();
}

void Engine::cleanupSwapChain() {
    for (auto framebuffer : swapChainFrameBuffers)
        vkDestroyFramebuffer(logicalDevice, framebuffer, nullptr);
    vkFreeCommandBuffers(logicalDevice, commandPool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
    vkDestroyPipeline(logicalDevice, graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(logicalDevice, pipelineLayout, nullptr);
    vkDestroyRenderPass(logicalDevice, renderPass, nullptr);
    for (const auto& imageView : swapChainImageViews)
        vkDestroyImageView(logicalDevice, imageView, nullptr);
    vkDestroySwapchainKHR(logicalDevice, swapChain, nullptr);
}
