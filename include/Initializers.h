//
// Created by theo on 10/07/2023.
//

#ifndef VK_ENGINE_INITIALIZERS_H
#define VK_ENGINE_INITIALIZERS_H

#include <vulkan/vulkan.h>

namespace Initializers {
    VkPipelineShaderStageCreateInfo
    pipeline_shader_stage_create_info(VkShaderStageFlagBits stage, VkShaderModule shader_module, const char* entry_point = "main");

    VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info();

    VkPipelineInputAssemblyStateCreateInfo input_assembly_create_info(VkPrimitiveTopology topology);

    VkPipelineRasterizationStateCreateInfo rasterization_state_create_info(VkPolygonMode polygonMode);

    VkPipelineMultisampleStateCreateInfo multisampling_state_create_info();

    VkPipelineColorBlendAttachmentState color_blend_attachment_state();

    VkPipelineLayoutCreateInfo pipeline_layout_create_info();
}


#endif //VK_ENGINE_INITIALIZERS_H
