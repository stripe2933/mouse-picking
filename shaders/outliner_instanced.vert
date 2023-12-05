#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

flat out uint instanceId;

uniform uint instance_id;
uniform mat4 model;
layout (std140) uniform VpMatrix{
    mat4 projection_view;
    vec3 view_pos;
};

void main() {
    gl_Position = projection_view * model * vec4(aPos, 1.0);
    instanceId = instance_id;
}
