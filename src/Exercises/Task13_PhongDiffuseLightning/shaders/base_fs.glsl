#version 410

//in vec3 color;
in vec2 TexCoord;
in vec3 vertex_normal_in_vs;
in vec3 vertex_position_in_vs;

layout(std140) uniform Light {
    vec3 position_in_vs;
    vec3 color;
    vec3 a;
} light;

layout(location=0) out vec4 vFragColor;

uniform sampler2D diffuse_map;

void main() {
    vFragColor = texture(diffuse_map, TexCoord);

    vec3 normal = normalize(vertex_normal_in_vs);
    vec3 light_vector = normalize(light.position_in_vs - vertex_position_in_vs);
    float light_in = max(0.0,dot(normal, light_vector));

    vec4 diffuse_color = texture(diffuse_map, TexCoord);
    vFragColor.a = diffuse_color.a;
    vFragColor.rgb = light_in * diffuse_color.rgb*light.color;
}
