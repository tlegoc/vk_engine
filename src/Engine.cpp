//
// Created by theo on 02/07/23.
//

#include <Engine.h>

#include <Initializers.h>
#include <Helpers.h>

#include <iostream>

using namespace std;
#define VK_CHECK(x)                                                     \
        {                                                               \
        VkResult err = x;                                               \
        if (err)                                                        \
        {                                                               \
            std::cout <<"Detected Vulkan error: " << err << std::endl;  \
            abort();                                                    \
        }                                                               \
        }

void Engine::init() {

    init_glfw();
    init_vulkan();
    init_swapchain();

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

}

// Small helper, this will be used once, and only here
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
        vkDestroyCommandPool(m_device, m_main_command_pool, nullptr);

        vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);

        //destroy swapchain resources
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

    builder.set_app_name("VulkanEngine")
            .request_validation_layers(true)
            .require_api_version(1, 3, 0)
            .use_default_debug_messenger();

    std::cout << "Available extensions:" << std::endl;

    for (auto ext : Helpers::get_supported_extensions()) {
        std::cout << "\t" << ext << std::endl;
    }

    uint32_t glfw_extensions_count;
    const char** extensions = glfwGetRequiredInstanceExtensions(&glfw_extensions_count);

    for (int i = 0; i < glfw_extensions_count; i++) {
        std::cout << "Enabling extension: " << extensions[i] << std::endl;
        builder.enable_extension(extensions[i]);
    }

    std::cout << "Enabling extension: VK_KHR_dynamic_rendering" << std::endl;
    builder.enable_extension("VK_KHR_dynamic_rendering");

    auto inst_result = builder.build();

    vkb::Instance vkb_inst = inst_result.value();

    m_instance = vkb_inst.instance;

    m_debug_messenger = vkb_inst.debug_messenger;

    glfwCreateWindowSurface(m_instance, m_window, nullptr, &m_surface);

    vkb::PhysicalDeviceSelector selector{vkb_inst};
    m_vkb_physical_device = selector
            .set_minimum_version(1, 3)
            .set_surface(m_surface)
            .select()
            .value();


    vkb::DeviceBuilder device_builder{m_vkb_physical_device};

    m_vkb_device = device_builder.build().value();

    m_device = m_vkb_device.device;
    m_physical_device = m_vkb_physical_device.physical_device;

    m_graphics_queue = m_vkb_device.get_queue(vkb::QueueType::graphics).value();
    m_graphics_queue_family = m_vkb_device.get_queue_index(vkb::QueueType::graphics).value();
}

void Engine::init_swapchain() {
    vkb::SwapchainBuilder swapchain_builder{m_vkb_device, m_surface};

    m_vkb_swapchain = swapchain_builder
            .use_default_format_selection()
                    //use vsync present mode
            .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
            .set_desired_extent(m_window_extent.width, m_window_extent.height)
            .build()
            .value();

    m_swapchain = m_vkb_swapchain.swapchain;
    m_swapchain_images = m_vkb_swapchain.get_images().value();
    m_swapchain_images_view = m_vkb_swapchain.get_image_views().value();

    m_swapchain_image_format = m_vkb_swapchain.image_format;
}

void Engine::init_commands() {

    auto command_pool_create_info = Initializers::command_pool_create_info(m_graphics_queue_family, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

    VK_CHECK(vkCreateCommandPool(m_device, &command_pool_create_info, nullptr, &m_main_command_pool))

    auto command_buffer_allocate_info = Initializers::command_buffer_allocate_info(m_main_command_pool, 1);

    VK_CHECK(vkAllocateCommandBuffers(m_device, &command_buffer_allocate_info, &m_main_command_buffer))
}