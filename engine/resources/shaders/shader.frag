#version 450
layout(binding = 1) uniform Material {
    float metallicFactor;
    float roughnessFactor;
    float aoFactor;
    float heightScale;
    vec2 uvTiling;
    vec2 uvOffset;
    vec4 albedoFactor;
    vec3 emissiveFactor;
} material;

layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

layout(binding = 2) uniform sampler2D albedo;
layout(binding = 3) uniform sampler2D normal;
layout(binding = 4) uniform sampler2D metalicRoughAO;
layout(binding = 5) uniform sampler2D emmision;
layout(binding = 6) uniform sampler2D height;

void main() {
    // outColor = vec4(1.f);
    outColor = texture(albedo, fragTexCoord * material.uvTiling);
}