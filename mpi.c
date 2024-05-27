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
int *aux_local, *a_local, *b_local, *a, *b;

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
        ////printf("Soy rank %d y aux_local[%d] es %d, asignado a ar\n",rank, k, aux_local[k]);
        ar[k] = aux_local[k];
    }
}

int main(int argc, char*argv[]){
    int i;
    double timetick;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    if (argc < 2){
        if (rank == 0) {
            //printf("\n Falta un argumento:: N dimension del arreglo, T cantidad de procesos \n");
        }
        MPI_Finalize();
        return 0;
    }
    
    N = atol(argv[1]);
    //printf("Hola soy el rank %d y N es %ld\n", rank, N);

    elementos_por_proc = N / num_procs;

    //printf("Soy rank %d y voy a reservar memoria para a_local, b_local y aux_local\n", rank);
    int n_local = N / pow(2, (int)log2(rank + 1));
    //printf("Soy rank %d y n_local es %d\n", rank, n_local);
    a_local = (int*)malloc(sizeof(int) * n_local);
    b_local = (int*)malloc(sizeof(int) * n_local);
    aux_local = (int*)malloc(sizeof(int) * n_local);
    //printf("Soy rank %d y ya reservé memoria para a_local, b_local y aux_local\n", rank);

    if (rank == 0) {
        //printf("Hola soy el rank %d y vot a reservar memoria para a, b y aux\n", rank);
        a = (int*)malloc(sizeof(int) * N);
        b = (int*)malloc(sizeof(int) * N);
        // Inicialización de los arreglos
        //printf("Soy rank %d y voy a inicializar arreglos \n", rank);
        for (i = 0; i < N; i++) {
            b[i] = i;
            aux_local[i] = 0;
            a[i] = (N - 1) - i;
        }
        //printf("Soy rank %d y ya inicialicé arreglos \n", rank);
        timetick = dwalltime();
    }
    

    /*
    MPI_Bcast(a, N, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(b, N, MPI_INT, 0, MPI_COMM_WORLD);
    */
    
    //int inicio = rank * elementos_por_proc;
    //int fin = (rank == num_procs - 1) ? N : (rank + 1) * elementos_por_proc;
    //printf("Soy rank %d y voy a hacer scatter de a\n", rank);
    MPI_Scatter(a, elementos_por_proc, MPI_INT, a_local, elementos_por_proc, MPI_INT, 0, MPI_COMM_WORLD);

    //printf("Soy rank %d y elementos_por_proc es %d\n", rank, elementos_por_proc);

    for (int L = 0; L < elementos_por_proc; L += 2) {
        ordenarPar(L, L + 1, a_local);
    }
    //printf("Soy rank %d y ya ordené a_local con ordenarPar \n", rank);

    for (int lenTrabajo = 4; lenTrabajo <= N; lenTrabajo *= 2) {
        //printf("Soy rank %d y lenTrabajo es %d\n", rank, lenTrabajo);
        for (int L = 0; L < elementos_por_proc; L += lenTrabajo) {
            int M = L + lenTrabajo / 2 - 1;
            int R = min(L + lenTrabajo - 1, N - 1);
            //printf("Soy rank %d y L es %d, M es %d, R es %d\n", rank, L, M, R);
            combinar(L, M, R, a_local);
            //printf("Soy rank %d y ya combiné a_local con combinar \n", rank);
        }
        //MPI_Barrier(MPI_COMM_WORLD);
        if (elementos_por_proc == 0){ 
            //printf("Soy rank %d y elementos_por_proc para mi es 0, SALGO!!!!\n", rank);
            break; 
        }
        if (lenTrabajo>=elementos_por_proc && elementos_por_proc < N){
            //printf("Soy rank %d y voy a hacer gather de a\n", rank);
            //MPI_Barrier(MPI_COMM_WORLD);
            MPI_Gather(a_local, elementos_por_proc, MPI_INT, a, elementos_por_proc, MPI_INT, 0, MPI_COMM_WORLD);
            //printf("Soy rank %d y ya hice gather de a\n", rank);
            elementos_por_proc*=2;
            if (elementos_por_proc < N){
                //printf("Soy rank %d y voy a hacer scatter de a\n", rank);
                MPI_Scatter(a, elementos_por_proc, MPI_INT, a_local, elementos_por_proc, MPI_INT, 0, MPI_COMM_WORLD);
                //printf("Soy rank %d y ya hice scatter de a, recibi elementos por proceso de: %d\n", rank, elementos_por_proc);
            }else{
                if (rank != 0) {
                    elementos_por_proc = 0;
                }else{
                    a_local = a;
                    //printf("Soy rank %d, voy a ordenar todo los %d elementos de a\n", rank, elementos_por_proc);
                    //printf("Hasta este punto, el arreglo a es: \n");
                    for (i = 0; i < N; i++) {
                        //printf("a[%d] es %d\n", i, a[i]);
                    }
                }
            }
        }
    }
    
    if (rank == 0) {
        //printf("Soy rank %d, hemos terminao por ahora, el arreglo a es: \n", rank);
        a = a_local;
                    for (i = 0; i < N; i++) {
                        //printf("a[%d] es %d\n", i, a[i]);
                    }
    }

    elementos_por_proc = N / num_procs;

    MPI_Scatter(b, elementos_por_proc, MPI_INT, b_local, elementos_por_proc, MPI_INT, 0, MPI_COMM_WORLD);

    for (int L = 0; L < elementos_por_proc; L += 2) {
        ordenarPar(L, L + 1, b_local);
    }
    //printf("Soy rank %d y ya ordené b_local con ordenarPar \n", rank);

    for (int lenTrabajo = 4; lenTrabajo <= N; lenTrabajo *= 2) {
        //printf("Soy rank %d y lenTrabajo es %d\n", rank, lenTrabajo);
        for (int L = 0; L < elementos_por_proc; L += lenTrabajo) {
            int M = L + lenTrabajo / 2 - 1;
            int R = min(L + lenTrabajo - 1, N - 1);
            //printf("Soy rank %d y L es %d, M es %d, R es %d\n", rank, L, M, R);
            combinar(L, M, R, b_local);
            //printf("Soy rank %d y ya combiné b_local con combinar \n", rank);
        }
        //MPI_Barrier(MPI_COMM_WORLD);
        if (elementos_por_proc == 0){ 
            //printf("Soy rank %d y elementos_por_proc para mi es 0, SALGO!!!!\n", rank);
            break; 
        }
        if (lenTrabajo>=elementos_por_proc && elementos_por_proc < N){
            //printf("Soy rank %d y voy a hacer gather de b\n", rank);
            //MPI_Barrier(MPI_COMM_WORLD);
            MPI_Gather(b_local, elementos_por_proc, MPI_INT, b, elementos_por_proc, MPI_INT, 0, MPI_COMM_WORLD);
            //printf("Soy rank %d y ya hice gather de b\n", rank);
            elementos_por_proc*=2;
            if (elementos_por_proc < N){
                //printf("Soy rank %d y voy a hacer scatter de b\n", rank);
                MPI_Scatter(b, elementos_por_proc, MPI_INT, b_local, elementos_por_proc, MPI_INT, 0, MPI_COMM_WORLD);
                //printf("Soy rank %d y ya hice scatter de b, recibi elementos por proceso de: %d\n", rank, elementos_por_proc);
            }else{
                if (rank != 0) {
                    elementos_por_proc = 0;
                }else{
                    b_local = b;                
                }
            }
        }
    }

    if (rank == 0) {
        b = b_local;
    }

    //MPI_Barrier(MPI_COMM_WORLD);

    elementos_por_proc = N / num_procs;
    //printf("Soy rank %d y llegue al scatter de a, para recorrer el arreglo y comparar\n", rank);
    MPI_Scatter(a, elementos_por_proc, MPI_INT, a_local, elementos_por_proc, MPI_INT, 0, MPI_COMM_WORLD);
    //printf("Soy rank %d y llegue al scatter de b, para recorrer el arreglo y comparar\n", rank);
    MPI_Scatter(b, elementos_por_proc, MPI_INT, b_local, elementos_por_proc, MPI_INT, 0, MPI_COMM_WORLD);
    //printf("Soy rank %d y ya hice scatter de a y b\n", rank);
    /*
    for (i = 0; i < elementos_por_proc; i++) {
       //printf("Soy rank %d y a_local[%d] es %d\n", rank, i, a_local[i]);
    }
    for (i = 0; i < elementos_por_proc; i++) {
       //printf("Soy rank %d y b_local[%d] es %d\n", rank, i, b_local[i]);
    }
    */
    for (i = 0; i < elementos_por_proc; i++) {
        if (a_local[i] != b_local[i]) {
            resultado = 1; 
            break; 
        }
    }
    //printf("Soy rank %d y ya recorrí los arreglos, resultado para mi es %d \n", rank, resultado);
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
