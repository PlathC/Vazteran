// result = matrixData * vectorData, the matrix being stored row-major.
// One workgroup per row: the threads share the row, then thread 0 adds their partial sums.

[[vk::binding(0, 0)]]
StructuredBuffer<float> matrixData;

[[vk::binding(1, 0)]]
StructuredBuffer<float> vectorData;

[[vk::binding(2, 0)]]
RWStructuredBuffer<float> result;

static const uint GroupSize = 256;
groupshared float partial[GroupSize];

[shader("compute")]
[numthreads(GroupSize, 1, 1)]
void main(uint3 group: SV_GroupID, uint thread: SV_GroupIndex)
{
    uint rows, columns, stride;
    result.GetDimensions(rows, stride);
    vectorData.GetDimensions(columns, stride);

    // 1. Each thread sums a strided part of the row, so that consecutive threads read consecutive elements
    float sum = 0.0;
    for (uint c = thread; c < columns; c += GroupSize)
        sum += matrixData[group.x * columns + c] * vectorData[c];
    partial[thread] = sum;

    GroupMemoryBarrierWithGroupSync();

    // 2. Thread 0 adds the partial sums
    if (thread == 0)
    {
        float total = 0.0;
        for (uint t = 0; t < GroupSize; t++)
            total += partial[t];
        result[group.x] = total;
    }
}
