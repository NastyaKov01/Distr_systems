#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "mpi.h"


int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    int rank, numtasks;
    // unsigned L = 8000;
    unsigned L = 80;
    int* message = malloc(L * sizeof(int));
    int paths[2][7] = {{0, 1, 2, 3, 7, 11, 15}, {0, 4, 8, 12, 13, 14, 15}};
    MPI_Status status;
    MPI_Request req1, req2;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    srand((unsigned) time(NULL));
    if (rank == 0) {
        printf("Created message in process %d:\n", rank);
        for (unsigned i = 0; i < L; i++) {
            message[i] = rand() % L;
            printf("%d ",  message[i]);
        }
        printf("\n");
    }
    
    if (rank == 15) {
        MPI_Irecv(message, L / 2, MPI_INT, 11, 0, MPI_COMM_WORLD, &req1);
        MPI_Irecv(message + L / 2, L / 2, MPI_INT, 14, 0, MPI_COMM_WORLD, &req2);
        printf("\n");
    } else {
        for (unsigned i = 0, tag = 0; i < 2; i++, tag++) {
            for (unsigned j = 1; j < 6; j++) {
                if (paths[i][j] == rank) {
                    unsigned shift = 0;
                    if (i == 1) {
                        shift = L / 2;
                    }
                    MPI_Irecv(message + shift, L / 2, MPI_INT, paths[i][j - 1], 0, MPI_COMM_WORLD, &req1);
                    break;
                }
            }
        }
    }
    
    MPI_Barrier(MPI_COMM_WORLD);

    if (rank == 15) {
        MPI_Wait(&req1,  MPI_STATUS_IGNORE);
        MPI_Wait(&req2,  MPI_STATUS_IGNORE);
        printf("Received message in process %d:\n", rank);
        for (unsigned i = 0; i < L; i++) {
            printf("%d ",  message[i]);
        }
        printf("\n");
    } else if (rank == 0) {
        MPI_Irsend(message, L / 2, MPI_INT, 1, 0, MPI_COMM_WORLD, &req1);
        MPI_Irsend(message + L / 2, L / 2, MPI_INT, 4, 0, MPI_COMM_WORLD, &req2);
        MPI_Wait(&req1,  MPI_STATUS_IGNORE);
        MPI_Wait(&req2,  MPI_STATUS_IGNORE);
    } else {
        
        for (unsigned i = 0; i < 2; i++) {
            for (unsigned j = 0; j < 6; j++) {
                if (paths[i][j] == rank) {
                    MPI_Wait(&req1,  MPI_STATUS_IGNORE);
                    unsigned shift = 0;
                    if (i == 1) {
                        shift = L / 2;
                    }
                    MPI_Rsend(message + shift, L / 2, MPI_INT, paths[i][j + 1], 0, MPI_COMM_WORLD);
                    break;
                }
            }
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);
    free(message);
    MPI_Finalize();
    return 0;
}