#version 410

in vec3 vertex_color;
layout(location=0) out vec3 vFragColor;

void main() {
    vFragColor = vertex_color;
}
