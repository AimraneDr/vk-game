#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 proj;
} ubo;

layout(push_constant) uniform PushConstant{
    mat4 model;
}pc;

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec2 inCoord;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec2 fragTexCoord;

void main() {
    gl_Position = ubo.proj * pc.model * vec4(inPosition, 1.0, 1.0);
    fragColor = vec4(1.f);
    fragTexCoord = inCoord;
}