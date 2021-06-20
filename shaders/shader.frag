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

    vec3 N = normalize(normal);
    vec3 L = normalize(lightPosition - vertPosition);
    vec3 V = normalize(-vertPosition);

    // Compute half way vector between viewer and light-source
    vec3 halfWay = normalize(V + L);
    float sdn = dot(L, N);

    vec3 ambient = material.ambient.xyz * texture(objectAmbientMap, fragmentTextureCoordinates).xyz;
    vec3 diffuse = material.diffuse.xyz * max(sdn, 0.0) * texture(objectDiffuseMap, fragmentTextureCoordinates).xyz;
    vec3 specular = material.specular.xyz * mix( 0.0, pow(dot(N, halfWay), material.shininess), step( 0.0, sdn))
                    * texture(objectSpecularMap, fragmentTextureCoordinates).xyz;

    outColor = vec4(lightIntensity * (ambient + diffuse + specular), 1.);
}
