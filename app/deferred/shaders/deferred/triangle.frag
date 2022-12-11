#version 450
#extension GL_ARB_separate_shader_objects  : enable

layout(location = 0) in vec3 vVsPosition;
layout(location = 1) in vec3 vNormal;

layout(location = 0) out vec4 vsPosition;
layout(location = 1) out vec4 normal;

void main() 
{
    vsPosition = vec4(vVsPosition, 1.);
    normal     = vec4(vNormal, 90.);
}
