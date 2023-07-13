//
// Created by theo on 10/07/2023.
//

#ifndef VK_ENGINE_PIPELINEBUILDER_H
#define VK_ENGINE_PIPELINEBUILDER_H

#include <vulkan/vulkan.h>

#include <vector>

class PipelineBuilder {

public:

    std::vector<VkPipelineShaderStageCreateInfo> m_shader_stages;
    VkPipelineVertexInputStateCreateInfo m_vertex_input_info;
    VkPipelineInputAssemblyStateCreateInfo m_input_assembly;
    VkViewport m_viewport;
    VkRect2D m_scissor;
    VkPipelineRasterizationStateCreateInfo m_rasterizer;
    VkPipelineColorBlendAttachmentState m_color_blend_attachment;
    VkPipelineMultisampleStateCreateInfo m_multisampling;
    VkPipelineLayout m_pipeline_layout;

    VkPipeline build_pipeline(VkDevice device);
};


#endif //VK_ENGINE_PIPELINEBUILDER_H
