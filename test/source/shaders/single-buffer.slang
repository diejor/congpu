StructuredBuffer<float> buffer0;
RWStructuredBuffer<float> result;

float f(float a)
{
    return pow(a, 2.0f) + 1.0f;
}

[shader("compute")]
[numthreads(1,1,1)]
void computeMain(uint3 threadId : SV_DispatchThreadID)
{
    let index = threadId.x;
    result[index] = f(buffer0[index]);
}

