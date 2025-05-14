// Based on: https://github.com/mmerchante/organic-mesh-growth

[[vk::binding(0, 0)]]
cbuffer Model { uint3 gridDimension; uint64_t time; }

[[vk::binding(1, 0)]]
RWTexture3D<float> sdf;

float smin( float a, float b, float k )
{
    k *= 1.0;
    float r = exp2(-a/k) + exp2(-b/k);
    return -k*log2(r);
}

[shader("compute")]
[numthreads(4, 4, 4)]
void main(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    const float3 origin = float3(dispatchThreadID);
    float3 p = origin / float3(gridDimension);

    float  r = .15;
    float3 s = float3(r);
    float3 q = p - (s*(round(p/s)));
    float  d = length( q ) - r * .5;
    sdf[dispatchThreadID] = d;
}
