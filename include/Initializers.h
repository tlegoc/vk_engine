//
// Created by theo on 02/07/23.
//

#ifndef VK_ENGINE_INITIALIZERS_H
#define VK_ENGINE_INITIALIZERS_H

#include <vulkan/vulkan.h>

namespace Initializers {
    VkCommandPoolCreateInfo command_pool_create_info(uint32_t queue_family_index, VkCommandPoolCreateFlags create_flags = 0);

    VkCommandBufferAllocateInfo command_buffer_allocate_info(VkCommandPool pool, uint32_t count = 1, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);
}


#endif //VK_ENGINE_INITIALIZERS_H
