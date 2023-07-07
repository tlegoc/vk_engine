//
// Created by theo on 07/07/23.
//

#include "Helpers.h"

#include <vulkan/vulkan.h>

#include <vector>
#include <iostream>

// https://stackoverflow.com/questions/42057041/how-to-get-supported-extensions-in-vulkan
std::set<std::string> Helpers::get_supported_extensions() {
    VkResult result;

    uint32_t count = 0;
    result = vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
    if (result != VK_SUCCESS) {
        std::cout << "vkEnumerateInstanceExtensionProperties failed: " << result << std::endl;
        return {};
    }

    std::vector<VkExtensionProperties> extensionProperties(count);

    result = vkEnumerateInstanceExtensionProperties(nullptr, &count, extensionProperties.data());
    if (result != VK_SUCCESS) {
        std::cout << "vkEnumerateInstanceExtensionProperties failed : " << result << std::endl;
        return {};
    }

    std::set<std::string> extensions;
    for (auto &extension: extensionProperties) {
        extensions.insert(extension.extensionName);
    }

    return extensions;
}