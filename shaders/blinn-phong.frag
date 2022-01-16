#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform Material {
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;

    float shininess;
} material;

layout(binding = 2) uniform sampler2D objectAmbientMap;
layout(binding = 3) uniform sampler2D objectDiffuseMap;
layout(binding = 4) uniform sampler2D objectSpecularMap;

layout(location = 0) in vec3 vsPosition;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec3 normal;

layout(location = 0) out vec4 outColor;

void main() {
    const vec3 lightPosition  = vec3( 1.0f );
    const vec3 lightIntensity = vec3( 1.0f );
    
    vec3 lightDirection = normalize(lightPosition - vsPosition);
    vec3 viewerDirection = normalize(-vsPosition);

    vec3 halfWay = normalize(viewerDirection + lightDirection);
    float lightDotNormal = max(dot(lightDirection, normal), 0.0f);
    
    vec3 ambient = material.ambient.xyz * texture(objectAmbientMap, uv).xyz;
    vec3 diffuse = material.diffuse.xyz * lightDotNormal * texture(objectDiffuseMap, uv).xyz;
    
    float specularAngle  = max(dot(normal, halfWay), 0.0f);
    float scaledSpecular = pow(specularAngle, material.shininess);
    vec3 specular        = material.specular.xyz * texture(objectSpecularMap, uv).xyz * scaledSpecular;

    outColor = vec4(lightIntensity * (ambient + diffuse + specular), 1.0f);
}
