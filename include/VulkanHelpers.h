//
// Created by theo on 19/07/2023.
//

#ifndef VK_ENGINE_VULKANHELPERS_H
#define VK_ENGINE_VULKANHELPERS_H

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

struct AllocatedBuffer {
    VkBuffer m_buffer;
    VmaAllocation m_allocation;
};

struct AllocatedImage {
    VkImage m_image;
    VmaAllocation m_allocation;
};


#endif //VK_ENGINE_VULKANHELPERS_H
