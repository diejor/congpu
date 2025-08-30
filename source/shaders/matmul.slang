import tools.printing;
import tensor;

Tensor<float, int, int> tensorA;

[numthreads(1,1,1)]
void computeMain(uint3 threadId : SV_DispatchThreadID)
{
    printf_("Element %d, %d: %f\n", threadId.x, threadId.y,  tensorA[threadId.x, threadId.y]);
    printf_("Size: %d\n", uint(tensorA.getCount()));
}

