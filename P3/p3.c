#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <math.h>
#include </usr/local/include/mpi.h>

#define DEBUG 1

/* Translation of the DNA bases
   A -> 0
   C -> 1
   G -> 2
   T -> 3
   N -> 4*/

#define M  16 // Number of sequences
#define N  4  // Number of bases per sequence

unsigned int g_seed = 0;

int fast_rand(void) {
    g_seed = (2143*g_seed+2301);
    return (g_seed>>16) % 5;
}

// The distance between two bases
int base_distance(int base1, int base2){

    if((base1 == 4) || (base2 == 4)){
        return 3;
    }

    if(base1 == base2) {
        return 0;
    }

    if((base1 == 0) && (base2 == 3)) {
        return 1;
    }

    if((base2 == 0) && (base1 == 3)) {
        return 1;
    }

    if((base1 == 1) && (base2 == 2)) {
        return 1;
    }

    if((base2 == 2) && (base1 == 1)) {
        return 1;
    }

    return 2;
}
//theoretical lectures all about design
//each task is comparing ONE LINE
//divide in chunks of rows, each block is a task
//SPMD implementation
//use mpi_scatter
//gather all in memory of p0
//measure time in mpi_gather and time in mpi_scatter (time in communication)

int main(int argc, char *argv[] ) {

    int i, j;
    int *data1, *data2;
    int *result, numprocs, rank;
    struct timeval tv1, tv2;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    int div = N*M/numprocs;
    int *recvbuf1 = (int*) malloc (div*sizeof(int));
    int *recvbuf2= (int*) malloc (div*sizeof(int));
    result = (int *) malloc(M * sizeof(int));
    int *result2 = (int *) malloc(div/N* sizeof(int));

    data1 = (int *) malloc(M * N * sizeof(int));
    data2 = (int *) malloc(M * N * sizeof(int));

    /* Initialize Matrices */
    for (i = 0; i < M; i++) {
        for (j = 0; j < N; j++) {
            /* random with 20% gap proportion */
            data1[i * N + j] = fast_rand();
            data2[i * N + j] = fast_rand();
            }
        }
    gettimeofday(&tv1, NULL);
    MPI_Scatter(data1,div,MPI_INT,recvbuf1,div,MPI_INT,0,MPI_COMM_WORLD);
    MPI_Scatter(data2,div,MPI_INT,recvbuf2,div,MPI_INT,0,MPI_COMM_WORLD);
        for (i = 0; i < div/N; i++) {
            result2[i] = 0;
            for (j = 0; j < N; j++) {
                result2[i] += base_distance(recvbuf1[i * N + j], recvbuf2[i * N + j]);
            }
        }

    MPI_Gather(result2, div/N, MPI_INT, result, div/N, MPI_INT, 0, MPI_COMM_WORLD);


    gettimeofday(&tv2, NULL);

    int microseconds = (tv2.tv_usec - tv1.tv_usec)+ 1000000 * (tv2.tv_sec - tv1.tv_sec);
    if(rank==0){
        /* Display result */
        if (DEBUG == 1) {
            int checksum = 0;
            for(i=0;i<M;i++) {
                checksum += result[i];
            }
            printf("Checksum: %d\n ", checksum);
        }
        else if (DEBUG == 2) {
            for(i=0;i<M;i++) {
                printf(" %d \t ",result[i]);
            }
            printf("\n");

        }
        else {
            printf ("Time (seconds) = %lf\n", (double) microseconds/1E6);
        }
    }
    free (result);
    free(result2);
    free(recvbuf2);
    free(recvbuf1);
    free(data1);
    free(data2);
    MPI_Finalize();


    return 0;
}
