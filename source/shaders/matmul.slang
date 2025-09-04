import tensor;
import tools.printing;

static const int M = 8;
static const int N = 8;

RWTensorBuffer<float, int, int> input;

[numthreads(1,1,1)]
void computeMain(uint3 tid  : SV_DispatchThreadID,
                 uint3 ltid : SV_GroupThreadID)
{
    Tensor<float, M, N> tensorA;
    var inputRef = input;

    if (ltid.x == 0) {
        // read
        for (int j = 0; j < N; ++j)
            for (int i = 0; i < M; ++i)
                tensorA[i, j] = inputRef[i, j];

        // do something & WRITE BACK (side effect)
        for (int j = 0; j < N; ++j)
            for (int i = 0; i < M; ++i) {
                inputRef[i, j] = tensorA[i, j] + 1.0f; // or any transform
                printf_("Element %d,%d: %f\n", uint(i), uint(j), tensorA[i, j]);
            }
    }

    // printf_("Element %d,%d: %f\n", tid.x, tid.y, tensorA[tid.x, tid.y]);
    // printf_("Size: %d\n", uint(tensorA.getCount()));
}
