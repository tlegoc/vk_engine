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
            std::cout << "Detected Vulkan error: " << err << std::endl; \
            abort();                                                    \
        }                                                               \
        }

void Engine::init() {

    init_glfw();
#ifndef NDEBUG
    print_system_info();
#endif
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

void Engine::print_system_info() {
    std::cout << "------ System info:" << std::endl;
    auto system_info_ret = vkb::SystemInfo::get_system_info();
    if (!system_info_ret) {
        std::cout << system_info_ret.error().message() << std::endl;
        abort();
    }
    auto system_info = system_info_ret.value();

    std::cout << "Available layers:" << std::endl;

    for (auto ext : system_info.available_layers) {
        std::cout << "\t" << ext.layerName << std::endl;
    }

    std::cout << "Available extensions:" << std::endl;

    for (auto ext : system_info.available_extensions) {
        std::cout << "\t" << ext.extensionName << std::endl;
    }
    std::cout << "------" << std::endl;
}

void Engine::init_glfw() {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    m_window = glfwCreateWindow(m_window_extent.width, m_window_extent.height, "VulkanEngine", nullptr, nullptr);
}

void Engine::init_vulkan() {
#ifndef NDEBUG
    std::cout << "Building instance..." << std::endl;
#endif
    vkb::InstanceBuilder builder;

    builder.set_app_name("VulkanEngine")
#ifndef NDEBUG
            .request_validation_layers(true)
            .use_default_debug_messenger()
#endif
            .set_engine_name("VulkanEngine")
            .require_api_version(1, 3, 0);

    uint32_t glfw_extensions_count;
    const char** extensions = glfwGetRequiredInstanceExtensions(&glfw_extensions_count);

    for (int i = 0; i < glfw_extensions_count; i++) {
        //builder.enable_extension(extensions[i]);
    }

    //builder.enable_extension("VK_KHR_dynamic_rendering");

    m_vkb_instance = builder.build().value();
    m_instance = m_vkb_instance.instance;
    m_debug_messenger = m_vkb_instance.debug_messenger;

#ifndef NDEBUG
    std::cout << "Done." << std::endl;
    std::cout << "Creating surface..." << std::endl;
#endif

    glfwCreateWindowSurface(m_instance, m_window, nullptr, &m_surface);

#ifndef NDEBUG
    std::cout << "Done." << std::endl;
    std::cout << "Selecting physical device..." << std::endl;
#endif

    vkb::PhysicalDeviceSelector selector{m_vkb_instance};
    selector.set_minimum_version(1, 3)
            .set_surface(m_surface);

    for (int i = 0; i < glfw_extensions_count; i++) {
        //selector.add_required_extension(extensions[i]);
    }

    //selector.add_required_extension("VK_KHR_dynamic_rendering");

    m_vkb_physical_device = selector.select().value();

#ifndef NDEBUG
    std::cout << "Done." << std::endl;
    std::cout << "Building device..." << std::endl;
#endif

    vkb::DeviceBuilder device_builder{m_vkb_physical_device};

    m_vkb_device = device_builder.build().value();

    m_device = m_vkb_device.device;
    m_physical_device = m_vkb_physical_device.physical_device;

    m_graphics_queue = m_vkb_device.get_queue(vkb::QueueType::graphics).value();
    m_graphics_queue_family = m_vkb_device.get_queue_index(vkb::QueueType::graphics).value();

#ifndef NDEBUG
    std::cout << "Done." << std::endl;
    std::cout << "Vulkan initialization finished." << std::endl;
#endif
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