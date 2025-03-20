#version 450

layout(binding = 1) uniform UniformBufferObject {
    vec4 color;
    vec2 size;
} ubo;
layout(binding = 2) uniform sampler2D texSampler;

layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;


void main() {
    outColor = ubo.color;
    // outColor = texture(texSampler, fragTexCoord);
}