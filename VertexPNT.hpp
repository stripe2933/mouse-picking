//
// Created by gomkyung2 on 11/27/23.
//

#pragma once

#include <type_traits>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include "VertexAttributes.hpp"

struct VertexPNT {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texcoords;
};
static_assert(std::is_standard_layout_v<VertexPNT>);

template <>
struct VertexAttributes<VertexPNT> {
    GLuint position;
    GLuint normal;
    GLuint texcoords;

    void setVertexAttribArrays() const {
        glEnableVertexAttribArray(position);
        glVertexAttribPointer(
            position,
            3,
            GL_FLOAT,
            GL_FALSE,
            sizeof(VertexPNT),
            reinterpret_cast<const void*>(offsetof(VertexPNT, position)));

        glEnableVertexAttribArray(normal);
        glVertexAttribPointer(
            normal,
            3,
            GL_FLOAT,
            GL_FALSE,
            sizeof(VertexPNT),
            reinterpret_cast<const void*>(offsetof(VertexPNT, normal)));

        glEnableVertexAttribArray(texcoords);
        glVertexAttribPointer(
            texcoords,
            2,
            GL_FLOAT,
            GL_FALSE,
            sizeof(VertexPNT),
            reinterpret_cast<const void*>(offsetof(VertexPNT, texcoords)));
    }
};