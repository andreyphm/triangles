#version 330 core
flat in vec3 triangle_color;
out vec4 color;

void main() {
    color = vec4(triangle_color, 1.0);
}
