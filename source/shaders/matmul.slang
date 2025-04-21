import printing;

[numthreads(4,1,1)]
void computeMain(uint3 threadId : SV_DispatchThreadID)
{
    printf_("Thread ID: %d, %d, %d\n", threadId.x, threadId.y, threadId.z);
}

