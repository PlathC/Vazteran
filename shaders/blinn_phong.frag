#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "helpers/compression.glsl"

layout (binding = 0) uniform sampler2D samplerPosition;
layout (binding = 1) uniform sampler2D samplerNormal;
layout (binding = 2) uniform sampler2D samplerAlbedo;

layout (location = 0) in vec2 inUV;
layout (location = 0) out vec4 outFragcolor;

const vec3  LightPosition  = vec3( 1.0f, 1.0f, 1.0f );
const vec3  LightIntensity = vec3( 1.0f, 1.0f, 1.0f );
const float Pi             = 3.14159265;


void main() {
    const vec4  fragPosition = texture(samplerPosition, inUV);
    const vec3  normal       = decodeNormal(texture(samplerNormal, inUV).xy);
    const vec4  albedo       = texture(samplerAlbedo, inUV);
    const float shininess    = fragPosition.w;

    const vec3 lightDir = normalize( LightPosition - fragPosition.xyz );
    const vec3 viewDir  = normalize( -fragPosition.xyz );

    const float cosTheta = max(dot(lightDir, normal), 0.0f);
    const vec3  halfWay  = normalize(viewDir + lightDir);
    const float specular = pow(max(dot(normal, halfWay), 0.0f), shininess);

    outFragcolor = vec4(albedo.rgb * LightIntensity * ( cosTheta + specular ), 1.0f);
}
