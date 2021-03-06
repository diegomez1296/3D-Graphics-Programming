#version 410

//in vec3 color;
in vec2 TexCoord;
layout(location=0) out vec4 vFragColor;

layout(std140) uniform Modifiers {
    float  strength;
    vec3   light;
};

uniform sampler2D diffuse_map;

void main() {
    vFragColor = texture(diffuse_map, TexCoord);
}
