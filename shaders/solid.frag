#version 330 core

in VS_OUT{
    vec3 fragPos;
    vec3 normal;
    vec2 texCoords;
} fs_in;

out vec4 FragColor;

uniform vec3 color = vec3(1.0, 0.5, 0.2);

void main() {
    FragColor = vec4(color, 1.0);
}
