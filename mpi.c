#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <mpi.h>
#include <math.h>

void ordenarPar(int p1, int p2, int *ar);
void combinar(int left, int medio, int right, int *ar);
double dwalltime();
static inline int min(int n1, int n2);

long N;
int num_procs;
int elementos_por_proc;
int resultado = 0;
int rank;
int *aux_local, *a_local, *b_local; //SON GLOBALES, NO COMPARTIDAS

double dwalltime(){
    double sec;
    struct timeval tv;

    gettimeofday(&tv, NULL);
    sec = tv.tv_sec + tv.tv_usec / 1000000.0;
    return sec;
}

static inline int min(int n1, int n2){
    return (n1 < n2) ? n1 : n2;
}

void ordenarPar(int p1, int p2, int *ar){
    int aux1;
    if (ar[p1] > ar[p2]){
        aux1 = ar[p1];
        ar[p1] = ar[p2];
        ar[p2] = aux1;
    }
}

void combinar(int left, int medio, int right, int *ar){
    int i = 0, j = 0, k;
    i = left;
    j = medio + 1;
    for (k = left; k <= right; k++) {
        if (i > medio) aux_local[k] = ar[j++];
        else if (j > right) aux_local[k] = ar[i++];
        else if (ar[i] < ar[j]) aux_local[k] = ar[i++];
        else aux_local[k] = ar[j++];
    }
    // Copiar de vuelta a 'a'
    for (k = left; k <= right; k++) {
        ar[k] = aux_local[k];
    }
}

int main(int argc, char*argv[]){
    int i, n_local;
    double timetick;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    if (argc < 2){
        if (rank == 0) {
            printf("\n Falta un argumento:: N dimension del arreglo, T cantidad de procesos \n");
        }
        MPI_Finalize();
        return 0;
    }
    
    N = atol(argv[1]);

    elementos_por_proc = N / num_procs;
    int num_procs_bk = num_procs;

    if ( rank == 0){
        n_local = N;
    }else{
        n_local = N / pow(2, (int)log2(rank + 2));
    }

    a_local = (int*)malloc(sizeof(int) * n_local);
    b_local = (int*)malloc(sizeof(int) * n_local);
    aux_local = (int*)malloc(sizeof(int) * n_local);
    int *sendcounts;
    int *displs;
    int *a_local_bk, *b_local_bk;
    int * a, *b;
    if (rank == 0) {
        sendcounts = (int*)malloc(sizeof(int) * num_procs);
        displs = (int*)malloc(sizeof(int) * num_procs);
        a = (int*)malloc(sizeof(int) * N);
        b = (int*)malloc(sizeof(int) * N);

        for (i = 0; i < N; i++) {
            b[i] = i;
            aux_local[i] = 0;
            a[i] = (N - 1) - i;
        }
        for (i = 0; i < num_procs; i++) {
            sendcounts[i] = elementos_por_proc;
            displs[i] = i * elementos_por_proc;
        }
        timetick = dwalltime(); //rank 0 mide el tiempo
    }
    
    MPI_Scatterv(a,sendcounts,displs, MPI_INT, a_local, elementos_por_proc, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Scatterv(b,sendcounts,displs, MPI_INT, b_local, elementos_por_proc, MPI_INT, 0, MPI_COMM_WORLD);

    for (int L = 0; L < elementos_por_proc; L += 2) {
        ordenarPar(L, L + 1, a_local);
        ordenarPar(L, L + 1, b_local);
    }


    for (int lenTrabajo = 4; lenTrabajo <= N; lenTrabajo *= 2) {
      if (elementos_por_proc <= n_local && elementos_por_proc != 0) {  
        for (int L = 0; L < elementos_por_proc; L += lenTrabajo) {
            int M = L + lenTrabajo / 2 - 1;
            int R = min(L + lenTrabajo - 1, N - 1);
            combinar(L, M, R, a_local);
            combinar(L, M, R, b_local);
        }
      }
        if (lenTrabajo>=elementos_por_proc && elementos_por_proc < N){
            MPI_Gatherv(a_local, elementos_por_proc, MPI_INT, a, sendcounts, displs, MPI_INT, 0, MPI_COMM_WORLD);
            MPI_Gatherv(b_local, elementos_por_proc, MPI_INT, b, sendcounts, displs, MPI_INT, 0, MPI_COMM_WORLD);
            elementos_por_proc*=2;
            if (rank == 0 && elementos_por_proc < N) {
                num_procs = num_procs / 2;
                for (i = 0; i < num_procs; i++) {
                    sendcounts[i] = elementos_por_proc;
                    displs[i] = i * elementos_por_proc;
                }
                for (i = num_procs; i < num_procs_bk; i++) {
                    sendcounts[i] = 0;
                    displs[i] = 0;
                }
            }
            if (elementos_por_proc < N){
                if (elementos_por_proc > n_local){
                    elementos_por_proc = 0;
                }
                MPI_Scatterv(a,sendcounts,displs, MPI_INT, a_local, elementos_por_proc, MPI_INT, 0, MPI_COMM_WORLD);
                MPI_Scatterv(b,sendcounts,displs, MPI_INT, b_local, elementos_por_proc, MPI_INT, 0, MPI_COMM_WORLD);
            }else{
                if (rank == 0) {
                    a_local_bk = a_local;
                    b_local_bk = b_local;
                    a_local = a; //al arreglo a le falta el ultimo combinar (lo hace rank0)
                    b_local = b;
                }
            }
        }

    }
    
    num_procs = num_procs_bk;
    elementos_por_proc = N / num_procs;

    if (rank == 0) {
        a_local = a_local_bk; //recupera la direc de a_local
        b_local = b_local_bk;
        for (i = 0; i < num_procs; i++) {
            sendcounts[i] = elementos_por_proc;
            displs[i] = i * elementos_por_proc;
        }
    }

    MPI_Scatter(a, elementos_por_proc, MPI_INT, a_local, elementos_por_proc, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Scatter(b, elementos_por_proc, MPI_INT, b_local, elementos_por_proc, MPI_INT, 0, MPI_COMM_WORLD);

    for (i = 0; i < elementos_por_proc; i++) {
        if (a_local[i] != b_local[i]) {
            resultado = 1; 
            break; 
        }
    }
    int global_resultado;
    MPI_Reduce(&resultado, &global_resultado, 1, MPI_INT, MPI_MAX, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        printf("Tiempo en segundos %f\n", dwalltime() - timetick);

        if (global_resultado == 0) {
            printf("Los arreglos son iguales\n");
        } else {
            printf("Los arreglos no son iguales\n");
        }
    }

    free(a_local);
    free(b_local);
    free(aux_local);
    
    if (rank == 0) {
        free(a);
        free(b);
    }

    MPI_Finalize();
    return 0;
}
