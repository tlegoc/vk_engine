//
// Created by theo on 02/07/23.
//

#ifndef VK_ENGINE_ENGINE_H
#define VK_ENGINE_ENGINE_H

#include <DeletionQueue.h>
#include <Mesh.h>
#include <Material.h>
#include <RenderObject.h>
#include <VulkanHelpers.h>

#include <GLFW/glfw3.h>
#include <VkBootstrap.h>
#include <vk_mem_alloc.h>

#include <vector>
#include <unordered_map>

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
    void cmd_render_commands();
    void cleanup();

    bool load_shader_module(const char* file_path, VkShaderModule* out_shader_module) const;
    void upload_mesh(Mesh& mesh);

    // Resource management
    DeletionQueue m_main_deletion_queue;
    DeletionQueue m_per_frame_deletion_queue;

    // Memory
    VmaAllocator m_allocator;

    // Global Vulkan object
    VkInstance m_instance;
    VkDebugUtilsMessengerEXT m_debug_messenger;

    VkPhysicalDevice m_physical_device;
    VkDevice m_device;

    // We need it for swapchain creation
    vkb::Device m_vkb_device;

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

    VkImageView m_depth_image_view;
    AllocatedImage m_depth_image;
    VkFormat m_depth_format;

    // Rendering data
    std::vector<RenderObject> m_renderables;

    std::unordered_map<std::string, Material> m_materials;
    std::unordered_map<std::string, Mesh> m_meshes;

    Material* create_material(VkPipeline pipeline, VkPipelineLayout layout,const std::string& name);

    Material* get_material(const std::string& name);

    Mesh* get_mesh(const std::string& name);

    void draw_objects(VkCommandBuffer cmd,RenderObject* first, int count);

    // Debug and tests pipeline, meshes, etc
    VkPipelineLayout m_triangle_pipeline_layout;
    VkPipeline m_triangle_pipeline;

    VkPipelineLayout m_debug_mesh_pipeline_layout;
    VkPipeline m_debug_mesh_pipeline;
    Mesh m_debug_triangle_mesh;
    Mesh m_debug_monkey_mesh;


private:
    void init_glfw();
    void init_vulkan();
    void init_swapchain();
    void init_commands();
    void init_sync_structures();
    void init_base_pipelines();
    void init_debug_meshes();
};


#endif //VK_ENGINE_ENGINE_H
