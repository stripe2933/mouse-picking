#version 330 core

flat in uint instanceId;

out vec4 FragColor;

uniform vec3 color = vec3(1.0, 0.5, 0.2);
uniform usampler2D instance_id_texture;

void main() {
    vec2 texcoord = gl_FragCoord.xy / textureSize(instance_id_texture, 0);
    if (instanceId == texture(instance_id_texture, texcoord).r){
        discard;
    }

    FragColor = vec4(color, 1.0);
}
