//
// Created by theo on 10/07/2023.
//

#ifndef VK_ENGINE_PIPELINEBUILDER_H
#define VK_ENGINE_PIPELINEBUILDER_H

#include <Initializers.h>

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
    std::vector<VkPipelineColorBlendAttachmentState> m_color_blend_attachment;
    VkPipelineMultisampleStateCreateInfo m_multisampling;
    VkPipelineLayout m_pipeline_layout;
    VkPipelineDepthStencilStateCreateInfo m_depth_stencil;
    VkFormat m_depth_stencil_format;

    void setup_default(VkExtent2D window_extent);
    VkPipeline build_pipeline(VkDevice device);
};


#endif //VK_ENGINE_PIPELINEBUILDER_H
