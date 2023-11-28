//
// Created by gomkyung2 on 11/27/23.
//

#pragma once

#include <GL/gl3w.h>

struct Material {
    OGLWrapper::Texture diffuse_map;
    OGLWrapper::Texture specular_map;

    void setTexture(GLenum diffuse_unit, GLenum specular_unit) const {
        // TODO: texture unit should be GL_TEXTURE0 ~ GL_TEXTURE15

        glActiveTexture(diffuse_unit);
        glBindTexture(GL_TEXTURE_2D, diffuse_map.handle);
        glActiveTexture(specular_unit);
        glBindTexture(GL_TEXTURE_2D, specular_map.handle);
    }
};
