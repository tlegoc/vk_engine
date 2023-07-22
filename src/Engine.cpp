//
// Created by theo on 22/07/2023.
//

#include <Engine.h>

#include <vulkan/vulkan.h>
#include <glm/gtx/transform.hpp>

#include <fstream>
#include <string>
#include <cstring>

void Engine::run() {
    bool quit = false;
    while (!glfwWindowShouldClose(m_window) && !quit) {
        glfwPollEvents();

        VK_CHECK(vkWaitForFences(m_device, 1, &m_render_fence, true, 1000000000))
        VK_CHECK(vkResetFences(m_device, 1, &m_render_fence))
        // We need to flush after vulkan has finished working.
        m_per_frame_deletion_queue.flush();
        draw();
    }
}

void Engine::draw() {
    // Used in debug
    float flash = glm::abs(glm::sin(m_frame_count / 120.f));

    // Will call present semaphore when done.
    uint32_t swapchain_image_index;
    VK_CHECK(vkAcquireNextImageKHR(m_device, m_swapchain, 1000000000, m_present_semaphore, nullptr,
                                   &swapchain_image_index))

    VK_CHECK(vkResetCommandBuffer(m_main_command_buffer, 0))

    // Buffer will be used once so we recreate it when we draw
    VkCommandBufferBeginInfo command_buffer_begin_info = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .pNext = nullptr,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
            .pInheritanceInfo = nullptr
    };

    VK_CHECK(vkBeginCommandBuffer(m_main_command_buffer, &command_buffer_begin_info))

    VkImageMemoryBarrier image_memory_barrier{
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .image = m_swapchain_images[swapchain_image_index],
            .subresourceRange = {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .baseMipLevel = 0,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1,
            }
    };

    vkCmdPipelineBarrier(
            m_main_command_buffer,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,  // srcStageMask
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, // dstStageMask
            0,
            0,
            nullptr,
            0,
            nullptr,
            1, // imageMemoryBarrierCount
            &image_memory_barrier // pImageMemoryBarriers
    );

    // Create as many color attachment as needed.
    // You can specify the layout, the image view (if use outside of a swapchain)
    // clear value and so on.
    const VkRenderingAttachmentInfo color_attachment_info{
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
            .imageView = m_swapchain_images_view[swapchain_image_index],
            .imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .clearValue = VkClearValue{
                    .color = {flash * .2f, flash, flash * .18f, 1}
            },
    };

    const VkRenderingAttachmentInfo depth_attachment_info{
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
            .imageView = m_depth_image_view,
            .imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .clearValue = VkClearValue{
                    .depthStencil = {1, 0}
            },
    };

    // Don't forget to include all color attachment.
    // You can select the desired output image in your shader by doing
    // layout(location = COLOR_ATTACHMENT INDEX) vecX variable_name;
    const VkRenderingInfo render_info{
            .sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR,
            .renderArea = {0, 0, m_window_extent},
            .layerCount = 1,
            .colorAttachmentCount = 1,
            .pColorAttachments = &color_attachment_info,
            .pDepthAttachment = &depth_attachment_info
    };

    // We make use of dynamic_rendering. This allows us to forget about renderpasses and
    // framebuffers completely. We can specify render attachments on the struct
    // above
    vkCmdBeginRendering(m_main_command_buffer, &render_info);

    cmd_render_commands();

    vkCmdEndRendering(m_main_command_buffer);

    // We need to convert image from render to present format
    image_memory_barrier = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            .oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            .image = m_swapchain_images[swapchain_image_index],
            .subresourceRange = {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .baseMipLevel = 0,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1,
            }
    };

    vkCmdPipelineBarrier(
            m_main_command_buffer,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,  // srcStageMask
            VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, // dstStageMask
            0,
            0,
            nullptr,
            0,
            nullptr,
            1, // imageMemoryBarrierCount
            &image_memory_barrier // pImageMemoryBarriers
    );

    VK_CHECK(vkEndCommandBuffer(m_main_command_buffer));

    VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    // Submit info (our draw calls). We want to wait for present semaphore, and will signal render semaphore when done.
    VkSubmitInfo submit = {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .pNext = nullptr,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &m_present_semaphore,
            .pWaitDstStageMask = &wait_stage,
            .commandBufferCount = 1,
            .pCommandBuffers = &m_main_command_buffer,
            .signalSemaphoreCount = 1,
            .pSignalSemaphores = &m_render_semaphore
    };


    // Render fence blocked
    VK_CHECK(vkQueueSubmit(m_graphics_queue, 1, &submit, m_render_fence))

    // Present info, we will wait for render semaphore.
    VkPresentInfoKHR present_info = {
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .pNext = nullptr,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &m_render_semaphore,
            .swapchainCount = 1,
            .pSwapchains = &m_swapchain,
            .pImageIndices = &swapchain_image_index,
    };

    VK_CHECK(vkQueuePresentKHR(m_graphics_queue, &present_info))

    m_frame_count++;

    // Info
    // glfwSetWindowTitle(m_window, ("VulkanEngine - Frame count: " + std::to_string(m_frame_count)).c_str());
}

