//
// Created by theo on 02/07/23.
//

#include <Engine.h>

#include <Initializers.h>
#include <PipelineBuilder.h>

#define VMA_IMPLEMENTATION

#include <vk_mem_alloc.h>

#include <iostream>
#include <fstream>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

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
    init_base_pipelines();
    init_debug_meshes();

    m_is_initialized = true;
}

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

    // Don't forget to include all color attachment.
    // You can select the desired output image in your shader by doing
    // layout(location = COLOR_ATTACHMENT INDEX) vecX variable_name;
    const VkRenderingInfo render_info{
            .sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR,
            .renderArea = {0, 0, m_window_extent},
            .layerCount = 1,
            .colorAttachmentCount = 1,
            .pColorAttachments = &color_attachment_info,
    };

    // We make use of dynamic_rendering. This allows us to forget about renderpasses and
    // framebuffers completely. We can specify render attachments on the struct
    // above
    vkCmdBeginRendering(m_main_command_buffer, &render_info);

    render_commands();

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

void Engine::render_commands() {
    vkCmdBindPipeline(m_main_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_debug_mesh_pipeline);

    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(m_main_command_buffer, 0, 1, &m_debug_triangle_mesh.m_vertex_buffer.m_buffer, &offset);

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

        MeshPushConstants constants;
        constants.render_matrix = mesh_matrix;

        //upload the matrix to the GPU via push constants
        vkCmdPushConstants(m_main_command_buffer, m_debug_mesh_pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0,
                           sizeof(MeshPushConstants), &constants);
    }
    
    vkCmdDraw(m_main_command_buffer, m_debug_triangle_mesh.m_vertices.size(), 1, 0, 0);
}

// Small helper, this will be used once, and only here so no need to put it somewhere else
void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
                                   const VkAllocationCallbacks *pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance,
                                                                            "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    } else {
        throw std::runtime_error(
                "Couldn't find pointer to vkDestroyDebugUtilsMessengerEXT. Please make sure your drivers are up to date.");
    }
}

void Engine::cleanup() {
    if (m_is_initialized) {
        // Wait for render fence
        VK_CHECK(vkWaitForFences(m_device, 1, &m_render_fence, true, 1000000000))
        m_main_deletion_queue.flush();

        vmaDestroyAllocator(m_allocator);

        vkDestroyDevice(m_device, nullptr);
        vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
        DestroyDebugUtilsMessengerEXT(m_instance, m_debug_messenger, nullptr);
        vkDestroyInstance(m_instance, nullptr);

        glfwDestroyWindow(m_window);
    }
}

bool Engine::load_shader_module(const char *file_path, VkShaderModule *out_shader_module) {
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
    const char **extensions = glfwGetRequiredInstanceExtensions(&glfw_extensions_count);

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

    VkPhysicalDeviceDynamicRenderingFeatures dynamic_rendering_feature{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR,
            .pNext = nullptr,
            .dynamicRendering = VK_TRUE,
    };

    vkb::DeviceBuilder device_builder{vkb_physical_device};
    device_builder.add_pNext<VkPhysicalDeviceDynamicRenderingFeatures>(&dynamic_rendering_feature);

    m_vkb_device = device_builder.build().value();

    m_device = m_vkb_device.device;
    m_physical_device = vkb_physical_device.physical_device;

    m_graphics_queue = m_vkb_device.get_queue(vkb::QueueType::graphics).value();
    m_graphics_queue_family = m_vkb_device.get_queue_index(vkb::QueueType::graphics).value();


    VmaAllocatorCreateInfo allocatorInfo = {
            .physicalDevice = m_physical_device,
            .device = m_device,
            .instance = m_instance
    };

    vmaCreateAllocator(&allocatorInfo, &m_allocator);
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


    for (auto &i: m_swapchain_images_view) {
        m_main_deletion_queue.push_function([=, this]() {
            vkDestroyImageView(m_device, i, nullptr);
        });
    }

    m_main_deletion_queue.push_function([=, this]() {
        vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
    });
}

void Engine::init_commands() {
    VkCommandPoolCreateInfo command_pool_create_info{
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .pNext = nullptr,
            .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            .queueFamilyIndex = m_graphics_queue_family
    };
    VK_CHECK(vkCreateCommandPool(m_device, &command_pool_create_info, nullptr, &m_main_command_pool))

    VkCommandBufferAllocateInfo command_buffer_allocate_info{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .pNext = nullptr,
            .commandPool = m_main_command_pool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1
    };
    VK_CHECK(vkAllocateCommandBuffers(m_device, &command_buffer_allocate_info, &m_main_command_buffer))

    m_main_deletion_queue.push_function([=, this]() {
        vkDestroyCommandPool(m_device, m_main_command_pool, nullptr);
    });
}

