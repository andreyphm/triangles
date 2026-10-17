#version 330 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 vertex_color;

uniform mat4 view;
uniform mat4 projection;

out vec3 point_position;
flat out vec3 triangle_color;

void main() {
    vec4 view_position = view * vec4(position, 1.0);
    gl_Position = projection * view_position;
    point_position = view_position.xyz;
    triangle_color = vertex_color;
}
