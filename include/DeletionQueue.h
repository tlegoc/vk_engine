//
// Created by theo on 13/07/2023.
//

#ifndef VK_ENGINE_DELETIONQUEUE_H
#define VK_ENGINE_DELETIONQUEUE_H

#include <deque>
#include <functional>

struct DeletionQueue {
    std::deque<std::function<void()>> deletors;

    void push_function(std::function<void()>&& function);

    void flush();
};


#endif //VK_ENGINE_DELETIONQUEUE_H
