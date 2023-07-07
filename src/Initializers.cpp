//
// Created by theo on 02/07/23.
//

#include "Initializers.h"

VkCommandPoolCreateInfo
Initializers::command_pool_create_info(uint32_t queue_family_index, VkCommandPoolCreateFlags create_flags) {
    VkCommandPoolCreateInfo command_pool_create_info{};
    command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    command_pool_create_info.pNext = nullptr;

    command_pool_create_info.queueFamilyIndex = queue_family_index;
    command_pool_create_info.flags = create_flags;

    return command_pool_create_info;
}

VkCommandBufferAllocateInfo
Initializers::command_buffer_allocate_info(VkCommandPool pool, uint32_t count, VkCommandBufferLevel level) {
    VkCommandBufferAllocateInfo command_buffer_allocate_info{};
    command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    command_buffer_allocate_info.pNext = nullptr;

    command_buffer_allocate_info.commandPool = pool;
    command_buffer_allocate_info.commandBufferCount = count;
    command_buffer_allocate_info.level = level;

    return command_buffer_allocate_info;
}