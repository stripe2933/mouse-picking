#version 330 core

out vec2 texCoord;

void main() {
    float x = (gl_VertexID % 2 == 0) ? 0.0 : 1.0;
    float y = (gl_VertexID / 2 == 0) ? 0.0 : 1.0;
    texCoord = vec2(x, y);
    gl_Position = vec4(mix(vec2(-1.0), vec2(1.0), texCoord), 0.0, 1.0);
}
