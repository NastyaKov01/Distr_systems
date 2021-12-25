#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <mpi.h>
#include <mpi-ext.h>

MPI_Comm main_comm = MPI_COMM_WORLD;
int rank, num_procs;

static void verbose_errhandler(MPI_Comm* pcomm, int* perr, ...) {
    // MPI_Comm comm = *pcomm;
    int err = *perr;
    char errstr[MPI_MAX_ERROR_STRING];
    int i, size, nf=0, len, eclass;
    MPI_Group group_c, group_f;
    int *ranks_gc, *ranks_gf;

    MPI_Error_class(err, &eclass);
    if( MPIX_ERR_PROC_FAILED != eclass ) {
        MPI_Abort(*pcomm, err);
    }

    MPI_Comm_rank(*pcomm, &rank);
    MPI_Comm_size(*pcomm, &size);

    MPIX_Comm_failure_ack(*pcomm);
    MPIX_Comm_failure_get_acked(*pcomm, &group_f);
    MPI_Group_size(group_f, &nf);
    MPI_Error_string(err, errstr, &len);
    printf("Rank %d / %d: Notified of error %s. %d found dead: { ", rank, size, errstr, nf);

    MPI_Comm_group(*pcomm, &group_c);
    ranks_gf = (int*)malloc(nf * sizeof(int));   /// ranks_gf: input ranks in the "failed group"
    ranks_gc = (int*)malloc(nf * sizeof(int));  /// ranks_gc: output ranks corresponding in the "comm group"
    for(i = 0; i < nf; i++)
        ranks_gf[i] = i;

    MPI_Group_translate_ranks(group_f, nf, ranks_gf, group_c, ranks_gc);
    for(i = 0; i < nf; i++)
        printf("%d ", ranks_gc[i]);
    printf("}\n");
    // MPIX_Comm_revoke(comm);
    // MPIX_Comm_shrink(comm, &new_comm);
    free(ranks_gf); free(ranks_gc);
    // int spawn_error;
    // MPI_Comm_spawn(argv[0], argv[1], 1, MPI_INFO_NULL, 0, MPI_COMM_WORLD, &main_comm, &spawn_error);
    MPIX_Comm_shrink(*pcomm, &main_comm);
    MPI_Comm_rank(main_comm, &rank);
    MPI_Comm_size(main_comm, &num_procs);
}

double fun(double x)
{
    return x / (x * x * x + x * x + 5 * x + 1);
}

double integral(double a, double b, int n, int myrank, int num_procs)
{
    double res = 0; 
    double h = (b - a) / (2 * n);
    double x1, x2;
    for (int i = myrank; i <= n; i += num_procs) {
        x1 = a + (2 * i - 1) * h;
        x2 = a + 2 * h * i;
        res += 4 * fun(x1);
        res += 2 * fun(x2);
    }
    res += fun(b - h);
    res += fun(a) + fun(b);
    res *= (h / 3);
    return res;
}

int main(int argc, char **argv)
{
    int n, flag;
    double a = 0, b = 200;
    double result;
    n = atoi(argv[1]);
    MPI_Errhandler errh;

    MPI_Init(&argc, &argv);
    
    MPI_Comm_size(main_comm, &num_procs);
    MPI_Comm_rank(main_comm, &rank);
    
    MPI_Comm_create_errhandler(verbose_errhandler, &errh);
    MPI_Comm_set_errhandler(main_comm, errh);

    double start, finish;
    if (rank == 0) {
        start = MPI_Wtime();
    }

    if (rank == num_procs - 1) {
        raise(SIGKILL);
    }
    MPI_Barrier(main_comm);
    MPI_Bcast(&n, 1, MPI_INT, 0, main_comm);
    double cur_sum = integral(a, b, n, rank, num_procs);
    MPI_Reduce(&cur_sum, &result, 1, MPI_DOUBLE, MPI_SUM, 0, main_comm);

    if (rank == 0) {
        printf("Result is %lf\n", result);
        finish = MPI_Wtime();
        printf("Time passed: %lf\n", finish - start);
    }

    MPI_Finalize();
    return 0;
}