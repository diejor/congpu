StructuredBuffer<float> buffer0;
RWStructuredBuffer<float> result;

[shader("compute")]
[numthreads(1,1,1)]
void computeMain(uint3 threadId : SV_DispatchThreadID)
{
    let index = threadId.x;
    result[index] = buffer0[index] + 1;
}
