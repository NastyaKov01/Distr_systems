#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <signal.h>
#include <mpi.h>
#include <mpi-ext.h>

MPI_Comm main_comm = MPI_COMM_WORLD;
int rank, num_procs, dead_rank, points_num = 4;

static void verbose_errhandler(MPI_Comm* pcomm, int* perr, ...) {
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
    dead_rank = ranks_gc[0];
    free(ranks_gf); free(ranks_gc);
    MPIX_Comm_shrink(*pcomm, &main_comm);
    MPI_Comm_rank(main_comm, &rank);
    MPI_Comm_size(main_comm, &num_procs);
}

double fun(double x)
{
    return x / (x * x * x + x * x + 5 * x + 1);
}

double integral(double a, double b, int n, int myrank)
{
    double res = 0; 
    double h = (b - a) / (2 * n);
    double x1, x2;
    int cnt = 1, part = n / num_procs / points_num;
    FILE *fw;
    char filename[15];
    for (int i = myrank; i <= n; i += num_procs) {
        x1 = a + (2 * i - 1) * h;
        x2 = a + 2 * h * i;
        res += 4 * fun(x1);
        res += 2 * fun(x2);
        if (i == myrank + cnt * part * num_procs && cnt != points_num) {
            sprintf(filename, "%d_%d", rank, cnt);
            fw = fopen(filename, "w");
            fprintf(fw, "%lf", res);
            cnt++;
            fclose(fw);
        }
        if (rank == num_procs - 1 && i > myrank + 2 * part * num_procs) {
            raise(SIGKILL);
        }
    }
    sprintf(filename, "%d_%d", rank, cnt);
    fw = fopen(filename, "w");
    fprintf(fw, "%lf", res);
    fclose(fw);
    res += fun(b - h);
    res += fun(a) + fun(b);
    res *= (h / 3);
    return res;
}

double recovery_integral(double a, double b, int n, int myrank, double cur_res, int cnt)
{
    double h = (b - a) / (2 * n);
    double x1, x2;
    int part = n / num_procs / points_num;
    for (int i = myrank + (cnt * part + 1) * num_procs; i <= n; i += num_procs) {
        x1 = a + (2 * i - 1) * h;
        x2 = a + 2 * h * i;
        cur_res += 4 * fun(x1);
        cur_res += 2 * fun(x2);
    }
    cur_res += fun(b - h);
    cur_res += fun(a) + fun(b);
    cur_res *= (h / 3);
    return cur_res;
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
    MPI_Barrier(main_comm);
    MPI_Bcast(&n, 1, MPI_INT, 0, main_comm);
    double cur_sum = integral(a, b, n, rank);
    MPI_Barrier(main_comm);
    MPI_Reduce(&cur_sum, &result, 1, MPI_DOUBLE, MPI_SUM, 0, main_comm);

    if (rank == 0) {
        FILE *fr;
        int cnt = points_num;
        char filename[15];
        double res;
        sprintf(filename, "%d_%d", dead_rank, cnt);
        fr = fopen(filename, "r");
        while (!fr) {
            cnt--;
            sprintf(filename, "%d_%d", dead_rank, cnt);
            fr = fopen(filename, "r");
        }
        fscanf(fr, "%lf", &res);
        fclose(fr);
        res = recovery_integral(a, b, n, dead_rank, res, cnt);
        result += res;
        printf("Result is %lf\n", result);
        finish = MPI_Wtime();
        printf("Time passed: %lf\n", finish - start);
    }

    MPI_Finalize();
    return 0;
}