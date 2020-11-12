#version 410

out vec3 vertex_color;
layout(location=0) in  vec4 a_vertex_position;
layout(location=1) in vec3 a_vertex_color;

layout(std140) uniform Matrixes {
    mat4 pvm;
};

void main() {
    gl_Position = pvm * a_vertex_position;
    vertex_color=a_vertex_color;
}
