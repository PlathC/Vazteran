// Based on: https://github.com/mmerchante/organic-mesh-growth

[[vk::binding(0, 0)]]
cbuffer Model { uint3 gridDimension; }

[[vk::binding(1, 0)]]
RWTexture3D<float> sdf;

[shader("compute")]
[numthreads(8, 8, 8)]
void main(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    const float3 point    = float3(dispatchThreadID) / float3(gridDimension);
    sdf[dispatchThreadID] = length(point - .5) - .5;
}
