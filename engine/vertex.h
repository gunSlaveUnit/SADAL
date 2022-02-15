//
// Created by gunSlaveUnit on 2/15/22.
//

#pragma once

#ifndef SHATTEREDSOULS_VERTEX_H
#define SHATTEREDSOULS_VERTEX_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>


struct Vertex {
    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    glm::vec3 position;
    glm::vec3 color;
};


#endif //SHATTEREDSOULS_VERTEX_H
