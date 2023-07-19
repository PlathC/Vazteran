#version 450
#extension GL_ARB_separate_shader_objects: enable
#extension GL_GOOGLE_include_directive: require

layout (binding = 0) uniform sampler2D samplerPosition;
layout (binding = 1) uniform sampler2D samplerNormal;

layout (location = 0) in vec2 inUV;

layout (location = 0) out vec4 outFragcolor;

const vec3 LightIntensity = vec3(.5);
const float Pi = 3.14159265;

void main()
{
    const vec4 vsPosition = texture(samplerPosition, inUV);
    const vec4 normal = texture(samplerNormal, inUV);
    const float shininess = normal.w;

    const vec3 lightDir = normalize(-vsPosition.xyz);
    const vec3 viewDir = lightDir;

    const float cosTheta = max(dot(lightDir, normal.xyz), 0.0f);
    const vec3 halfWay = normalize(viewDir + lightDir);
    const float specular = pow(max(dot(normal.xyz, halfWay), 0.0f), shininess);

    outFragcolor = vec4(LightIntensity * (cosTheta + specular), 1.0f);
}
