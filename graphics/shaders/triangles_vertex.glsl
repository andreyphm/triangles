#version 330 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 vertex_color;

out vec3 point_position;
flat out vec3 triangle_color;

void main() {
    gl_Position = vec4(position, 1.0);
    point_position = position;
    triangle_color = vertex_color;
}
