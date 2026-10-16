#version 330 core
uniform vec3 fill_color;
out vec4 color;

void main() {
    color = vec4(fill_color, 1.0);
}
