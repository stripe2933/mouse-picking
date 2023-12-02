#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out VS_OUT{
    vec3 fragPos;
    vec3 normal;
    vec2 texCoords;
} vs_out;

uniform mat4 model;
uniform mat4 inv_model;

uniform struct VpMatrix{
    vec3 view_pos;
    mat4 projection_view;
} vp_matrix;

void main() {
    vs_out.fragPos = aPos;
    gl_Position = vp_matrix.projection_view * model * vec4(vs_out.fragPos, 1.0);
    vs_out.normal = mat3(transpose(inv_model)) * aNormal;
    vs_out.texCoords = aTexCoords;
}
