//
// Created by theo on 13/07/2023.
//

#ifndef VK_ENGINE_DELETIONQUEUE_H
#define VK_ENGINE_DELETIONQUEUE_H

#include <deque>
#include <functional>

struct DeletionQueue {
    std::deque<std::function<void()>> deletors;

    void push_function(std::function<void()>&& function) {
        deletors.push_back(function);
    }

    void flush() {
        // reverse iterate the deletion queue to execute all the functions
        for (auto it = deletors.rbegin(); it != deletors.rend(); it++) {
            (*it)(); //call the function
        }

        deletors.clear();
    }
};


#endif //VK_ENGINE_DELETIONQUEUE_H
