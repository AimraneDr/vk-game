#version 450
#extension GL_EXT_nonuniform_qualifier : enable

layout(binding = 1) uniform UniformBufferObject {
    vec4 color;
    vec4 borderColor;
    vec2 size;
    float borderWidth;
    uint FontID;
    uint is_text;
} ubo;
layout(binding = 2) uniform sampler2D samplers[];

layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;


void main() {
    // outColor = ubo.borderColor;
    if(ubo.is_text==0){
        outColor = ubo.color;
    }else{
        outColor = ubo.color * texture(samplers[nonuniformEXT(ubo.FontID)], fragTexCoord).r;
    }
}