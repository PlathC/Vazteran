struct DrawCommand
{
    uint32_t indexCount;
    uint32_t instanceCount;
    uint32_t firstIndex;
    int32_t  vertexOffset;
    uint32_t firstInstance;
};

struct Uniforms
{
    uint32_t maxInstanceCount;
    uint32_t time;
};

[[vk::binding(0, 0)]]
ConstantBuffer<Uniforms> uniforms;

[[vk::binding(1, 0)]]
RWStructuredBuffer<float4> instances;

[[vk::binding(2, 0)]]
RWStructuredBuffer<DrawCommand> draw;

// https://iquilezles.org/articles/functions/
float gain( float x, float k )
{
    float a = 0.5*pow(2.0*((x<0.5)?x:1.0-x), k);
    return (x<0.5)?a:1.0-a;
}

// From: https://en.wikipedia.org/wiki/Torus_knot
float3 TorusKnot(float phi, float p, float q) {
    const float r = cos(q * phi) + 2.;
    const float x = r * cos(p * phi);
    const float y = -sin(q * phi);
    const float z = r * sin(p * phi);
    return float3(x, y, z);
}

const static float Pi = 3.1415926535897932384626;

[numthreads(256, 1, 1)]
void main(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    const uint id   = dispatchThreadID.x;
    const float phi = (float(id) / float(uniforms.maxInstanceCount)) * 2. * Pi;
    float3 position = TorusKnot(phi, 56.f, 15.f) * 3.;

    // Deform torus
    const float time = float(uniforms.time);
    position        += .3 * sin(position.x * position.z * gain(saturate(cos(time * 0.001 * 2.)), 1.f)).xxx;

    const float tt = .5 + .5f * gain(sin(time * 0.0001), 2.f);
    const uint32_t maxId = uint32_t( tt * float(uniforms.maxInstanceCount) );

    if(id == 0)
        draw[0].instanceCount = maxId;

    if( id < maxId )
        instances[id] = float4(position, 1.);
}
