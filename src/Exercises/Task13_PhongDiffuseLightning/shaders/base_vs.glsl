#version 410

//out vec3 color;
out vec2 TexCoord;

layout(location=0) in  vec4 a_vertex_position;
//layout(location=1) in vec3 a_color;
layout (location = 1) in vec2 aTexCoord;
layout(location=2) in  vec3 a_vertex_normal;

layout(std140) uniform Matrixes {
    mat4 PVM;
};

void main() {
    gl_Position = PVM * a_vertex_position;
    //color = a_color;
    TexCoord = aTexCoord;
}
