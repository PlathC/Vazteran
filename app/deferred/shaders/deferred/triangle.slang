cbuffer Model
{
    float4x4 modelViewMatrix;
    float4x4 projectionMatrix;
    float4x4 normalMatrix;
}

[[vk::binding(1, 0)]]
StructuredBuffer<float4> instances;

struct Vertex
{
    [[vk::location(0)]]
    float3 position : POSITION;
    [[vk::location(1)]]
    float3 normal : NORMAL;
};

struct VertexStageOutput
{
    float4 position : SV_Position;
    float3 vsPosition : POSITIONT;
    float3 normal : NORMAL;
};

[shader("vertex")]
VertexStageOutput vertexMain(Vertex vertex, uint instanceIndex: SV_InstanceID)
{
    const float3 position = vertex.position + instances[instanceIndex].xyz;
    const float4 viewSpacePosition = mul(transpose(modelViewMatrix), float4(position, 1.0));

    VertexStageOutput output;
    output.vsPosition = viewSpacePosition.xyz;
    output.position   = mul(transpose(projectionMatrix), viewSpacePosition);
    output.normal     = normalize(mul(transpose(normalMatrix), float4(vertex.normal, 0.0f)).xyz);

    return output;
}

[shader("fragment")]
float4 fragmentMain(float3 vsPosition: POSITIONT, float3 normal: NORMAL) : SV_Target
{
    return float4(dot(normal, -normalize(vsPosition)).xxx, 1.);
}
