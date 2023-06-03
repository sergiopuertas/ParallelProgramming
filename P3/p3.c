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

#define M  1000000 // Number of sequences
#define N  200  // Number of bases per sequence

unsigned int g_seed = 0;

int fast_rand(void) {
    g_seed = (214013*g_seed+2531011);
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
int main(int argc, char *argv[] ) {

    int i, j;
    int *data1, *data2, *recvbuf1,*recvbuf2,*result,*result2;
    int numprocs, rank;
    struct timeval tv1, tv2,tv3,tv4;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int rows = floor(M/numprocs);
    int chunk = N * rows;
    int messtime, comptime;

    if(!rank){
        data1 = (int *) malloc(M * N * sizeof(int));
        data2 = (int *) malloc(M * N * sizeof(int));
        result = (int *) malloc(M * sizeof(int));
        for (i = 0; i < M; i++) {
            for (j = 0; j < N; j++) {
                data1[i * N + j] = fast_rand();
                data2[i * N + j] = fast_rand();
            }
        }
    }

    recvbuf1 = (int *) malloc(chunk * sizeof(int));
    recvbuf2 = (int *) malloc(chunk * sizeof(int));
    result2 = (int *) malloc(rows * sizeof(int));
    

    
    gettimeofday(&tv3, NULL);

    gettimeofday(&tv1, NULL);
    MPI_Scatter(data1,chunk,MPI_INT,recvbuf1,chunk,MPI_INT,0,MPI_COMM_WORLD);
    MPI_Scatter(data2,chunk,MPI_INT,recvbuf2,chunk,MPI_INT,0,MPI_COMM_WORLD);
    gettimeofday(&tv2, NULL);
    messtime = (tv2.tv_usec - tv1.tv_usec)+ 1000000 * (tv2.tv_sec - tv1.tv_sec);

    gettimeofday(&tv1, NULL);
    for (i = 0; i < rows; i++) {
        result2[i] = 0;
        for (j = 0; j < N; j++) {
            result2[i] += base_distance(recvbuf1[i * N + j], recvbuf2[i * N + j]);

        }
    }
    if(M%numprocs != 0 && rank == 0){
        for (i = M - M%numprocs; i < M; i++) {
            result[i] = 0;
            for (j = 0; j < N; j++) {
                result[i] += base_distance(data1[i * N + j], data2[i * N + j]);
            }
        }
    }
    gettimeofday(&tv2, NULL);
    comptime = (tv2.tv_usec - tv1.tv_usec)+ 1000000 * (tv2.tv_sec - tv1.tv_sec);


    gettimeofday(&tv1, NULL);
    MPI_Gather(result2, rows, MPI_INT, result, rows, MPI_INT, 0, MPI_COMM_WORLD);
    gettimeofday(&tv2, NULL);
    messtime += (tv2.tv_usec - tv1.tv_usec)+ 1000000 * (tv2.tv_sec - tv1.tv_sec);


    printf ("Message passing time process nº%d= %lfs\n",rank, (double) messtime/1E6);
    printf ("Computation time process nº%d= %lfs\n\n",rank, (double) comptime/1E6);
    gettimeofday(&tv4, NULL);
    int microseconds = (tv4.tv_usec - tv3.tv_usec)+ 1000000 * (tv4.tv_sec - tv3.tv_sec);
    MPI_Finalize();

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
    if(!rank){
        free(data1);
        free(data2);
        free(result);
    }
    
    free(recvbuf1);
    free(recvbuf2);
    free(result2);

    return 0;
}
