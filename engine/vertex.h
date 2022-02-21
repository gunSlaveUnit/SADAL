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
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>


struct Vertex {
    static VkVertexInputBindingDescription getBindingDescription();
    static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions();
    bool operator==(const Vertex& other) const;

    glm::vec3 position;
    glm::vec3 color;
    glm::vec2 textureCoordinates;
};

namespace std {
    template<> struct hash<Vertex> {
        size_t operator()(Vertex const& vertex) const {
            return ((hash<glm::vec3>()(vertex.position) ^
                     (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
                   (hash<glm::vec2>()(vertex.textureCoordinates) << 1);
        }
    };
}


#endif //SHATTEREDSOULS_VERTEX_H
