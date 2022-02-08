#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (binding = 0) uniform sampler2D samplerPosition;
layout (binding = 1) uniform sampler2D samplerNormal;
layout (binding = 2) uniform sampler2D samplerAlbedo;

layout (location = 0) in vec2 inUV;

layout (location = 0) out vec4 outFragcolor;

const vec3  LightPosition  = vec3( 1.0f, 1.0f, 1.0f );
const vec3  LightIntensity = vec3( 1.0f, 1.0f, 1.0f );
const float SpecularStrength = 0.5;

void main() {
    const vec3 vertPosition = texture(samplerPosition, inUV).rgb;
    const vec3 normal       = texture(samplerNormal, inUV).rgb;
    const vec4 albedo       = texture(samplerAlbedo, inUV);

    // Blinn-Phong model: https://en.wikipedia.org/wiki/Blinn%E2%80%93Phong_reflection_model
    const vec3 lightDirection  = normalize( LightPosition - vertPosition );
    const vec3 viewerDirection = normalize( vertPosition );

    const float cosTheta = max(dot(lightDirection, normal), 0.0f);
    
    const vec3 halfWay   = normalize(viewerDirection + lightDirection);
    const float specular = SpecularStrength * pow(max(dot(normal, halfWay), 0.0f), 150.f);

    outFragcolor = vec4(albedo.rgb * LightIntensity * ( cosTheta + specular ), 1.0f);
}
