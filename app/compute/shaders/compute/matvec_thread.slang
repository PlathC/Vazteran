// result = matrixData * vectorData, the matrix being stored row-major.
// One thread per row: the simplest version, neighbouring threads read memory a whole row apart.

[[vk::binding(0, 0)]]
StructuredBuffer<float> matrixData;

[[vk::binding(1, 0)]]
StructuredBuffer<float> vectorData;

[[vk::binding(2, 0)]]
RWStructuredBuffer<float> result;

[shader("compute")]
[numthreads(256, 1, 1)]
void main(uint3 id: SV_DispatchThreadID)
{
    uint rows, columns, stride;
    result.GetDimensions(rows, stride);
    vectorData.GetDimensions(columns, stride);
    if (id.x >= rows)
        return;

    float sum = 0.0;
    for (uint c = 0; c < columns; c++)
        sum += matrixData[id.x * columns + c] * vectorData[c];

    result[id.x] = sum;
}
