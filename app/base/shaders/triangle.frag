#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : require

layout(binding = 1) uniform Material {
    vec4 albedo; // + shininess
} material;

layout(binding = 2) uniform sampler2D colorMap;

layout(location = 0) in vec3 vsPosition;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec3 normal;

layout (location = 0) out vec4 outColor;

void main() 
{
    outColor = vec4(material.albedo.rgb, 1.);
}