void Engine::init_sync_structures() {
    VkFenceCreateInfo fence_create_info = {
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .pNext = nullptr,
            //we want to create the fence with the Create Signaled flag, so we can wait on it before using it on a GPU command (for the first frame)
            .flags = VK_FENCE_CREATE_SIGNALED_BIT
    };
    VK_CHECK(vkCreateFence(m_device, &fence_create_info, nullptr, &m_render_fence))

    m_main_deletion_queue.push_function([=, this]() {
        vkDestroyFence(m_device, m_render_fence, nullptr);
    });

    VkSemaphoreCreateInfo semaphore_create_info = {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0
    };
    VK_CHECK(vkCreateSemaphore(m_device, &semaphore_create_info, nullptr, &m_present_semaphore))
    VK_CHECK(vkCreateSemaphore(m_device, &semaphore_create_info, nullptr, &m_render_semaphore))

    m_main_deletion_queue.push_function([=, this]() {
        vkDestroySemaphore(m_device, m_render_semaphore, nullptr);
    });
    m_main_deletion_queue.push_function([=, this]() {
        vkDestroySemaphore(m_device, m_present_semaphore, nullptr);
    });
}

void Engine::init_base_pipelines() {
    VkShaderModule triangle_frag_shader;
    if (!load_shader_module("triangle.frag.spv", &triangle_frag_shader)) {
        std::cout << "Error when building the triangle fragment shader module" << std::endl;
        abort();
    }

    VkShaderModule triangle_vertex_shader;
    if (!load_shader_module("triangle.vert.spv", &triangle_vertex_shader)) {
        std::cout << "Error when building the triangle vertex shader module" << std::endl;
        abort();
    }

    VkShaderModule base_trimesh_vertex_shader;
    if (!load_shader_module("base_trimesh.vert.spv", &base_trimesh_vertex_shader)) {
        std::cout << "Error when building the base trimesh vertex shader module" << std::endl;
        abort();
    }

    VkShaderModule base_vertex_color_frag_shader;
    if (!load_shader_module("base_vertex_color.frag.spv", &base_vertex_color_frag_shader)) {
        std::cout << "Error when building the base vertex color shader module" << std::endl;
        abort();
    }

    VkPipelineLayoutCreateInfo pipeline_layout_info = Initializers::pipeline_layout_create_info();
    VkPipelineLayoutCreateInfo mesh_pipeline_layout_info = Initializers::pipeline_layout_create_info();

    VkPushConstantRange push_constant;
    push_constant.offset = 0;
    push_constant.size = sizeof(MeshPushConstants);
    push_constant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    mesh_pipeline_layout_info.pPushConstantRanges = &push_constant;
    mesh_pipeline_layout_info.pushConstantRangeCount = 1;

    VK_CHECK(vkCreatePipelineLayout(m_device, &mesh_pipeline_layout_info, nullptr, &m_debug_mesh_pipeline_layout));
    VK_CHECK(vkCreatePipelineLayout(m_device, &pipeline_layout_info, nullptr, &m_triangle_pipeline_layout));

    PipelineBuilder pipeline_builder;

    pipeline_builder.m_shader_stages.push_back(
            Initializers::pipeline_shader_stage_create_info(VK_SHADER_STAGE_VERTEX_BIT, triangle_vertex_shader));

    pipeline_builder.m_shader_stages.push_back(
            Initializers::pipeline_shader_stage_create_info(VK_SHADER_STAGE_FRAGMENT_BIT, triangle_frag_shader));


    //vertex input controls how to read vertices from vertex buffers.
    pipeline_builder.m_vertex_input_info = Initializers::vertex_input_state_create_info();

    pipeline_builder.m_input_assembly = Initializers::input_assembly_create_info(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

    pipeline_builder.m_viewport.x = 0.0f;
    pipeline_builder.m_viewport.y = 0.0f;
    pipeline_builder.m_viewport.width = (float) m_window_extent.width;
    pipeline_builder.m_viewport.height = (float) m_window_extent.height;
    pipeline_builder.m_viewport.minDepth = 0.0f;
    pipeline_builder.m_viewport.maxDepth = 1.0f;

    pipeline_builder.m_scissor.offset = {0, 0};
    pipeline_builder.m_scissor.extent = m_window_extent;

    pipeline_builder.m_rasterizer = Initializers::rasterization_state_create_info(VK_POLYGON_MODE_FILL);

    pipeline_builder.m_multisampling = Initializers::multisampling_state_create_info();

    //a single blend attachment with no blending and writing to RGBA
    pipeline_builder.m_color_blend_attachment = Initializers::color_blend_attachment_state();

    pipeline_builder.m_pipeline_layout = m_triangle_pipeline_layout;

    m_triangle_pipeline = pipeline_builder.build_pipeline(m_device);

    //
    //
    //base trimesh pipeline
    pipeline_builder.m_shader_stages.clear();

    VertexInputDescription vertex_description = Vertex::get_vertex_description();

    //connect the pipeline builder vertex input info to the one we get from Vertex
    pipeline_builder.m_vertex_input_info.pVertexAttributeDescriptions = vertex_description.attributes.data();
    pipeline_builder.m_vertex_input_info.vertexAttributeDescriptionCount = vertex_description.attributes.size();

    pipeline_builder.m_vertex_input_info.pVertexBindingDescriptions = vertex_description.bindings.data();
    pipeline_builder.m_vertex_input_info.vertexBindingDescriptionCount = vertex_description.bindings.size();

    pipeline_builder.m_pipeline_layout = m_debug_mesh_pipeline_layout;

    pipeline_builder.m_shader_stages.push_back(
            Initializers::pipeline_shader_stage_create_info(VK_SHADER_STAGE_VERTEX_BIT, base_trimesh_vertex_shader));

    pipeline_builder.m_shader_stages.push_back(
            Initializers::pipeline_shader_stage_create_info(VK_SHADER_STAGE_FRAGMENT_BIT, base_vertex_color_frag_shader));

    m_debug_mesh_pipeline = pipeline_builder.build_pipeline(m_device);

    vkDestroyShaderModule(m_device, triangle_frag_shader, nullptr);
    vkDestroyShaderModule(m_device, triangle_vertex_shader, nullptr);
    vkDestroyShaderModule(m_device, base_trimesh_vertex_shader, nullptr);
    vkDestroyShaderModule(m_device, base_vertex_color_frag_shader, nullptr);

    m_main_deletion_queue.push_function([=, this]() {
        vkDestroyPipeline(m_device, m_triangle_pipeline, nullptr);
        vkDestroyPipeline(m_device, m_debug_mesh_pipeline, nullptr);
        vkDestroyPipelineLayout(m_device, m_debug_mesh_pipeline_layout, nullptr);
        vkDestroyPipelineLayout(m_device, m_triangle_pipeline_layout, nullptr);
    });
}

void Engine::init_debug_meshes() {
    m_debug_triangle_mesh.m_vertices.resize(3);

    m_debug_triangle_mesh.m_vertices[0].position = {1.f, 1.f, 0.f};
    m_debug_triangle_mesh.m_vertices[1].position = {-1.f, 1.f, 0.f};
    m_debug_triangle_mesh.m_vertices[2].position = {0.f, -1.f, 0.f};

    m_debug_triangle_mesh.m_vertices[0].color = {0.f, 1.f, 0.f};
    m_debug_triangle_mesh.m_vertices[1].color = {1.f, 0.f, 0.f};
    m_debug_triangle_mesh.m_vertices[2].color = {0.f, 0.f, 1.f};

    upload_mesh(m_debug_triangle_mesh);
}

void Engine::upload_mesh(Mesh &mesh) {
    VkBufferCreateInfo buffer_info = {};
    buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.size = mesh.m_vertices.size() * sizeof(Vertex);
    buffer_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;


    VmaAllocationCreateInfo vma_alloc_info = {};
    vma_alloc_info.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

    VK_CHECK(vmaCreateBuffer(m_allocator, &buffer_info, &vma_alloc_info,
                             &mesh.m_vertex_buffer.m_buffer,
                             &mesh.m_vertex_buffer.m_allocation,
                             nullptr));

    m_main_deletion_queue.push_function([=]() {

        vmaDestroyBuffer(m_allocator, mesh.m_vertex_buffer.m_buffer, mesh.m_vertex_buffer.m_allocation);
    });

    // Copy the data
    void *data;
    vmaMapMemory(m_allocator, mesh.m_vertex_buffer.m_allocation, &data);

    memcpy(data, mesh.m_vertices.data(), mesh.m_vertices.size() * sizeof(Vertex));

    vmaUnmapMemory(m_allocator, mesh.m_vertex_buffer.m_allocation);
}