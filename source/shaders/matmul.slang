import tools.printing;
import tensor;

Tensor<float, uint, uint> tensorA;

[numthreads(1,1,1)]
void computeMain(uint3 threadId : SV_DispatchThreadID)
{
    printf_("Element 0: %f\n", tensorA[0, 3]);
    printf_("Element 0: %f\n", tensorA[threadId.x, threadId.y]);
}

