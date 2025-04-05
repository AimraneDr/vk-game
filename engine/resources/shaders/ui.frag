#version 450

layout(binding = 1) uniform UniformBufferObject {
    vec4 color;
    vec4 borderColor;
    vec2 size;
    float borderWidth;
} ubo;
layout(binding = 2) uniform sampler2D texSampler;

layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;


void main() {
    // outColor = ubo.borderColor;
    outColor = ubo.color;
}