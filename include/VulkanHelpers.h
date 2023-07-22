//
// Created by theo on 19/07/2023.
//

#ifndef VK_ENGINE_VULKANHELPERS_H
#define VK_ENGINE_VULKANHELPERS_H



#include <iostream>
#define VK_CHECK(x)                                                     \
        {                                                               \
        VkResult err = x;                                               \
        if (err)                                                        \
        {                                                               \
            std::cout << "Detected Vulkan error: " << err << std::endl; \
            abort();                                                    \
        }                                                               \
        }

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
