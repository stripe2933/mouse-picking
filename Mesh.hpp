//
// Created by gomkyung2 on 11/27/23.
//

#pragma once

#include <vector>

#include <OGLWrapper/VertexArray.hpp>
#include <OGLWrapper/Buffer.hpp>

#include "VertexAttributes.hpp"

template <typename VertexT>
struct MeshBase {
    using vertex_type = VertexT;

    std::vector<VertexT> vertices;
};

template <typename VertexT>
struct GpuFacetedMesh{
    OGLWrapper::VertexArray vertex_arrray;
    OGLWrapper::Buffer<VertexT> vertex_buffer;

    void draw() const {
        glBindVertexArray(vertex_arrray.handle);
        glDrawArrays(GL_TRIANGLES, 0, vertex_buffer.capacity);
    }
};

template <typename VertexT>
struct FacetedMesh : MeshBase<VertexT>{
    explicit FacetedMesh(std::vector<VertexT> vertices) : MeshBase<VertexT>{ std::move(vertices) } {

    }

    GpuFacetedMesh<VertexT> transferToGpu(
        const VertexAttributes<VertexT> &attribute_indices,
        OGLWrapper::BufferUsage usage) const
    {
        OGLWrapper::VertexArray vertex_array{};
        glBindVertexArray(vertex_array.handle);

        OGLWrapper::Buffer<VertexT> vertex_buffer{ OGLWrapper::BufferBindingTarget::ArrayBuffer, usage };
        vertex_buffer.store(MeshBase<VertexT>::vertices);

        attribute_indices.setVertexAttribArrays();

        return { std::move(vertex_array), std::move(vertex_buffer) };
    }
};

// template <typename VertexT, std::integral IndexT>
//     requires std::invocable<decltype(VertexT::setVertexAttribArray), const typename VertexT::AttributeIndices &>
// struct SmoothMesh{
//     using vertex_type = VertexT;
//     using index_type = IndexT;
//
//     std::vector<VertexT> vertices;
//     std::vector<IndexT> indices;
//
//     void transfer(OGLWrapper::BufferBase<VertexT> &vertex_buffer,
//                   OGLWrapper::BufferBase<IndexT> &index_buffer,
//                   const typename VertexT::AttributeIndices &attribute_indices) const
//     {
//         vertex_buffer.store(vertices);
//         VertexT::setVertexAttribArray(attribute_indices);
//
//         index_buffer.store(indices);
//     }
// };