//
// Created by gomkyung2 on 11/27/23.
//

#pragma once

#include <GL/gl3w.h>

#include <OGLWrapper/Texture.hpp>

struct Material {
    OGLWrapper::Texture<GL_TEXTURE_2D> diffuse_map;
    OGLWrapper::Texture<GL_TEXTURE_2D> specular_map;

    void setTexture(GLenum diffuse_unit, GLenum specular_unit) const {
        // TODO: texture unit should be GL_TEXTURE0 ~ GL_TEXTURE15

        glActiveTexture(diffuse_unit);
        diffuse_map.bind();
        glActiveTexture(specular_unit);
        specular_map.bind();
    }
};
