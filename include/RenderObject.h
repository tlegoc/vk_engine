//
// Created by theo on 22/07/2023.
//

#ifndef VK_ENGINE_RENDEROBJECT_H
#define VK_ENGINE_RENDEROBJECT_H

#include <Mesh.h>
#include <Material.h>

#include <glm/glm.hpp>

struct RenderObject {
    Mesh *m_mesh;

    Material *m_material;

    glm::mat4 m_transform_matrix;
};


#endif //VK_ENGINE_RENDEROBJECT_H
