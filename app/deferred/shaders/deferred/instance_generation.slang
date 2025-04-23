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

bool isVisible(int id) {
    const float t = gain(abs(sin(float(uniforms.time) * 0.0005)), .5f);
    return id < t * float(uniforms.maxInstanceCount);
}

// https://catlikecoding.com/unity/tutorials/basics/compute-shaders/
const static float PI = 3.1415926535897932384626;
float3 TorusDeformed (float u, float v, float t) {
	float r1 = 0.7 + 0.1 * sin(PI * (6.0 * u + 0.5 * t));
	float r2 = 0.15 + 0.05 * sin(PI * (8.0 * u + 4.0 * v + 2.0 * t));
	float s = r2 * cos(PI * v) + r1;
	float3 p;
	p.x = s * sin(PI * u);
	p.y = r2 * sin(PI * v);
	p.z = s * cos(PI * u);
	return p;
}

// https://commons.wikimedia.org/wiki/File:Parametric_surface_illustration_(trefoil_knot).png
float3 Trefoil(float u, float v, float t) {
    const float x = t * sin(3 * u) / (2 + cos(v));
    const float y = t * (sin(u) + 2 * sin(2 * u)) / (2 + cos(v + PI * 2 / 3));
    const float z = t / 2 * (cos(u) - 2 * cos(2 * u)) * (2 + cos(v)) * (2 + cos(v + PI * 2 / 3)) / 4;
    return float3(x, y, z);
}

float3 Torus(float u, float v, float r1, float r2) {
    const float x = r2 * sin(v);
    const float y = (r1 + r2 * cos(v)) * sin(u);
    const float z = (r1 + r2 * cos(v)) * cos(u);
    return float3(x, y, z);
}

[numthreads(256, 1, 1)]
void main(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    const uint id = dispatchThreadID.x;

    const int    range = int(sqrt(float(uniforms.maxInstanceCount)));
    const float    x   = (float(id / range) / float(range));
    const float    y   = (float(id % range) / float(range));

    const float2 uv = float2(x, y) * 2. * PI;
    const float3 position = Torus(uv.x, uv.y, 5.f, 3.f);

    if (isVisible(id))
    {
        uint writeId;
        InterlockedAdd(draw[0].instanceCount, 1u, writeId);
        instances[writeId] = float4(position, 1.);
    }
}
