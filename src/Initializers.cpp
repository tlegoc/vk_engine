//
// Created by theo on 10/07/2023.
//

#include "Initializers.h"

VkPipelineShaderStageCreateInfo
Initializers::pipeline_shader_stage_create_info(VkShaderStageFlagBits stage, VkShaderModule shader_module,
                                                const char* entry_point) {
    VkPipelineShaderStageCreateInfo info{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .pNext = nullptr,
            .stage = stage,
            .module = shader_module,
            .pName = entry_point
    };

    return info;
}

VkPipelineVertexInputStateCreateInfo Initializers::vertex_input_state_create_info() {
    VkPipelineVertexInputStateCreateInfo info = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            .pNext = nullptr,
            .vertexBindingDescriptionCount = 0,
            .vertexAttributeDescriptionCount = 0
    };

    return info;
}

VkPipelineInputAssemblyStateCreateInfo Initializers::input_assembly_create_info(VkPrimitiveTopology topology) {
    VkPipelineInputAssemblyStateCreateInfo info = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
            .pNext = nullptr,
            .topology = topology,
            .primitiveRestartEnable = VK_FALSE
    };

    return info;
}

VkPipelineRasterizationStateCreateInfo Initializers::rasterization_state_create_info(VkPolygonMode polygonMode) {
    VkPipelineRasterizationStateCreateInfo info = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
            .pNext = nullptr,

            .depthClampEnable = VK_FALSE,
            //discards all primitives before the rasterization stage if enabled which we don't want
            .rasterizerDiscardEnable = VK_FALSE,

            .polygonMode = polygonMode,
            .cullMode = VK_CULL_MODE_NONE,
            .frontFace = VK_FRONT_FACE_CLOCKWISE,
            //no depth bias
            .depthBiasEnable = VK_FALSE,
            .depthBiasConstantFactor = 0.0f,
            .depthBiasClamp = 0.0f,
            .depthBiasSlopeFactor = 0.0f,
            .lineWidth = 1.0f
    };

    return info;
}

VkPipelineMultisampleStateCreateInfo Initializers::multisampling_state_create_info() {
    VkPipelineMultisampleStateCreateInfo info = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
            .pNext = nullptr,
            .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
            .sampleShadingEnable = VK_FALSE,
            .minSampleShading = 1.0f,
            .pSampleMask = nullptr,
            .alphaToCoverageEnable = VK_FALSE,
            .alphaToOneEnable = VK_FALSE
    };
    return info;
}

VkPipelineColorBlendAttachmentState Initializers::color_blend_attachment_state() {
    VkPipelineColorBlendAttachmentState colorBlendAttachment = {
            .blendEnable = VK_FALSE,
            .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                              VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
    };
    return colorBlendAttachment;
}

VkPipelineLayoutCreateInfo Initializers::pipeline_layout_create_info() {
    VkPipelineLayoutCreateInfo info{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .pNext = nullptr,
            //empty defaults
            .flags = 0,
            .setLayoutCount = 0,
            .pSetLayouts = nullptr,
            .pushConstantRangeCount = 0,
            .pPushConstantRanges = nullptr
    };

    return info;
}

