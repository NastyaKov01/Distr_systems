#include <stdio.h>
#include <stdlib.h>

#include "mpi.h"


int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    int rank, numtasks;
    // unsigned L = 8000;
    unsigned L = 80;
    // unsigned K = 100;
    unsigned K = 10; 
    unsigned block_size = L / (2 * K); 
    int* message = malloc(L * sizeof(int));
    int paths[2][7] = {{0, 1, 2, 3, 7, 11, 15}, {0, 4, 8, 12, 13, 14, 15}};
    MPI_Status status;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    printf("Hello, I'm %d\n", rank);
    if (rank == 0) {
        printf("Message creation\n");
        for (unsigned i = 0; i < L; i++) {
            message[i] = rand() % L;
            printf("%d ",  message[i]);
        }
        printf("\n");
    }
    
    for (unsigned i = 0, tag = 0; i < 2; i++, tag++) {
        for (unsigned j = 1; j < 7; j++) {
            if (paths[i][j] == rank) {
                unsigned shift = 0;
                if (i == 1) {
                    shift = K * block_size;
                }
                printf("Receive in %d\n", rank);
                for (unsigned k = 0; k < K; k++) {
                    MPI_Recv(message + k * block_size + shift, block_size, MPI_INT, paths[i][j - 1], tag, MPI_COMM_WORLD, &status);
                    printf("%d ", k);
                }
                printf("\n");
                break;
            }
        }
    }
    MPI_Barrier(MPI_COMM_WORLD);
    // unsigned buff_size = L * sizeof(double) + MPI_BSEND_OVERHEAD;
    // double* buff = malloc(buff_size);
    // MPI_Buffer_attach(buff, buff_size);
    for (unsigned i = 0, tag = 1; i < 2; i++, tag++) {
        for (unsigned j = 0; j < 6; j++) {
            if (paths[i][j] == rank) {
                unsigned shift = 0;
                if (i == 1) {
                    shift = K * block_size;
                }
                printf("Send in %d\n", rank);
                for (unsigned k = 0; k < K; k++) {
                    MPI_Rsend(message + k * block_size + shift, block_size, MPI_INT, paths[i][j + 1], tag, MPI_COMM_WORLD);
                    printf("%d ", k);
                }
                printf("\n");
                break;
            }
        }
    }
    // MPI_Buffer_detach(buff, &buff_size);
    // free(buff);
    
    if (rank == 15) {
        printf("Received message:\n");
        for (unsigned i = 0; i < L; i++) {
            printf("%d \n",  message[i]);
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);
    free(message);
    MPI_Finalize();
    return 0;
}