//
// Created by theo on 02/07/23.
//

#ifndef VK_ENGINE_TYPES_H
#define VK_ENGINE_TYPES_H

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

namespace Types {
    struct AllocatedBuffer {
        VkBuffer m_buffer;
        VmaAllocation m_allocation;
    };
}

#endif //VK_ENGINE_TYPES_H
