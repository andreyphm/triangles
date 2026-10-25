#version 330 core
in vec3 point_position;
flat in vec3 triangle_color;

out vec4 color;

const float ambient_strength = 0.65;
const float diffuse_strength = 0.35;

void main() {
    vec3 normal = normalize(cross(dFdx(point_position), dFdy(point_position)));
    vec3 to_eye = normalize(-point_position);
    float diffuse = abs(dot(normal, to_eye));
    color = vec4(triangle_color * (ambient_strength + diffuse_strength * diffuse), 1.0);
}
