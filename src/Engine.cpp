//
// Created by theo on 02/07/23.
//

#include <Engine.h>

#include <Initializers.h>

#include <iostream>
#include <glm/glm.hpp>

using namespace std;
#define VK_CHECK(x)                                                     \
        {                                                               \
        VkResult err = x;                                               \
        if (err)                                                        \
        {                                                               \
            std::cout << "Detected Vulkan error: " << err << std::endl; \
            abort();                                                    \
        }                                                               \
        }

void Engine::init() {

    m_frame_count = 0;

    init_glfw();
    init_vulkan();
    init_swapchain();
    init_commands();
    init_sync_structures();

    m_is_initialized = true;
}

void Engine::run() {
    bool quit = false;
    while (!glfwWindowShouldClose(m_window) && !quit) {
        glfwPollEvents();

        draw();
    }
}

void Engine::draw() {
    // Wait for render fence
    VK_CHECK(vkWaitForFences(m_device, 1, &m_render_fence, true, 1000000000))
    VK_CHECK(vkResetFences(m_device, 1, &m_render_fence))

    float flash = abs(sin(m_frame_count / 120.f));

    // Will call present semaphore.
    uint32_t swapchain_image_index;
    VK_CHECK(vkAcquireNextImageKHR(m_device, m_swapchain, 1000000000, m_present_semaphore, nullptr, &swapchain_image_index))

    VK_CHECK(vkResetCommandBuffer(m_main_command_buffer, 0))

    // Buffer will be used once so we recreate it when we draw
    // In the long run we will have one secondary buffer we record once and update on map load that will be used
    // to render static elements.
    VkCommandBufferBeginInfo command_buffer_begin_info = {};
    command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    command_buffer_begin_info.pNext = nullptr;
    command_buffer_begin_info.pInheritanceInfo = nullptr;
    command_buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    VK_CHECK(vkBeginCommandBuffer(m_main_command_buffer, &command_buffer_begin_info))

    VkImageMemoryBarrier image_memory_barrier {
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


    const VkRenderingAttachmentInfo color_attachment_info {
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
            .imageView = m_swapchain_images_view[swapchain_image_index],
            .imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .clearValue = VkClearValue{
                .color = {0, 0, flash, 1}
            },
    };

    const VkRenderingInfo render_info {
            .sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR,
            .renderArea = {0, 0, m_window_extent},
            .layerCount = 1,
            .colorAttachmentCount = 1,
            .pColorAttachments = &color_attachment_info,
    };

    vkCmdBeginRendering(m_main_command_buffer, &render_info);

    // Draw calls

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

    // Submit info (our draw calls). We want to wait for present semaphore, and will signal render semaphore when done.
    VkSubmitInfo submit = {};
    submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit.pNext = nullptr;

    VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    submit.pWaitDstStageMask = &wait_stage;

    submit.waitSemaphoreCount = 1;
    submit.pWaitSemaphores = &m_present_semaphore;

    submit.signalSemaphoreCount = 1;
    submit.pSignalSemaphores = &m_render_semaphore;

    submit.commandBufferCount = 1;
    submit.pCommandBuffers = &m_main_command_buffer;

    // Render fence blocked
    VK_CHECK(vkQueueSubmit(m_graphics_queue, 1, &submit, m_render_fence))

    // Present info, we will wait for render semaphore.
    VkPresentInfoKHR present_info = {};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.pNext = nullptr;

    present_info.pSwapchains = &m_swapchain;
    present_info.swapchainCount = 1;

    present_info.pWaitSemaphores = &m_render_semaphore;
    present_info.waitSemaphoreCount = 1;

    present_info.pImageIndices = &swapchain_image_index;

    VK_CHECK(vkQueuePresentKHR(m_graphics_queue, &present_info))

    m_frame_count++;

    glfwSetWindowTitle(m_window, to_string(m_frame_count).c_str());
}

// Small helper, this will be used once, and only here so no need to put it somewhere else
void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    } else {
        throw std::runtime_error("Couldn't find pointer to vkDestroyDebugUtilsMessengerEXT. Please make sure your drivers are up to date.");
    }
}

