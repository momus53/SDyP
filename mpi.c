#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <mpi.h>

void ordenarPar(int p1, int p2, int *ar);
void combinar(int left, int medio, int right, int *ar);
double dwalltime();
static inline int min(int n1, int n2);

int *aux, *a, *b;
long N;
int num_procs;
int elementos_por_proc;
int resultado = 0;

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
        if (i > medio) aux[k] = ar[j++];
        else if (j > right) aux[k] = ar[i++];
        else if (ar[i] < ar[j]) aux[k] = ar[i++];
        else aux[k] = ar[j++];
    }
    // Copiar de vuelta a 'a'
    for (k = left; k <= right; k++) {
        ar[k] = aux[k];
    }
}

int main(int argc, char*argv[]){
    int rank, i;
    double timetick;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    if (argc < 3){
        if (rank == 0) {
            printf("\n Falta un argumento:: N dimension del arreglo, T cantidad de procesos \n");
        }
        MPI_Finalize();
        return 0;
    }
    
    N = atol(argv[1]);
    num_procs = atoi(argv[2]);

    elementos_por_proc = N / num_procs;

    if (rank == 0) {
        a = (int*)malloc(sizeof(int) * N);
        b = (int*)malloc(sizeof(int) * N);
        aux = (int*)malloc(sizeof(int) * N);
        // Inicialización de los arreglos
        printf("inicializando arreglos\n");
        for (i = 0; i < N; i++) {
            b[i] = i;
            aux[i] = 0;
            a[i] = (N - 1) - i;
        }
        printf("ordenando");
        timetick = dwalltime();
    }

    n_local = N / pow(2, (int)log2(rank + 1));
    a_local = (int*)malloc(sizeof(int) * n_local);
    b_local = (int*)malloc(sizeof(int) * n_local);
    aux_local = (int*)malloc(sizeof(int) * n_local);
    /*
    MPI_Bcast(a, N, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(b, N, MPI_INT, 0, MPI_COMM_WORLD);
    */
    
    //int inicio = rank * elementos_por_proc;
    //int fin = (rank == num_procs - 1) ? N : (rank + 1) * elementos_por_proc;
    
    int MPI_Scatter(a, elementos_por_proc, MPI_INT, a_local, elementos_por_proc, MPI_INT, 0, MPI_COMM_WORLD);

    for (int L = inicio; L < fin; L += 2) {
        ordenarPar(L, L + 1, a_local);
    }

    for (int lenTrabajo = 4; lenTrabajo <= N; lenTrabajo *= 2) {
        for (int L = 0; L < elementos_por_proc; L += lenTrabajo) {
            int M = L + lenTrabajo / 2 - 1;
            int R = min(L + lenTrabajo - 1, N - 1);
            combinar(L, M, R, a_local);
        }
        //MPI_Barrier(MPI_COMM_WORLD);
        if (lenTrabajo>=elementos_por_proc && elementos_por_proc < N){
            int MPI_Gather(a_local, elementos_por_proc, MPI_INT, a, elementos_por_proc, MPI_INT, 0, MPI_COMM_WORLD);
            elementos_por_proc*=2;
            if (elementos_por_proc < N){
                int MPI_Scatter(a, elementos_por_proc, MPI_INT, a_local, elementos_por_proc, MPI_INT, 0, MPI_COMM_WORLD);
            }
        }
    }
    
    if (rank == 0) {
        a = a_local;
    }

    elementos_por_proc = N / num_procs;

    int MPI_Scatter(b, elementos_por_proc, MPI_INT, b_local, elementos_por_proc, MPI_INT, 0, MPI_COMM_WORLD);

    for (int L = inicio; L < fin; L += 2) {
        ordenarPar(L, L + 1, b_local);
    }

    for (int lenTrabajo = 4; lenTrabajo <= N; lenTrabajo *= 2) {
        for (int L = 0; L < elementos_por_proc; L += lenTrabajo) {
            int M = L + lenTrabajo / 2 - 1;
            int R = min(L + lenTrabajo - 1, N - 1);
            combinar(L, M, R, b_local);
        }
        //MPI_Barrier(MPI_COMM_WORLD);
        if (lenTrabajo>=elementos_por_proc && elementos_por_proc < N){
            int MPI_Gather(b_local, elementos_por_proc, MPI_INT, b, elementos_por_proc, MPI_INT, 0, MPI_COMM_WORLD);
            elementos_por_proc*=2;
            if (elementos_por_proc < N){
                int MPI_Scatter(b, elementos_por_proc, MPI_INT, b_local, elementos_por_proc, MPI_INT, 0, MPI_COMM_WORLD);
            }
        }
    }
    
    if (rank == 0) {
        b = b_local;
    }
    MPI_Barrier(MPI_COMM_WORLD);

    elementos_por_proc = N / num_procs;

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
        free(aux);
    }

    MPI_Finalize();
    return 0;
}