void Engine::cmd_render_commands() {
    vkCmdBindPipeline(m_main_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_debug_mesh_pipeline);

    // Will be moved somewhere else in the end
    {
        //make a model view matrix for rendering the object
        //camera position
        glm::vec3 cam_pos = {0.f, 0.f, -2.f};

        glm::mat4 view = glm::translate(glm::mat4(1.f), cam_pos);
        //camera projection
        glm::mat4 projection = glm::perspective(glm::radians(70.f), 1700.f / 900.f, 0.1f, 200.0f);
        projection[1][1] *= -1;
        //model rotation
        glm::mat4 model = glm::rotate(glm::mat4{1.0f}, glm::radians(m_frame_count * 0.4f), glm::vec3(0, 1, 0));

        //calculate final mesh matrix
        glm::mat4 mesh_matrix = projection * view * model;

        MeshPushConstants constants{};
        constants.render_matrix = mesh_matrix;

        //upload the matrix to the GPU via push constants
        vkCmdPushConstants(m_main_command_buffer, m_debug_mesh_pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0,
                           sizeof(MeshPushConstants), &constants);
    }

    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(m_main_command_buffer, 0, 1, &m_debug_monkey_mesh.m_vertex_buffer.m_buffer, &offset);

    vkCmdDraw(m_main_command_buffer, m_debug_monkey_mesh.m_vertices.size(), 1, 0, 0);
}

bool Engine::load_shader_module(const char *file_path, VkShaderModule *out_shader_module) const {
    std::ifstream file(file_path, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        return false;
    }

    size_t file_size = (size_t) file.tellg();

    std::vector<uint32_t> buffer(file_size / sizeof(uint32_t));

    file.seekg(0);
    file.read((char *) buffer.data(), file_size);

    file.close();

    VkShaderModuleCreateInfo create_info = {
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .pNext = nullptr,
            // Size in bytes
            .codeSize = buffer.size() * sizeof(uint32_t),
            .pCode = buffer.data(),
    };
    VkShaderModule shader_module;
    if (vkCreateShaderModule(m_device, &create_info, nullptr, &shader_module) != VK_SUCCESS) {
        return false;
    }
    *out_shader_module = shader_module;
    return true;
}

void Engine::upload_mesh(Mesh &mesh) {
    VkBufferCreateInfo buffer_info = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = mesh.m_vertices.size() * sizeof(Vertex),
            .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT
    };

    VmaAllocationCreateInfo vma_alloc_info = {};
    vma_alloc_info.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

    VK_CHECK(vmaCreateBuffer(m_allocator, &buffer_info, &vma_alloc_info,
                             &mesh.m_vertex_buffer.m_buffer,
                             &mesh.m_vertex_buffer.m_allocation,
                             nullptr));

    m_main_deletion_queue.push_function([=, this]() {
        vmaDestroyBuffer(m_allocator, mesh.m_vertex_buffer.m_buffer, mesh.m_vertex_buffer.m_allocation);
    });

    // Copy the data
    void *data;
    vmaMapMemory(m_allocator, mesh.m_vertex_buffer.m_allocation, &data);

    memcpy(data, mesh.m_vertices.data(), mesh.m_vertices.size() * sizeof(Vertex));

    vmaUnmapMemory(m_allocator, mesh.m_vertex_buffer.m_allocation);
}