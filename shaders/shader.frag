#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform Material {
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;

    float shininess;
} material;

layout(binding = 1) uniform sampler2D objectAmbientMap;
layout(binding = 2) uniform sampler2D objectDiffuseMap;
layout(binding = 3) uniform sampler2D objectSpecularMap;

layout(location = 0) in vec3 fragmentPosition;
layout(location = 1) in vec2 fragmentTextureCoordinates;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec3 viewPosition;
layout(location = 4) in vec3 vertPosition;

layout(location = 0) out vec4 outColor;

void main() {
    const vec3 lightPosition = vec3(1.0f,  1.0f, 1.0f);
    const vec3 lightIntensity = vec3( 1.0f,  1.0f, 1.0f);
    
    // Blinn-Phong model: https://en.wikipedia.org/wiki/Blinn%E2%80%93Phong_reflection_model
    vec3 lightDirection = normalize(lightPosition - vertPosition);
    vec3 viewerDirection = normalize(-vertPosition);

    vec3 halfWay = normalize(viewerDirection + lightDirection);
    float lightDotNormal = max(dot(lightDirection, normal), 0.0f);
    
    vec3 ambient = material.ambient.xyz * texture(objectAmbientMap, fragmentTextureCoordinates).xyz;
    vec3 diffuse = material.diffuse.xyz * lightDotNormal * texture(objectDiffuseMap, fragmentTextureCoordinates).xyz;
    
    float specularAngle = max(dot(normal, halfWay), 0.0f);
    float scaledSpecular = pow(specularAngle, material.shininess);
    vec3 specular = material.specular.xyz * scaledSpecular * texture(objectSpecularMap, fragmentTextureCoordinates).xyz;

    outColor = vec4(lightIntensity * (ambient + diffuse + specular), 1.0f);
}
