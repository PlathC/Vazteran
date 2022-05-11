#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "helpers/compression.glsl"

layout(binding = 1) uniform Material {
    vec4 color; // + shininess
} material;

layout(binding = 2) uniform sampler2D colorMap;

layout(location = 0) in vec3 vsPosition;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec3 normal;

layout (location = 0) out vec4 outPosition;
layout (location = 1) out vec2 outNormal;
layout (location = 2) out vec4 outAlbedo;


void main() 
{
    outPosition = vec4(vsPosition, material.color.w);
    outAlbedo = vec4(texture(colorMap, uv).rgb * material.color.rgb, 1.);
    outNormal.xy = encodeNormal(normal);
}
