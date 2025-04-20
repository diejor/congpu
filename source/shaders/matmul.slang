import printing;

[numthreads(4,1,1)]
void computeMain(uint3 threadId : SV_DispatchThreadID)
{
    if (threadId.x == 0)
    {
        uint mat[3][3] = {
            { 1, 2, 3 },
            { 4, 5, 6 },
            { 7, 8, 9 }
        };

        IPrintf[] all = {
            mat[0][0], mat[0][1], mat[0][2],
            mat[1][0], mat[1][1], mat[1][2],
            mat[2][0], mat[2][1], mat[2][2]
        };

        printf_(
            "[[%u, %u, %u,\n"
            "  %u, %u, %u,\n"
            "  %u, %u, %u]]\n",
            all
        );
    }
}

