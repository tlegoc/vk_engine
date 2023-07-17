//
// Created by theo on 13/07/2023.
//

#ifndef VK_ENGINE_MESH_H
#define VK_ENGINE_MESH_H

#include <Types.h>
#include <vector>
#include <glm/vec3.hpp>

struct VertexInputDescription {

    std::vector<VkVertexInputBindingDescription> bindings;
    std::vector<VkVertexInputAttributeDescription> attributes;

    VkPipelineVertexInputStateCreateFlags flags = 0;
};

struct Vertex {

    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 color;

    static VertexInputDescription get_vertex_description();
};

struct Mesh {
    std::vector<Vertex> m_vertices;

    Types::AllocatedBuffer m_vertex_buffer;
};


#endif //VK_ENGINE_MESH_H
