//
// Created by theo on 02/07/23.
//

#include <Engine.h>

#include <Initializers.h>
#include <PipelineBuilder.h>

#include <vulkan/vulkan.h>
#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>


void Engine::init() {

    m_frame_count = 0;

    init_glfw();
    init_vulkan();
    init_swapchain();
    init_commands();
    init_sync_structures();
    init_base_pipelines();
#ifndef NDEBUG
    init_debug_meshes();
#endif

    m_is_initialized = true;
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

    // Depth texture
    VkExtent3D depth_image_extent = {
            m_window_extent.width,
            m_window_extent.height,
            1
    };

    m_depth_format = VK_FORMAT_D32_SFLOAT;

    VkImageCreateInfo dimg_info = Initializers::image_create_info(m_depth_format,
                                                                  VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                                                                  depth_image_extent);

    VmaAllocationCreateInfo dimg_allocinfo = {};
    dimg_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    dimg_allocinfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vmaCreateImage(m_allocator, &dimg_info, &dimg_allocinfo, &m_depth_image.m_image, &m_depth_image.m_allocation,
                   nullptr);

    VkImageViewCreateInfo dview_info = Initializers::imageview_create_info(m_depth_format, m_depth_image.m_image,
                                                                           VK_IMAGE_ASPECT_DEPTH_BIT);

    VK_CHECK(vkCreateImageView(m_device, &dview_info, nullptr, &m_depth_image_view));

    m_main_deletion_queue.push_function([=, this]() {
        vkDestroyImageView(m_device, m_depth_image_view, nullptr);
        vmaDestroyImage(m_allocator, m_depth_image.m_image, m_depth_image.m_allocation);
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
    VK_CHECK(vkCreatePipelineLayout(m_device, &pipeline_layout_info, nullptr, &m_triangle_pipeline_layout));

    VkPipelineLayoutCreateInfo mesh_pipeline_layout_info = Initializers::pipeline_layout_create_info();
    VkPushConstantRange push_constant;
    push_constant.offset = 0;
    push_constant.size = sizeof(MeshPushConstants);
    push_constant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    mesh_pipeline_layout_info.pPushConstantRanges = &push_constant;
    mesh_pipeline_layout_info.pushConstantRangeCount = 1;
    VK_CHECK(vkCreatePipelineLayout(m_device, &mesh_pipeline_layout_info, nullptr, &m_debug_mesh_pipeline_layout));

    PipelineBuilder pipeline_builder;

    pipeline_builder.setup_default(m_window_extent);

    pipeline_builder.m_depth_stencil_format = m_depth_format;
    pipeline_builder.m_depth_stencil = Initializers::depth_stencil_create_info(true, true, VK_COMPARE_OP_LESS_OR_EQUAL);


    pipeline_builder.m_shader_stages.push_back(
            Initializers::pipeline_shader_stage_create_info(VK_SHADER_STAGE_VERTEX_BIT, triangle_vertex_shader));

    pipeline_builder.m_shader_stages.push_back(
            Initializers::pipeline_shader_stage_create_info(VK_SHADER_STAGE_FRAGMENT_BIT, triangle_frag_shader));

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
            Initializers::pipeline_shader_stage_create_info(VK_SHADER_STAGE_FRAGMENT_BIT,
                                                            base_vertex_color_frag_shader));

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

    m_debug_monkey_mesh.load_from_obj("./assets/monkey.obj");

    upload_mesh(m_debug_triangle_mesh);
    upload_mesh(m_debug_monkey_mesh);
}
