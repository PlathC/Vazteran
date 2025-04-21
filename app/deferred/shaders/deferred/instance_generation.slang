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

bool isVisible(int id) { return id < abs(sin(float(uniforms.time) * 0.0005)) * float(uniforms.maxInstanceCount); }

[numthreads(256, 1, 1)]
void main(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    const uint id = dispatchThreadID.x;

    const int    range    = int(sqrt(float(uniforms.maxInstanceCount)));
    const int    x        = id % range - range / 2;
    const int    y        = id / range - range / 2;
    const float3 position = float3(x * 1.5f, y * 1.5f, sin(float(uniforms.time) * 1e-3f + id) * 5e-1f) * 1.75f;

    if (isVisible(id))
    {
        uint writeId;
        InterlockedAdd(draw[0].instanceCount, 1u, writeId);
        instances[writeId] = float4(position, 1.);
    }
}
