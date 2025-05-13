// From: https://github.com/sebbbi/rust_test/blob/main/shader/main.vert
// From: https://github.com/sebbbi/rust_test/blob/main/shader/main.frag
[[vk::binding(0, 0)]]
cbuffer Uniforms
{
    float4x4 worldToScreen;
    float4   cameraPosition;
    float4   texelScale;
}

[[vk::combinedImageSampler]][[vk::binding(1, 0)]]
Texture3D<float> sdf;
[[vk::combinedImageSampler]][[vk::binding(1, 0)]]
SamplerState samplerSdf;

struct VertexStageOutput
{
    float4 position : SV_Position;
    float3 uvw;
    float4 localCameraPosLod;
    float3 localPos;
};

const static float Radius = .05;

[shader("vertex")]
VertexStageOutput vertexMain(uint32_t vx: SV_VertexID, uint32_t instanceIndex: SV_InstanceID)
{
    uint3  xyz = uint3(vx & 0x1, (vx & 0x4) >> 2, (vx & 0x2) >> 1);
    float3 uvw = float3(xyz);
    float3 pos = uvw * 2.0 - 1.0;

    float3 position = { 0., 0., 0. };

    float3 localPos       = pos.xyz;
    float3 localCameraPos = cameraPosition.xyz - position;

    float  lod           = 0.5 * log2(dot(localCameraPos, localCameraPos)) - 6.0;
    float3 texelScaleLod = texelScale.xyz * exp2(clamp(lod, 0.0, 5.0));

    VertexStageOutput output = {};
    output.position          = mul(worldToScreen, float4(localPos + position, 1.0));
    output.uvw               = uvw * (float3(1.) - texelScaleLod) + texelScaleLod * 0.5;
    output.localCameraPosLod = float4(localCameraPos, lod);
    output.localPos          = localPos;

    return output;
}

float3 palette(in float t, in float3 a, in float3 b, in float3 c, in float3 d)
{
    return a + b * cos(6.283185 * (c * t + d));
}

float3 idToColor(int id, int maximum)
{
    return palette(float(id) / float(maximum), float3(.5), float3(.5), float3(1.), float3(0.00, 0.10, 0.20));
}

float3 normal(VertexStageOutput output, float3 uvw)
{
    const float  lod = output.localCameraPosLod.w;
    const float3 e   = texelScale.xyz * 0.5;

    const float xm = sdf.Sample(samplerSdf, uvw + float3(-e.x, 0, 0), 0, lod);
    const float xp = sdf.Sample(samplerSdf, uvw + float3(+e.x, 0, 0), 0, lod);
    const float ym = sdf.Sample(samplerSdf, uvw + float3(0, -e.y, 0), 0, lod);
    const float yp = sdf.Sample(samplerSdf, uvw + float3(0, +e.y, 0), 0, lod);
    const float zm = sdf.Sample(samplerSdf, uvw + float3(0, 0, -e.z), 0, lod);
    const float zp = sdf.Sample(samplerSdf, uvw + float3(0, 0, +e.z), 0, lod);
    return normalize(float3(xp - xm, yp - ym, zp - zm));
}

bool outside(float3 uwv) { return any(abs(uwv - float3(0.5, 0.5, 0.5)) > float3(0.5, 0.5, 0.5)); }

[shader("fragment")]
float4 fragmentMain(VertexStageOutput output)
{
    float3 rayPos = output.uvw;
    float3 rayDir = normalize(output.localPos - output.localCameraPosLod.xyz);

    rayDir *= 1.; // ubo.volume_scale.xyz;

    // Do the first step without the outside check
    // Rasterization does not guarantee perfect interpolation
    float s = sdf.Sample(samplerSdf, rayPos, 0, output.localCameraPosLod.w);
    s       = s * 2.0 - 1.0;

    float d = s;
    if (s > 0.00025)
    {
        for (uint i = 0; i < 1024; ++i)
        {
            float3 uvw = rayPos + rayDir * d;
            if (outside(uvw))
            {
                discard;
                break;
            }

            float s = sdf.Sample(samplerSdf, uvw, 0, output.localCameraPosLod.w).x;
            s       = s * 2.0 - 1.0;

            d += s;
            if (s < 0.00025)
                break;
        }
    }
    return float4(normal(output, rayPos + rayDir * d), 1.0);
}
