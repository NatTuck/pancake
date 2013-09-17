
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

#include <pancake/shim.h>

#include "pclu.h"
#include "timer.h"

#define SIZE   4096
#define SPIN   1
#define DTYPE  float
#define FORMAT "%.02f"
#define ONE    1.0f

#define PRINT_DATA 0

DTYPE
rand48()
{
    static unsigned short seed[3] = { 50, 461, 0 };
    static int seeded = 0;

    if (seeded == 0) {
        seed[2] = (short)getpid();
        seeded = 1;
    }

    return (DTYPE) erand48(seed);
}

typedef struct matrix {
    DTYPE *data;
    size_t rows;
    size_t cols;
} matrix;

size_t
matrix_size(matrix * mm)
{
    return mm->rows * mm->cols;
}

size_t
matrix_bytes(matrix * mm)
{
    return matrix_size(mm) * sizeof(DTYPE);
}

matrix *
create_matrix(size_t rows, size_t cols)
{
    matrix *mm = (matrix *) malloc(sizeof(matrix));
    mm->rows = rows;
    mm->cols = cols;

    size_t size = matrix_size(mm);
    mm->data = (DTYPE *) calloc(size, sizeof(DTYPE));

    return mm;
}

void
destroy_matrix(matrix * mm)
{
    free(mm->data);
    free(mm);
}

void
set_cell(matrix * mm, size_t ii, size_t jj, DTYPE value)
{
    mm->data[ii * mm->cols + jj] = value;
}

DTYPE
get_cell(matrix * mm, size_t ii, size_t jj)
{
    return mm->data[ii * mm->cols + jj];
}

matrix *
create_identity_matrix(size_t rows, size_t cols)
{
    matrix *mm = create_matrix(rows, cols);

    for (size_t ii = 0; ii < rows; ++ii) {
        if (ii >= cols)
            break;

        set_cell(mm, ii, ii, ONE);
    }

    return mm;
}

matrix *
create_random_matrix(size_t rows, size_t cols)
{
    matrix *mm = create_matrix(rows, cols);

    for (size_t ii = 0; ii < rows; ++ii) {
        for (size_t jj = 0; jj < cols; ++jj) {
            set_cell(mm, ii, jj, 10 * rand48());
        }
    }

    return mm;
}

void
print_matrix(matrix * mm)
{
    for (size_t ii = 0; ii < mm->rows; ++ii) {
        for (size_t jj = 0; jj < mm->cols; ++jj) {
            DTYPE xx = get_cell(mm, ii, jj);
            printf(FORMAT, xx);
            printf(" ");
        }
        printf("\n");
    }
}

void
check_result(matrix * cc, matrix * aa)
{
    /* Checks that each element in cc is twice the
     * coresponding element in aa.
     *
     * This assumes square matrixes, an identity matrix
     * for bb, and a spin value of 2.
     */

    const DTYPE EPS = 0.001;

    for (size_t ii = 0; ii < cc->rows; ++ii) {
        for (size_t jj = 1; jj < cc->cols; ++jj) {
            DTYPE xx = get_cell(aa, ii, jj);
            DTYPE yy = get_cell(cc, ii, jj);

            if (abs(SPIN*xx - yy) > EPS) {
                printf("ERROR - Incorrect result.\n");
                printf("At %ld, %ld got %f instead of %f\n",
                       ii, jj, yy, xx);
                return;
            }
        }
    }

    printf("Result is correct.\n");
}

void
matrix_multiply_cl(pclu_context * pclu, matrix * cc, matrix * aa, matrix * bb)
{
    pclu_program *pgm = pclu_create_program(pclu, "fmma.cl");

    char *log = pclu_program_build_log(pgm);
    if (strlen(log) > 0)
        printf("Build log:\n%s\n", log);

    size_t aa_size = matrix_bytes(aa);
    pclu_buffer *aa_buf = pclu_create_buffer(pclu, aa_size);
    pclu_write_buffer(aa_buf, aa_size, aa->data);

    size_t bb_size = matrix_bytes(bb);
    pclu_buffer *bb_buf = pclu_create_buffer(pclu, bb_size);
    pclu_write_buffer(bb_buf, bb_size, bb->data);

    size_t cc_size = matrix_bytes(cc);
    pclu_buffer *cc_buf = pclu_create_buffer(pclu, cc_size);
    pclu_write_buffer(cc_buf, cc_size, cc->data);

    pclu_range range = pclu_range_2d(cc->rows, cc->cols);

    cl_long nn = SIZE;
    cl_long spin = SPIN;

    timer* tt = timer_alloc();
    pclu_call_kernel(pgm, "fmma", range, 5, buf_arg(cc_buf), buf_arg(aa_buf),
                     buf_arg(bb_buf), lit_arg(nn), lit_arg(spin));
    double kt = timer_read(tt);
    timer_free(tt);

    printf("Kernel took %.04f seconds.\n", kt);

    pclu_read_buffer(cc_buf, cc_size, cc->data);

    printf("alpha\n");

    pclu_destroy_program(pgm);

    printf("beta\n");
}

int
main(int argc, char *argv[])
{
    pclu_context *pclu = pclu_create_context();
    printf("\n%s\n", pclu_context_info(pclu));

    matrix *aa = create_random_matrix(SIZE, SIZE);
    matrix *bb = create_identity_matrix(SIZE, SIZE);

#if PRINT_DATA
    printf("Matrix aa:\n");
    print_matrix(aa);
    printf("\n");

    printf("Matrix bb:\n");
    print_matrix(bb);
    printf("\n");
#endif

    timer* tt1 = timer_alloc();

    matrix *cc = create_matrix(SIZE, SIZE);
    matrix_multiply_cl(pclu, cc, aa, bb);
    
    double tm1 = timer_read(tt1);
    timer_free(tt1);
    
    printf("Matrix_multiply_cl took %.04f seconds.\n", tm1);

#if PRINT_DATA
    printf("Matrix cc:\n");
    print_matrix(cc);
    printf("\n");
#endif

    timer* tt = timer_alloc();
    check_result(aa, cc);
    
    double ct = timer_read(tt);
    timer_free(tt);
    
    printf("Check result took %.04f seconds.\n", ct);

    destroy_matrix(aa);
    destroy_matrix(bb);

    pclu_destroy_context(pclu);
    return 0;
}