void Engine::cleanup() {
    if (m_is_initialized) {
        // Wait for render fence
        VK_CHECK(vkWaitForFences(m_device, 1, &m_render_fence, true, 1000000000))
        vkDestroySemaphore(m_device, m_render_semaphore, nullptr);
        vkDestroySemaphore(m_device, m_present_semaphore, nullptr);
        vkDestroyFence(m_device, m_render_fence, nullptr);

        vkDestroyCommandPool(m_device, m_main_command_pool, nullptr);

        vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);

        for (int i = 0; i < m_swapchain_images_view.size(); i++) {

            vkDestroyImageView(m_device, m_swapchain_images_view[i], nullptr);
        }

        vkDestroyDevice(m_device, nullptr);
        vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
        DestroyDebugUtilsMessengerEXT(m_instance, m_debug_messenger, nullptr);
        vkDestroyInstance(m_instance, nullptr);

        glfwDestroyWindow(m_window);
    }
}

void Engine::init_glfw() {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    m_window = glfwCreateWindow(m_window_extent.width, m_window_extent.height, "VulkanEngine", nullptr, nullptr);
}

void Engine::init_vulkan() {
    vkb::InstanceBuilder builder;

    builder.set_app_name("Vulkan game")
#ifndef NDEBUG
            .request_validation_layers(true)
            .use_default_debug_messenger()
#endif
            .set_engine_name("VulkanEngine")
            .require_api_version(1, 3, 0);

    uint32_t glfw_extensions_count;
    const char** extensions = glfwGetRequiredInstanceExtensions(&glfw_extensions_count);

    for (int i = 0; i < glfw_extensions_count; i++) {
        builder.enable_extension(extensions[i]);
    }

    auto vkb_instance = builder.build().value();
    m_instance = vkb_instance.instance;
    m_debug_messenger = vkb_instance.debug_messenger;

    glfwCreateWindowSurface(m_instance, m_window, nullptr, &m_surface);

    vkb::PhysicalDeviceSelector selector{vkb_instance};
    selector.set_minimum_version(1, 3)
            .set_surface(m_surface);

    // VK_KHR_dynamic_rendering isn't an instance extensions, we need to enable
    // it on the device.
    selector.add_required_extension("VK_KHR_dynamic_rendering");

    auto vkb_physical_device = selector.select().value();

    constexpr VkPhysicalDeviceDynamicRenderingFeatures dynamic_rendering_feature {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR,
            .pNext = nullptr,
            .dynamicRendering = VK_TRUE,
    };

    vkb::DeviceBuilder device_builder{vkb_physical_device};
    device_builder.add_pNext<VkPhysicalDeviceDynamicRenderingFeatures>((VkBaseOutStructure *) &dynamic_rendering_feature);

    m_vkb_device = device_builder.build().value();

    m_device = m_vkb_device.device;
    m_physical_device = vkb_physical_device.physical_device;

    m_graphics_queue = m_vkb_device.get_queue(vkb::QueueType::graphics).value();
    m_graphics_queue_family = m_vkb_device.get_queue_index(vkb::QueueType::graphics).value();
}

void Engine::init_swapchain() {
    vkb::SwapchainBuilder swapchain_builder{m_vkb_device, m_surface};

    auto vkb_swapchain = swapchain_builder
            .use_default_format_selection()
                    //use vsync present mode
            .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
            .set_desired_extent(m_window_extent.width, m_window_extent.height)
            .build()
            .value();

    m_swapchain = vkb_swapchain.swapchain;
    m_swapchain_images = vkb_swapchain.get_images().value();
    m_swapchain_images_view = vkb_swapchain.get_image_views().value();

    m_swapchain_image_format = vkb_swapchain.image_format;
}

void Engine::init_commands() {

    auto command_pool_create_info = Initializers::command_pool_create_info(m_graphics_queue_family, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

    VK_CHECK(vkCreateCommandPool(m_device, &command_pool_create_info, nullptr, &m_main_command_pool))

    auto command_buffer_allocate_info = Initializers::command_buffer_allocate_info(m_main_command_pool, 1);

    VK_CHECK(vkAllocateCommandBuffers(m_device, &command_buffer_allocate_info, &m_main_command_buffer))
}

void Engine::init_sync_structures() {
    VkFenceCreateInfo fence_create_info = {};
    fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_create_info.pNext = nullptr;

    //we want to create the fence with the Create Signaled flag, so we can wait on it before using it on a GPU command (for the first frame)
    fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    VK_CHECK(vkCreateFence(m_device, &fence_create_info, nullptr, &m_render_fence));

    VkSemaphoreCreateInfo semaphore_create_info = {};
    semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphore_create_info.pNext = nullptr;
    semaphore_create_info.flags = 0;

    VK_CHECK(vkCreateSemaphore(m_device, &semaphore_create_info, nullptr, &m_present_semaphore))
    VK_CHECK(vkCreateSemaphore(m_device, &semaphore_create_info, nullptr, &m_render_semaphore))
}