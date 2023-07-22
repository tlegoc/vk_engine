//
// Created by theo on 21/07/2023.
//

#ifndef VK_ENGINE_MATERIAL_H
#define VK_ENGINE_MATERIAL_H

#include <Mesh.h>

#include <vulkan/vulkan.h>

struct Material {
    VkPipeline m_pipeline;
    VkPipelineLayout m_pipeline_layout;
};

#endif //VK_ENGINE_MATERIAL_H
