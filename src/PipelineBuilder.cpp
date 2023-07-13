//
// Created by theo on 10/07/2023.
//

#include "PipelineBuilder.h"
#include <iostream>

VkPipeline PipelineBuilder::build_pipeline(VkDevice device) {
    VkPipelineViewportStateCreateInfo viewport_state = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            .pNext = nullptr,
            .viewportCount = 1,
            .pViewports = &m_viewport,
            .scissorCount = 1,
            .pScissors = &m_scissor
    };

    //setup dummy color blending. We aren't using transparent objects yet
    //the blending is just "no blend", but we do write to the color attachment
    VkPipelineColorBlendStateCreateInfo color_blending = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            .pNext = nullptr,
            .logicOpEnable = VK_FALSE,
            .logicOp = VK_LOGIC_OP_COPY,
            .attachmentCount = 1,
            .pAttachments = &m_color_blend_attachment
    };

    //build the actual pipeline
    //we now use all of the info structs we have been writing into into this one to create the pipeline
    VkGraphicsPipelineCreateInfo pipeline_info = {
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .pNext = nullptr,

            .stageCount = static_cast<uint32_t>(m_shader_stages.size()),
            .pStages = m_shader_stages.data(),
            .pVertexInputState = &m_vertex_input_info,
            .pInputAssemblyState = &m_input_assembly,
            .pViewportState = &viewport_state,
            .pRasterizationState = &m_rasterizer,
            .pMultisampleState = &m_multisampling,
            .pColorBlendState = &color_blending,
            .layout = m_pipeline_layout,
            .subpass = 0,
            .basePipelineHandle = VK_NULL_HANDLE
    };

    //it's easy to error out on create graphics pipeline, so we handle it a bit better than the common VK_CHECK case
    VkPipeline new_pipeline;
    if (vkCreateGraphicsPipelines(
            device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &new_pipeline) != VK_SUCCESS) {
        std::cout << "failed to create pipeline" << std::endl;
        return VK_NULL_HANDLE; // failed to create graphics pipeline
    } else {
        return new_pipeline;
    }
}
