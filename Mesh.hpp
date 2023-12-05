//
// Created by gomkyung2 on 12/2/23.
//

#pragma once

#include <tuple>

#include <OGLWrapper/Helper/Mesh.hpp>

template <typename VertexT, typename... InstanceTs>
struct GpuInstancedMesh{
    OGLWrapper::VertexArray vertex_array;
	OGLWrapper::Buffer<GL_ARRAY_BUFFER, VertexT> vertex_buffer;
    std::tuple<OGLWrapper::Buffer<GL_ARRAY_BUFFER, InstanceTs>...> instance_buffers;

    void draw(GLsizei instance_count) const {
        vertex_array.bind();
        glDrawArraysInstanced(GL_TRIANGLES, 0, vertex_buffer.capacity, instance_count);
    }
};