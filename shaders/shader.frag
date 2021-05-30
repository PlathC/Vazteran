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
        vec3 R = reflect(-L, N);           // Reflected light vector
        vec3 V = normalize(-vertPosition); // Vector to viewer

        // Compute the specular term
        float specAngle = max(dot(R, V), 0.0);
        specular = pow(specAngle, 75.);
    }

    outColor = 1.f * texture(objectAmbient, fragmentTextureCoordinates) +
                1.f * lambertian * texture(objectDiffuse, fragmentTextureCoordinates) +
                1.f * specular * texture(objectSpecular, fragmentTextureCoordinates);
}
