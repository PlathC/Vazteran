#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform Material {
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;

    float shininess;
} material;

layout(binding = 2) uniform sampler2D colorMap;
layout(binding = 3) uniform sampler2D objectDiffuseMap;
layout(binding = 4) uniform sampler2D objectSpecularMap;

layout(location = 0) in vec3 vsPosition;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec3 normal;

layout (location = 0) out vec4 outPosition;
layout (location = 1) out vec4 outNormal;
layout (location = 2) out vec4 outAlbedo;

void main() 
{
    outPosition.xyz = vsPosition;
    outAlbedo = texture(objectDiffuseMap, uv);
    outNormal.xyz = normal;
}
