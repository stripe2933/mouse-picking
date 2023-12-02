//
// Created by gomkyung2 on 11/27/23.
//

#pragma once

#include <istream>
#include <type_traits>

#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float3.hpp>
#include <OGLWrapper/Helper/VertexAttributes.hpp>

inline std::istream &operator>>(std::istream &os, glm::vec2 &v) {
    return os >> v.x >> v.y;
}

inline std::istream &operator>>(std::istream &os, glm::vec3 &v) {
    return os >> v.x >> v.y >> v.z;
}

struct VertexPNT {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texcoords;

    friend std::istream &operator>>(std::istream &is, VertexPNT &v) {
        return is >> v.position >> v.normal >> v.texcoords;
    }
};
static_assert(std::is_standard_layout_v<VertexPNT>);

template <>
struct OGLWrapper::Helper::VertexAttributes<VertexPNT> {
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