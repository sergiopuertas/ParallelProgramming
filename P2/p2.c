#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include </usr/local/include/mpi.h>
int MPI_FlatreeColective(void* buf, void* recvbuf, int count, MPI_Datatype datatype,MPI_Op op,int root,MPI_Comm comm){
    int numprocs,rank;
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD , &rank);
    MPI_Send(buf ,count,MPI_INT ,root,0,MPI_COMM_WORLD);
    int ret = MPI_SUCCESS;
    if(rank == root){
        for (int j = 0; j < numprocs; ++j) {
            ret = MPI_Recv(buf ,count,datatype ,MPI_ANY_SOURCE,0,comm,MPI_STATUS_IGNORE);
            if(ret != MPI_SUCCESS){
                return ret;
            }
            *(int*) recvbuf += *(int*) buf;
        }
    }
    return ret;
}

int MPI_BinomialBcast(void *buf, int count, MPI_Datatype datatype, int root, MPI_Comm comm) {
    int numprocs, rank;
    MPI_Comm_size(comm, &numprocs);
    MPI_Comm_rank(comm, &rank);
    int depth = (int) (ceil(log2(numprocs)));
    int i ,partner ,ret = MPI_SUCCESS;
    for (i = 1; i <= depth; i++) {
        if (rank < (int) pow(2, i-1)) {
            partner= rank + (int) pow(2, i-1);
            if(partner < numprocs) {
                printf("Proc %d sends to partner %d\n", rank, partner);
                ret  = MPI_Send(buf, count, datatype, partner, 0, comm);
                if(ret != MPI_SUCCESS){
                    return ret;
                }
            }
        }
        else{
            int source = rank - (int) pow(2, i - 1);
            if(source < (int) pow(2, i - 1)) {
                printf("Proc %d receives value from %d\n", rank, source);
                ret = MPI_Recv(buf, count, datatype, source, 0, comm, MPI_STATUS_IGNORE);
                if(ret != 0){
                    return ret;
                }
            }
        }
    }
    return ret;
}

void inicializaCadena(char *cadena, int n){
    int i;
    for(i=0; i<n/2; i++){
        cadena[i] = 'A';
    }
    for(i=n/2; i<3*n/4; i++){
        cadena[i] = 'C';
    }
    for(i=3*n/4; i<9*n/10; i++){
        cadena[i] = 'G';
    }
    for(i=9*n/10; i<n; i++){
        cadena[i] = 'T';
    }
}

int main(int argc, char *argv[])
{
    int i, n, count=0;
    int numprocs, rank;
    char *cadena;
    char L;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD , &rank);

    n = atoi(argv[1]);
    L = *argv[2];

    int ret;

    //ret = MPI_Bcast(&n ,1,MPI_INT ,0,MPI_COMM_WORLD);
    //ret = MPI_Bcast(&L ,1,MPI_CHAR ,0,MPI_COMM_WORLD);

    ret = MPI_BinomialBcast(&n ,1,MPI_INT ,0,MPI_COMM_WORLD);
    ret += MPI_BinomialBcast(&L ,1,MPI_CHAR ,0,MPI_COMM_WORLD);

    if(ret != 0){
        printf("Error: MPI_BinomialBcast failed");
        exit(ret/2);
    }

    cadena = (char *) malloc(n*sizeof(char));
    inicializaCadena(cadena, n);

    for(i=rank; i<n; i+=numprocs){
        if(cadena[i] == L){
            count++;
        }
    }
    int sum = 0;

    //ret = MPI_Reduce(&count, &sum, 1,MPI_INT,MPI_SUM, 0 , MPI_COMM_WORLD);
    ret = MPI_FlatreeColective(&count, &sum, 1,MPI_INT,MPI_SUM, 0 , MPI_COMM_WORLD);
    if(ret != 0){
        printf("Error: MPI_BinomialBcast failed");
        exit(ret);
    }
    if(rank == 0){
        printf("El numero de apariciones de la letra %c es %d\n", L, sum);
    }
    free(cadena);
    MPI_Finalize();
    exit(0);
}
