//
// Created by theo on 10/07/2023.
//

#ifndef VK_ENGINE_INITIALIZERS_H
#define VK_ENGINE_INITIALIZERS_H

#include <vulkan/vulkan.h>

namespace Initializers {
    VkPipelineShaderStageCreateInfo
    pipeline_shader_stage_create_info(VkShaderStageFlagBits stage, VkShaderModule shader_module);

    VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info();

    VkPipelineInputAssemblyStateCreateInfo input_assembly_create_info(VkPrimitiveTopology topology);
}


#endif //VK_ENGINE_INITIALIZERS_H
