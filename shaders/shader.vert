#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(push_constant) uniform Model {
    mat4 model;
    mat4 view;
    mat4 projection;
    vec3 viewPosition;
} model;
 
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTextureCoordinates;
layout(location = 3) in vec3 inNormal;

layout(location = 0) out vec3 fragmentColor;
layout(location = 1) out vec3 fragmentPosition;
layout(location = 2) out vec2 fragmentTextureCoordinates;
layout(location = 3) out vec3 normal;
layout(location = 5) out vec3 vertPosition;


void main() {
    gl_Position = model.projection * model.view * model.model * vec4(inPosition, 1.0f);
    fragmentColor = inColor;
    fragmentPosition = vec3(model.model * vec4(inPosition, 1.f));
    fragmentTextureCoordinates = inTextureCoordinates;

    mat4 modelViewMatrix = model.view * model.model;
    mat4 normalMatrix = transpose(inverse(modelViewMatrix));
    normal = normalize((normalMatrix * vec4(inNormal, 0.0f)).xyz);

    vec4 vertPosition4 = modelViewMatrix * vec4(inPosition, 1.0f);
    vertPosition = vec3(vertPosition4) / vertPosition4.w;
}
