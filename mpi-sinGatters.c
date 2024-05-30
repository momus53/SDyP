#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <mpi.h>
#include <math.h>

void ordenarPar(int p1, int p2, int *ar);
void combinar(int left, int medio, int right, int *ar);
double dwalltime();
static inline int min(int n1, int n2);

int N;
int num_procs;
int elementos_por_proc, elementos_por_recibir;
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
    
    int *a_bk, *b_bk;
    int * a, *b;
    if (rank == 0) {
        a = (int*)malloc(sizeof(int) * N);
        b = (int*)malloc(sizeof(int) * N);
        for (i = 0; i < N; i++) {
            b[i] = i;
            aux_local[i] = 0;
            a[i] = (N - 1) - i;
        }
        timetick = dwalltime(); //rank 0 mide el tiempo
    }
    
    //cambiar por scatter normal
    MPI_Scatter(a, elementos_por_proc, MPI_INT, a_local, elementos_por_proc, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Scatter(b, elementos_por_proc, MPI_INT, b_local, elementos_por_proc, MPI_INT, 0, MPI_COMM_WORLD);

    for (int L = 0; L < elementos_por_proc; L += 2) {
        ordenarPar(L, L + 1, a_local);
        ordenarPar(L, L + 1, b_local);
    }

    for (int lenTrabajo = 4; lenTrabajo <= N; lenTrabajo *= 2) {
        
        for (int L = 0; L < elementos_por_proc; L += lenTrabajo) {
            int M = L + lenTrabajo / 2 - 1;
            int R = min(L + lenTrabajo - 1, N - 1);
            combinar(L, M, R, a_local);
            combinar(L, M, R, b_local);
        }
        
        if (lenTrabajo>=elementos_por_proc && elementos_por_proc < N){
            if (rank != 0){ //el 0 solo recibe, no envia
       //         printf("rank %d envia a %d, la cantidad de %d elementos\n", rank, rank/2, elementos_por_proc);
       //         for (int i = 0; i < elementos_por_proc; i++) {
      //              printf("RANK: %d SEND to RANK%d A_LOCAL[%d/%d]=%d \n",rank,rank/2,i,elementos_por_proc, a_local[i]);
     //           }
                MPI_Send(a_local, elementos_por_proc, MPI_INT, rank/2, 0, MPI_COMM_WORLD);
                MPI_Send(b_local, elementos_por_proc, MPI_INT, rank/2, 0, MPI_COMM_WORLD);
            }
            elementos_por_recibir = elementos_por_proc;
            elementos_por_proc*=2;
            
            if(elementos_por_proc <= n_local ){ // si el proceso tiene espacio para recibir elementos_por_proc*2 entonces entra
                if (rank == 0) {
                    MPI_Recv (&a_local[elementos_por_recibir], elementos_por_recibir, MPI_INT, 1 ,MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE); // 0 recibe siempre de 1
                    MPI_Recv (&b_local[elementos_por_recibir], elementos_por_recibir, MPI_INT, 1 ,MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
           //         for (int i = 0; i < n_local; i++) {
         //               printf("RANK: %d RECIEVE FROM RANK%d A_LOCAL[%d] queda: =%d \n",rank,1,i, a_local[i]);
       //             }
                }else{
                    MPI_Recv (a_local, elementos_por_recibir, MPI_INT, rank*2 , MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    MPI_Recv (b_local, elementos_por_recibir, MPI_INT, rank*2 , MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
     //               for (int i = 0; i < n_local; i++) {
 //                       printf("RANK: %d RECIEVE FROM RANK%d A_LOCAL[%d] queda: =%d \n",rank,rank*2,i, a_local[i]);
   //                 }
                    MPI_Recv (&a_local[elementos_por_recibir], elementos_por_recibir, MPI_INT, rank*2+1 , MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    MPI_Recv (&b_local[elementos_por_recibir], elementos_por_recibir, MPI_INT, rank*2+1 , MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
//                    for (int i = 0; i < n_local; i++) {
//                        printf("RANK: %d RECIEVE FROM RANK%d A_LOCAL[%d] queda: =%d \n",rank,rank*2+1,i, a_local[i]);
//                    }
                }
            }else{ //No tengo mas trabajo que hacer.
//                printf("RANK %d NO TIENE MAS TRABAJO QUE HACER SALGO!\n", rank);
                break; // el 0 siempre tiene trabajo que hacer.
            }

        }

    }
    
    elementos_por_proc = N / num_procs;

    if (rank == 0) {
        a_bk = a; //guarda la direccion de memoria de a
        a = a_local; //cambia a por a_local que ahora es el arreglo a pero ordenado
        a_local = a_bk; //a_local ahora va a ser el anterior espacio de a
        b_bk = b;
        b = b_local;
        b_local = b_bk;
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
