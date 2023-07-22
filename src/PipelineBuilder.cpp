//
// Created by theo on 10/07/2023.
//

#include "PipelineBuilder.h"
#include <iostream>


void PipelineBuilder::setup_default(VkExtent2D window_extent) {

    //vertex input controls how to read vertices from vertex buffers.
    m_vertex_input_info = Initializers::vertex_input_state_create_info();

    m_input_assembly = Initializers::input_assembly_create_info(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

    m_viewport.x = 0.0f;
    m_viewport.y = 0.0f;
    m_viewport.width = (float) window_extent.width;
    m_viewport.height = (float) window_extent.height;
    m_viewport.minDepth = 0.0f;
    m_viewport.maxDepth = 1.0f;

    m_scissor.offset = {0, 0};
    m_scissor.extent = window_extent;

    m_rasterizer = Initializers::rasterization_state_create_info(VK_POLYGON_MODE_FILL);

    m_multisampling = Initializers::multisampling_state_create_info();

    //a single blend attachment with no blending and writing to RGBA
    m_color_blend_attachment.push_back(Initializers::color_blend_attachment_state());
}

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
            .attachmentCount = static_cast<uint32_t>(m_color_blend_attachment.size()),
            .pAttachments = m_color_blend_attachment.data()
    };

    // We ignore color attachment because it works without it.
    // We might change stencil tho
    const VkPipelineRenderingCreateInfoKHR pipeline_rendering_create_info {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR,
            .colorAttachmentCount = 0,
            .pColorAttachmentFormats = nullptr,
            .depthAttachmentFormat = m_depth_stencil_format,
            .stencilAttachmentFormat = VK_FORMAT_UNDEFINED,
    };

    VkGraphicsPipelineCreateInfo pipeline_info = {
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .pNext = &pipeline_rendering_create_info,
            .stageCount = static_cast<uint32_t>(m_shader_stages.size()),
            .pStages = m_shader_stages.data(),
            .pVertexInputState = &m_vertex_input_info,
            .pInputAssemblyState = &m_input_assembly,
            .pViewportState = &viewport_state,
            .pRasterizationState = &m_rasterizer,
            .pMultisampleState = &m_multisampling,
            .pDepthStencilState = &m_depth_stencil,
            .pColorBlendState = &color_blending,
            .layout = m_pipeline_layout,
            .subpass = 0,
            .basePipelineHandle = VK_NULL_HANDLE
    };

    VkPipeline new_pipeline;
    if (vkCreateGraphicsPipelines(
            device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &new_pipeline) != VK_SUCCESS) {
        std::cout << "failed to create pipeline" << std::endl;
        return VK_NULL_HANDLE;
    } else {
        return new_pipeline;
    }
}
