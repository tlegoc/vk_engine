//
// Created by theo on 13/07/2023.
//

#ifndef VK_ENGINE_MESH_H
#define VK_ENGINE_MESH_H

#include <vulkan/vulkan.h>
#include <VulkanHelpers.h>
#include <vector>
#include <glm/glm.hpp>

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

struct MeshPushConstants {
    glm::vec4 data;
    glm::mat4 render_matrix;
};

struct Mesh {
    std::vector<Vertex> m_vertices;

    AllocatedBuffer m_vertex_buffer;

    bool load_from_obj(const char* filename);
};


#endif //VK_ENGINE_MESH_H
