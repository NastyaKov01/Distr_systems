#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <mpi.h>

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
    int n, num_procs, rank, flag;
    double a = 0, b = 200;
    double result;
    n = atoi(argv[1]);

    MPI_Init(&argc, &argv);
    
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    double start, finish;
    if (rank == 0) {
        start = MPI_Wtime();
    }

    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
    double cur_sum = integral(a, b, n, rank, num_procs);
    MPI_Reduce(&cur_sum, &result, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        printf("Result is %lf\n", result);
        finish = MPI_Wtime();
        printf("Time passed: %lf\n", finish - start);
    }

    MPI_Finalize();
    return 0;
}