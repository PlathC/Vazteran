#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform sampler2D textureSampler;

layout(location = 0) in vec3 fragmentColor;
layout(location = 1) in vec3 fragmentPosition;
layout(location = 2) in vec2 fragmentTextureCoordinates;
layout(location = 3) in vec3 normal;

layout(location = 0) out vec4 outColor;

void main() {
    const vec3 lightColor = vec3(1.f, 1.f, 1.f);
    const vec3 lightPosition = vec3(2.f, 0.f, 2.f);
    float ambientStrength = 0.1f;
    vec3 ambient = ambientStrength * lightColor;

    vec3 unitNormal = normalize(normal);
    vec3 lightDir = normalize(lightPosition - fragmentPosition);
    float lightImpact = max(dot(unitNormal, lightDir), 0.0);

    vec3 diffuse = lightImpact * lightColor;
    vec3 result =  (ambient + diffuse) * fragmentColor;

    outColor = vec4(result, 1.f);
}