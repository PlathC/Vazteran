#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform ObjectData {
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;

    mat4 model;
    mat4 view;
    mat4 projection;
    vec3 viewPosition;

    float shininess;
} objectData;

layout(binding = 1) uniform sampler2D objectAmbient;
layout(binding = 2) uniform sampler2D objectDiffuse;
layout(binding = 3) uniform sampler2D objectSpecular;

layout(location = 0) in vec3 fragmentColor;
layout(location = 1) in vec3 fragmentPosition;
layout(location = 2) in vec2 fragmentTextureCoordinates;
layout(location = 3) in vec3 normal;
layout(location = 4) in vec3 viewPosition;
layout(location = 5) in vec3 vertPosition;

layout(location = 0) out vec4 outColor;

void main() {

    const vec3 lightPosition = vec3(0.f, -2.f, 0.f);
    const vec3 lightIntensity = vec3(0.4f, 0.4f, 0.4f);

    vec3 N = normalize(normal);
    vec3 L = normalize(lightPosition - vertPosition);
    vec3 V = normalize(-vertPosition);
    vec3 H = normalize(V + L);
    float sdn = dot(L, N);
    vec3 ambient = objectData.ambient.xyz * texture(objectAmbient, fragmentTextureCoordinates).xyz;
    vec3 diffuse = objectData.diffuse.xyz * max(sdn, 0.0) * texture(objectDiffuse, fragmentTextureCoordinates).xyz;
    vec3 specular = objectData.specular.xyz * mix( 0.0, pow( dot(H, N), objectData.shininess), step( 0.0, sdn))
                    * texture(objectSpecular, fragmentTextureCoordinates).xyz;

    outColor = vec4(lightIntensity * (ambient + diffuse + specular), 1);
}
