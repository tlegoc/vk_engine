//
// Created by theo on 02/07/23.
//

#ifndef VK_ENGINE_ENGINE_H
#define VK_ENGINE_ENGINE_H

#include <Types.h>
#include <GLFW/glfw3.h>
#include <VkBootstrap.h>

#include <vector>

class Engine {
public:

    bool m_is_initialized = false;
    bool m_frame_count = 0;

    VkExtent2D m_window_extent = { 1280, 720 };

    // Window
    GLFWwindow* m_window;

    void init();
    void run();
    void draw();
    void cleanup();
    static void print_system_info();

    vkb::Instance m_vkb_instance;
    vkb::PhysicalDevice m_vkb_physical_device;
    vkb::Device m_vkb_device;
    vkb::Swapchain m_vkb_swapchain;

    VkInstance m_instance;
    VkDebugUtilsMessengerEXT m_debug_messenger;

    VkPhysicalDevice m_physical_device;
    VkDevice m_device;

    VkSwapchainKHR m_swapchain;
    VkFormat m_swapchain_image_format;
    VkSurfaceKHR m_surface;
    std::vector<VkImage> m_swapchain_images;
    std::vector<VkImageView> m_swapchain_images_view;

    VkQueue m_graphics_queue;
    uint32_t m_graphics_queue_family;

    VkCommandPool m_main_command_pool;
    VkCommandBuffer m_main_command_buffer;

private:
    void init_glfw();
    void init_vulkan();
    void init_swapchain();
    void init_commands();
};


#endif //VK_ENGINE_ENGINE_H
