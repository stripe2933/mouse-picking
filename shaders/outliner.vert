#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

uniform mat4 model;

uniform struct VpMatrix{
    vec3 view_pos;
    mat4 projection_view;
} vp_matrix;

void main() {
    gl_Position = vp_matrix.projection_view * model * vec4(aPos, 1.0);
}
