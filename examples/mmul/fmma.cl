
/* Matrix multiply and accumulate */

kernel void
fmma(global float *C, global float *A, global float *B, long nn, long spin)
/* @spec fmma(nn, spin) */
{
    const int xx = get_global_id(0);
    const int yy = get_global_id(1);

    for (long it = 0; it < spin; ++it) {
        float sum = C[nn * yy + xx];

        for (int kk = 0; kk < nn; ++kk) {
            sum += A[nn * yy + kk] * B[nn * kk + xx];
        }

        C[nn * yy + xx] = sum;
    }
}
