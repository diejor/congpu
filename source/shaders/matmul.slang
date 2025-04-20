groupshared Atomic<uint> m[4];

[numthreads(4,1,1)]
void computeMain(int i : SV_GroupThreadID)
{
    m[i] = 0;
    GroupMemoryBarrierWithGroupSync();
    uint oldvalue;
    m[i] += 1;  // or equivalently: m[i].add(1);
    GroupMemoryBarrierWithGroupSync();
    if (m[i].load() != 0)
    {
        // This thread is not the first to write to m[i]
        // Do something else
    }
    else
    {
        // This thread is the first to write to m[i]
        // Do something else
    }
}
