#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include </usr/local/include/mpi.h>
int MPI_FlatreeColective(void* buf, void* recvbuf, int count, MPI_Datatype datatype,MPI_Op op,int root,MPI_Comm comm){
    int numprocs,rank;
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD , &rank);
    MPI_Send(buf ,count,MPI_INT ,root,0,MPI_COMM_WORLD);
    if(rank == root){
        for (int j = 0; j < numprocs; ++j) {
            MPI_Recv(buf ,count,datatype ,MPI_ANY_SOURCE,0,comm,MPI_STATUS_IGNORE);
            *(int*) recvbuf += *(int*) buf;
        }
    }
    return 0;
}

int MPI_BinomialBcast(void *buf, int count, MPI_Datatype datatype, int root, MPI_Comm comm) {
    int numprocs, rank;
    MPI_Comm_size(comm, &numprocs);
    MPI_Comm_rank(comm, &rank);
    int depth = (int) (log2(numprocs));
    int i;
    int partner;
    for (i = 0; i < depth; i++) {
        partner= rank + (int) pow(2, i-1);
        if (partner < numprocs)  {
            if (rank < (int) pow(2, i-1)) {
                printf("proc %d sends to partner %d\n", rank, partner);
                MPI_Send(buf, count, datatype, partner, 0, comm);
            }
            if(rank == partner){
                MPI_Recv(buf, count, datatype, rank, 0, comm, MPI_STATUS_IGNORE);
            }
        }
    }
    return 0;
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
    if(argc != 3){
        printf("Numero incorrecto de parametros\nLa sintaxis debe ser: program n L\n  program es el nombre del ejecutable\n  n es el tamaño de la cadena a generar\n  L es la letra de la que se quiere contar apariciones (A, C, G o T)\n");
        exit(1);
    }

    int i, n, count=0;
    int numprocs, rank;
    char *cadena;
    char L;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD , &rank);

    n = atoi(argv[1]);
    L = *argv[2];

    MPI_BinomialBcast(&n ,1,MPI_INT ,0,MPI_COMM_WORLD);
    MPI_BinomialBcast(&L ,1,MPI_CHAR ,0,MPI_COMM_WORLD);

    cadena = (char *) malloc(n*sizeof(char));
    inicializaCadena(cadena, n);

    for(i=rank; i<n; i+=numprocs){
        if(cadena[i] == L){
            count++;
        }
    }
    int sum = 0;

    //MPI_Reduce(&count, &sum, 1,MPI_INT,MPI_SUM, 0 , MPI_COMM_WORLD);
    MPI_FlatreeColective(&count, &sum, 1,MPI_INT,MPI_SUM, 0 , MPI_COMM_WORLD);

    if(rank == 0){
        printf("El numero de apariciones de la letra %c es %d\n", L, sum);
    }
    free(cadena);
    MPI_Finalize();
    exit(0);
}
