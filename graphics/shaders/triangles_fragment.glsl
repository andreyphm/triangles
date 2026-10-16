#version 330 core
in vec3 point_position;
flat in vec3 triangle_color;
out vec4 color;

const float ambient_strength = 0.25;
const float diffuse_strength = 0.75;
const vec3 view_axis = vec3(0.0, 0.0, 1.0);

void main() {
    vec3 normal = normalize(cross(dFdx(point_position), dFdy(point_position)));
    float diffuse = abs(dot(normal, view_axis));
    color = vec4(triangle_color * (ambient_strength + diffuse_strength * diffuse), 1.0);
}
