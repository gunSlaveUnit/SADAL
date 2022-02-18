#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;

layout(location = 0) out vec3 fragColor;

layout(binding = 0) uniform Transformation {
    mat4 model;
    mat4 view;
    mat4 projection;
} transform;

void main() {
    gl_Position = transform.projection * transform.view * transform.model *  vec4(position, 1.0);
    fragColor = color;
}
