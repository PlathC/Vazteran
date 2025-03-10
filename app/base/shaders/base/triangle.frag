#version 450
#extension GL_ARB_separate_shader_objects  : enable

layout(binding = 1) uniform Material 
{
    vec4 albedo; // + shininess
} material;

layout(location = 0) in vec3 vsPosition;
layout(location = 1) in vec3 normal;

layout(location = 0) out vec4 outColor;

void main() 
{
    outColor = vec4(vec3(1.) * dot(normal, -normalize(vsPosition)), 1.);
}
