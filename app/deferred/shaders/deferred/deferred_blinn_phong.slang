// From:
// https://github.com/SaschaWillems/Vulkan/blob/74be818cc6b6fcab23932adbf245f624f4cf4ada/shaders/hlsl/deferredshadows/deferred.vert
struct VSOutput
{
    float4 Pos : SV_POSITION;
    [[vk::location(0)]]
    float2 UV : TEXCOORD0;
};

[shader("vertex")]
VSOutput vertexMain(uint VertexIndex: SV_VertexID)
{
    VSOutput output = (VSOutput)0;
    output.UV       = float2((VertexIndex << 1) & 2, VertexIndex & 2);
    output.Pos      = float4(output.UV * 2.0f - 1.0f, 0.0f, 1.0f);
    return output;
}

Texture2D    texturePosition : register(t0);
SamplerState samplerPosition : register(s0);
Texture2D    textureNormal : register(t1);
SamplerState samplerNormal : register(s1);

static const float3 LightIntensity = float3(.5);
static const float  Pi             = 3.14159265;

[shader("fragment")]
float4 fragmentMain([[vk::location(0)]] float2 inUV: TEXCOORD0) : SV_Target
{
    const float3 vsPosition = texturePosition.Sample(samplerPosition, inUV).rgb;
    const float4 normal     = textureNormal.Sample(samplerNormal, inUV);
    const float  shininess  = normal.w;

    const float3 Background = float3(1.f, 0.91f, 0.69f);
    if(dot(normal, float4(1.)) == 0.)
        return float4(Background, 1.f);

    const float3 lightDir = normalize(-vsPosition.xyz);
    const float3 viewDir  = lightDir;

    const float  cosTheta = max(dot(lightDir, normal.xyz), 0.0f);
    const float3 halfWay  = normalize(viewDir + lightDir);
    const float  specular = pow(max(dot(normal.xyz, halfWay), 0.0f), shininess);

    const float3 BackgroundStrength = .2;
    return float4(LightIntensity * (cosTheta + specular) + Background * BackgroundStrength, 1.0f);
}
