#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 projection;
    vec3 viewPosition;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTextureCoordinates;
layout(location = 3) in vec3 inNormal;

layout(location = 0) out vec3 fragmentColor;
layout(location = 1) out vec3 fragmentPosition;
layout(location = 2) out vec2 fragmentTextureCoordinates;
layout(location = 3) out vec3 normal;
layout(location = 4) out vec3 viewPosition;
layout(location = 5) out vec3 vertPosition;


void main() {
    gl_Position = ubo.projection * ubo.view * ubo.model * vec4(inPosition, 1.f);
    fragmentColor = inColor;
    fragmentPosition = vec3(ubo.model * vec4(inPosition, 1.f));
    fragmentTextureCoordinates = inTextureCoordinates;
    normal = inNormal;
    vec4 vertPosition4 = ubo.view * ubo.model * vec4(inPosition, 1.f);
    vertPosition = vec3(vertPosition4) / vertPosition4.w;
}
