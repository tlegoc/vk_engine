//
// Created by theo on 02/07/23.
//

#ifndef VK_ENGINE_ENGINE_H
#define VK_ENGINE_ENGINE_H

#include <Types.h>
#include <DeletionQueue.h>

#include <GLFW/glfw3.h>
#include <VkBootstrap.h>

#include <vector>

class Engine {
public:

    bool m_is_initialized = false;
    uint32_t m_frame_count = 0;

    VkExtent2D m_window_extent = { 1280, 720 };

    // Window
    GLFWwindow* m_window;

    void init();
    void run();
    void draw();
    void render_commands();
    void cleanup();

    bool load_shader_module(const char* file_path, VkShaderModule* out_shader_module);

    DeletionQueue m_main_deletion_queue;
    DeletionQueue m_per_frame_deletion_queue;

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

    VkRenderingInfo m_render_info;
    VkSemaphore m_present_semaphore;
    VkSemaphore m_render_semaphore;
    VkFence m_render_fence;

    VkPipelineLayout m_triangle_pipeline_layout;
    VkPipeline m_triangle_pipeline;

private:
    // We need it for swapchain creation
    vkb::Device m_vkb_device;

    void init_glfw();
    void init_vulkan();
    void init_swapchain();
    void init_commands();
    void init_sync_structures();
    void init_base_pipelines();
};


#endif //VK_ENGINE_ENGINE_H
