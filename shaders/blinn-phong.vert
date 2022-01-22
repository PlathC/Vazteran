#version 450
#extension GL_ARB_separate_shader_objects : enable

layout( binding = 0 ) uniform Model {
    mat4 modelViewMatrix;
    mat4 projectionMatrix;
    mat4 normalMatrix;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTextureCoordinates;
layout(location = 2) in vec3 inNormal;

layout(location = 0) out vec3 vsPosition;
layout(location = 1) out vec2 uv;
layout(location = 2) out vec3 normal;


void main() {
    uv     = inTextureCoordinates;
    normal = normalize((ubo.normalMatrix * vec4(inNormal, 1.0f)).xyz);

    const vec4 viewSpacePosition = ubo.modelViewMatrix * vec4(inPosition, 1.0f);

    vsPosition  = viewSpacePosition.xyz;
    gl_Position = ubo.projectionMatrix * viewSpacePosition;
}
