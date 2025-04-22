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

struct SBT {
  float r;
  float g;
  float b;
};
[[vk::shader_record_ext]]
ConstantBuffer<SBT> sbt;

[shader("raygeneration")]
void rayGen()
{
    uint3 launchID = DispatchRaysIndex();
    uint3 launchSize = DispatchRaysDimensions();

    const float2 pixelCenter = float2(launchID.xy) + float2(0.5);
    const float2 inUV = pixelCenter/float2(launchSize.xy);
    const float2 d = inUV * 2.0 - 1.0;

	const float4 origin = mul(transpose(camera.invView), float4(0, 0, 0, 1));
	const float4 target = camera.invProj * vec4(d.x, d.y, 1, 1) ;
	const float4 direction = camera.invView * vec4(normalize(target.xyz), 0) ;

    const float tmin = 0.001;
    const float tmax = 10000.0;

	// use border to demonstrate raygen record data
	if (all(launchID.xy > int2(16, 16)) && all(launchID.xy < launchSize.xy - int2(16, 16)))
	{
		// Generate a checker board pattern to trace out rays or use hit record data
		int2 pos = int2(launchID.xy / 16);
		if (((pos.x + pos.y % 2) % 2) == 0) {
			// This will set hit value to either hit or miss SBT record color
			TraceRay(rs, RAY_FLAG_FORCE_OPAQUE, 0xff, 0, 0, 0, rayDesc, payload);
		}
		else {
			// Set the hit value to the raygen SBT data
			payload.hitValue = float3(sbt.r, sbt.g, sbt.b);
		}
	}
	else {
		// Set hit value to black
		hitValue = vec3(0.0, 0.0, 0.0);
	}

	image[int2(launchID.xy)] = float4(payload.hitValue, 0.0);
}

[shader("miss")]
void main(inout Payload payload)
{
    // Update the hit value to the hit record SBT data associated with this
    // miss record
    payload.hitValue = float3(sbt.r, sbt.g, sbt.g);
}

struct Attributes
{
  float2 bary;
};

[shader("closesthit")]
void closestHit(inout Payload payload, in Attributes attribs)
{
        // Update the hit value to the hit record SBT data associated with this
        // geometry ID and ray ID
    payload.hitValue = float3(sbt.r, sbt.g, sbt.g);
}