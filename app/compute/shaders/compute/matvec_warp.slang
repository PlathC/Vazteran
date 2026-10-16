// result = matrixData * vectorData, the matrix being stored row-major.
// One warp per row: the reduction stays in registers, without shared memory nor barrier.
// Assumes 32-wide subgroups; a wider one only runs 32 of its lanes, a narrower one gives a wrong result.

[[vk::binding(0, 0)]]
StructuredBuffer<float> matrixData;

[[vk::binding(1, 0)]]
StructuredBuffer<float> vectorData;

[[vk::binding(2, 0)]]
RWStructuredBuffer<float> result;

static const uint WarpSize = 32;

[shader("compute")]
[numthreads(WarpSize, 1, 1)]
void main(uint3 group: SV_GroupID, uint lane: SV_GroupIndex)
{
    uint rows, columns, stride;
    result.GetDimensions(rows, stride);
    vectorData.GetDimensions(columns, stride);

    float sum = 0.0;
    for (uint c = lane; c < columns; c += WarpSize)
        sum += matrixData[group.x * columns + c] * vectorData[c];

    sum = WaveActiveSum(sum);
    if (lane == 0)
        result[group.x] = sum;
}
