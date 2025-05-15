// Copyright 2020 Google LLC
// Reference: https://github.com/SaschaWillems/Vulkan/blob/master/examples

[[vk::binding(0, 0)]]
RaytracingAccelerationStructure topLevelAS : register(t0);
[[vk::binding(1, 0)]]
RWTexture2D<float4> image : register(u1);

struct Camera
{
	float4x4 invView;
	float4x4 invProj;
};

[[vk::binding(2, 0)]]
ConstantBuffer<Camera> camera;

struct Payload
{
    [[vk::location(0)]]
    float3 hitValue;
};

[shader("raygeneration")]
void rayGen()
{
    uint3 launchID = DispatchRaysIndex();
    uint3 launchSize = DispatchRaysDimensions();

    const float2 pixelCenter = float2(launchID.xy) + float2(0.5);
    const float2 inUV        = pixelCenter/float2(launchSize.xy);
    const float2 d           = inUV * 2.0 - 1.0;
	const float4 target      = mul((camera.invProj), float4(d.x, d.y, 1, 1));

	Payload payload;

	RayDesc rayDesc;
	rayDesc.Origin = mul((camera.invView), float4(0,0,0,1)).xyz;
	rayDesc.Direction = mul((camera.invView), float4(normalize(target.xyz), 0)).xyz;
	rayDesc.TMin = 0.001;
	rayDesc.TMax = 10000.0;

	// use border to demonstrate raygen record data
	if (all(launchID.xy > int2(16, 16)) && all(launchID.xy < launchSize.xy - int2(16, 16)))
	{
		// Generate a checker board pattern to trace out rays or use hit record data
		int2 pos = int2(launchID.xy / 16);
		if (((pos.x + pos.y % 2) % 2) == 0) {
			// This will set hit value to either hit or miss SBT record color
			TraceRay(topLevelAS, RAY_FLAG_FORCE_OPAQUE, 0xff, 0, 0, 0, rayDesc, payload);
		}
		else {
			payload.hitValue = float3(1.);
		}
	}
	else {
		// Set hit value to black
		payload.hitValue = float3(0.0, 0.0, 0.0);
	}

	image[int2(launchID.xy)] = float4(payload.hitValue, 0.0);
}

[shader("miss")]
void miss(inout Payload payload)
{
    payload.hitValue = float3(0., 1., 0.);
}

struct Attributes
{
  float2 bary;
};

[shader("closesthit")]
void closestHit(inout Payload payload, in Attributes attribs)
{
    payload.hitValue = float3(0., 0., 1.);
}
