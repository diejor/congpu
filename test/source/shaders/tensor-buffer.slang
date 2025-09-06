import tensor;

RWTensorBuffer<float, int, int> input;

[numthreads(1,1,1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    var inputRef = input;
    inputRef[0,0] = inputRef[0,0];
}
