#version 450
#extension GL_EXT_nonuniform_qualifier : enable

layout(binding = 1) uniform Material {
    uint albedo_idx;
    uint normal_idx;
    uint metalRoughAO_idx;
    uint emissive_idx;
    uint height_idx;

    float metallicFactor;
    float roughnessFactor;
    float aoFactor;
    float heightScale;
    vec2 uvTiling;
    vec2 uvOffset;
    vec4 albedoFactor;
    vec4 emissiveFactor;
} material;

layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

// [0] albedo;
// [1] normal;
// [2] metalicRoughAO;
// [3] emmision;
// [4] height;
layout(binding = 2) uniform sampler2D samplers[];

void main() {
    // outColor = vec4(1.f);
    outColor = texture(
        samplers[nonuniformEXT(material.albedo_idx)], 
        fragTexCoord * material.uvTiling
    );
}