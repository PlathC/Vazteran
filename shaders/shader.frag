#version 450
#extension GL_ARB_separate_shader_objects : enable

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
    vec3 N = normalize(normal);

    const vec3 lightPosition = vec3(2.f, 2.f, 2.f);
    vec3 L = normalize(lightPosition - vertPosition);

    // Lambert's cosine law
    float lambertian = max(dot(N, L), 0.0);
    float specular = 0.0;
    if(lambertian > 0.0) {
        vec3 R = reflect(-L, N);      // Reflected light vector
        vec3 V = normalize(-vertPosition); // Vector to viewer
        // Compute the specular term
        float specAngle = max(dot(R, V), 0.0);
        specular = pow(specAngle, 75.);
    }
    outColor = 1.f * texture(objectAmbient, fragmentTextureCoordinates) +
               1.f * lambertian * texture(objectDiffuse, fragmentTextureCoordinates) +
               1.f * specular * texture(objectDiffuse, fragmentTextureCoordinates) ;
/*
    const vec3 lightColor = vec3(0.4f, 0.4f, 0.4f);
    vec3 ambient = texture(objectAmbient, fragmentTextureCoordinates).xyz * lightColor;

    vec3 unitNormal = normalize(normal);
    vec3 lightDir = normalize(lightPosition - fragmentPosition);
    float lightImpact = max(dot(unitNormal, lightDir), 0.0);

    vec3 viewDir = normalize(viewPosition - fragmentPosition);
    vec3 reflectDir = reflect(-lightDir, unitNormal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 225);
    vec3 specular = texture(objectSpecular, fragmentTextureCoordinates).xyz * spec * lightColor;

    vec3 diffuse = texture(objectDiffuse, fragmentTextureCoordinates).xyz * lightImpact * lightColor;
    vec3 result =  (ambient + diffuse) * fragmentColor;

    outColor = vec4(result, 1.f);*/
}