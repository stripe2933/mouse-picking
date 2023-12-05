#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in mat4 aInstanceModel; // occupy 3~6

out VS_OUT{
    vec3 fragPos;
    vec3 normal;
    vec2 texCoords;
} vs_out;
flat out int instanceId;

layout (std140) uniform VpMatrix{
    mat4 projection_view;
    vec3 view_pos;
};

void main() {
    vs_out.fragPos = vec3(aInstanceModel * vec4(aPos, 1.0));
    gl_Position = projection_view * vec4(vs_out.fragPos, 1.0);
    vs_out.normal = mat3(transpose(inverse(aInstanceModel))) * aNormal;
    vs_out.texCoords = aTexCoords;
    instanceId = gl_InstanceID;
}
