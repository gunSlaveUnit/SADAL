//
// Created by gunSlaveUnit on 2/15/22.
//

#pragma once

#ifndef SHATTEREDSOULS_VERTEX_H
#define SHATTEREDSOULS_VERTEX_H

#include <array>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>


struct Vertex {
    static VkVertexInputBindingDescription getBindingDescription();
    static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions();

    glm::vec3 position;
    glm::vec3 color;
};


#endif //SHATTEREDSOULS_VERTEX_H
