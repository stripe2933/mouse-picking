#version 330 core

in VS_OUT{
    vec3 fragPos;
    vec3 normal;
    vec2 texCoords;
} fs_in;

out vec4 FragColor;

uniform struct DirectionalLight{
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
} light = DirectionalLight(vec3(-1.0), vec3(0.4), vec3(0.8), vec3(0.5));

uniform struct VpMatrix{
    vec3 view_pos;
    mat4 projection_view;
} vp_matrix;

uniform sampler2D diffuse_map;
uniform sampler2D specular_map;

void main() {
    vec3 ambient = light.ambient * vec3(texture(diffuse_map, fs_in.texCoords));

    vec3 norm = normalize(fs_in.normal);
    vec3 lightDir = normalize(-light.direction);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * vec3(texture(diffuse_map, fs_in.texCoords));

    vec3 viewDir = normalize(vp_matrix.view_pos - fs_in.fragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(norm, halfwayDir), 0.0), 32.0);
    vec3 specular = light.specular * spec * vec3(texture(specular_map, fs_in.texCoords));

    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
}
