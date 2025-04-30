[[vk::binding(0, 0)]]
cbuffer Model
{
    float4x4 modelViewMatrix;
    float4x4 projectionMatrix;
    float4x4 normalMatrix;
}

[[vk::binding(1, 0)]]
StructuredBuffer<float4> instances;

struct VertexStageOutput
{
    float4 position   : SV_Position;
    float3 vsCenter   : POSITIONT;
    float3 vsPosition : POSITIONT;
    float3 normal     : NORMAL;
    uint32_t id;
};

const static float Radius = .05;

[shader("vertex")]
VertexStageOutput vertexMain(uint32_t vertexId : SV_VertexID, uint32_t instanceIndex : SV_InstanceID)
{
    const float3 position = instances[instanceIndex].xyz;
    const float4 viewSpacePosition = mul(transpose(modelViewMatrix), float4(position, 1.0));

    const float3 u = normalize(cross(float3(0., 0., 1.), viewSpacePosition.xyz));
    const float3 v = normalize(cross(u, viewSpacePosition.xyz));

    // 1-5-----3
    // | \     |
    // |   \   |
    // |     \ |
    // 0------2-4
    const int vId = vertexId % 6;

    // 1-2-----0
    // | \     |
    // |   \   |
    // |     \ |
    // 0------2-1
    const int triangleVertexId = vId % 3;

    // 0-0-----1
    // | \     |
    // |   \   |
    // |     \ |
    // 0------1-1
    const int xshift = abs((vId / 3) - int(triangleVertexId == 2));
    const float2 shift = float2(xshift == 0 ? -1. : 1., vId % 2 == 0 ? -1. : 1.);

    VertexStageOutput output;
    output.vsPosition = viewSpacePosition.xyz + u * shift.x * Radius + v * shift.y * Radius;
    output.vsCenter   = viewSpacePosition.xyz;
    output.position   = mul(transpose(projectionMatrix), float4(output.vsPosition, 1.));
    output.normal     = normalize(-viewSpacePosition.xyz);// normalize(mul(transpose(normalMatrix), float4(vertex.normal, 0.0f)).xyz);

    output.id = instanceIndex;
    return output;
}

float3 palette( in float t, in float3 a, in float3 b, in float3 c, in float3 d )
{
    return a + b*cos( 6.283185*(c*t+d) );
}

float3 idToColor(int id, int maximum)
{
    return palette(float(id) / float(maximum), float3(.5), float3(.5), float3(1.), float3(0.00, 0.10, 0.20) );
}


[shader("fragment")]
float4 fragmentMain(VertexStageOutput output)
{
    float d = length(output.vsCenter - output.vsPosition);
     if(d > Radius)
         discard;

    const float delta = fwidth( d );
    const float t     = lerp( Radius - delta, Radius, d );

    const float3 Background = float3(1.f, 0.91f, 0.69f);
    const float3 color = idToColor(output.id, 2 << 19);

    return float4(color, 1. - t);
}
