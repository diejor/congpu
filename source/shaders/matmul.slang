import printing;

[numthreads(4,1,1)]
void computeMain(int i : SV_GroupThreadID)
{
    println("Hola from thread ", i);
}
