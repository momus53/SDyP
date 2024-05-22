#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include <math.h>

//#define DEBUG

void* ordenarIterativo(void* arg);
void ordenarPar(int, int, double*);
void combinar(int left, int medio, int right, double*);

double *aux, *a, *b;
int N, num_threads,chunk_size;
double timetick;

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

void ordenarPar(int p1, int p2, double* array){
    double aux1;
    if (array[p1] > array[p2]){
        aux1 = array[p1];
        array[p1] = array[p2];
        array[p2] = aux1;
    }
}

void combinar(int left, int medio, int right, double* array){
    int len1 = medio - left + 1;
    int len2 = right - medio;
    int i = 0, j = 0, k;

    for (i = left, j = medio + 1, k = left; k <= right; k++) {
        if (i > medio) aux[k] = array[j++];
        else if (j > right) aux[k] = array[i++];
        else if (array[i] < array[j]) aux[k] = array[i++];
        else aux[k] = array[j++];
    }

    // Copiar de vuelta a 'array'
    for (k = left; k <= right; k++) {
        array[k] = aux[k];
    }
}

void* ordenarIterativo(void* arg){

    timetick = dwalltime(); // Empieza a contar el tiempo

    int id = *((int*)arg);
    int lenTrabajo, L, M, R;
    int start = id * chunk_size;
    int end = (id == num_threads - 1) ? N - 1 : (start + chunk_size - 1);

    ////////////////////////////////PRIMER MERGE////////////////////////////////////////
    
    // Ordenar pares dentro del chunk del hilo
    for (L = start; L < end; L += 2){
        ordenarPar(L, min(L + 1, end), a);
        //ordenarPar(L, min(L + 1, end), b);
    }

    for (lenTrabajo = 4; lenTrabajo <= chunk_size * 2; lenTrabajo *= 2){
        for (L = start; L < end; L += lenTrabajo){
            M = min(L + lenTrabajo / 2 - 1, end);
            if (M >= end) break;
            R = min(L + lenTrabajo - 1, end);
            combinar(L, M, R, a);
            //combinar(L, M, R, b);
        }
    }
    
    ////////////////////////////////SEGUNDO MERGE////////////////////////////////////////
    
    for (L = start; L < end; L += 2){
        //ordenarPar(L, min(L + 1, end), a);
        ordenarPar(L, min(L + 1, end), b);
    }

    for (lenTrabajo = 4; lenTrabajo <= chunk_size * 2; lenTrabajo *= 2){
        for (L = start; L < end; L += lenTrabajo){
            M = min(L + lenTrabajo / 2 - 1, end);
            if (M >= end) break;
            R = min(L + lenTrabajo - 1, end);
            //combinar(L, M, R, a);
            combinar(L, M, R, b);
        }
    }
    printf("Tiempo en segundos del hilo %d : %f\n",id, dwalltime() - timetick);
    pthread_exit(NULL);
    
}

int main(int argc, char* argv[]){
    N = atol(argv[1]);
    num_threads = atoi(argv[2]);
    chunk_size = N / num_threads;
    a = (double*)malloc(sizeof(double) * N);
    b = (double*)malloc(sizeof(double) * N);
    aux = (double*)malloc(sizeof(double) * N);

    // Inicialización de los arreglos
    for(int i = 0; i < N; i++){
	/*a[i] = i;
	aux[i] = i;
	b[i] = (N-1)-i;  */
	a[i] = rand() % 100; // Usar valores aleatorios para mayor variabilidad
        aux[i] = a[i];
        b[i] = a[i];
    }
    #ifdef DEBUG
    for(int i = 0; i < N; i++){
    	printf("%.1f ", a[i]);
    }
    printf("\n");
    for(int i = 0; i < N; i++){
    	printf("%.1f ", b[i]);
    }
    printf("\n");
    #endif
    pthread_t threads[num_threads];
    int thread_ids[num_threads];

    //////////////////////////////

    for (int i = 0; i < num_threads; i++){
        thread_ids[i] = i;
        pthread_create(&threads[i], NULL, ordenarIterativo, (void*)&thread_ids[i]);
    }

    for (int i = 0; i < num_threads; i++){
        pthread_join(threads[i], NULL);
    }

    //////////////////////////////

    // Combinar los resultados de los hilos
    for (int lenTrabajo = chunk_size; lenTrabajo < N; lenTrabajo *= 2){
        for (int L = 0; L < N - 1; L += 2 * lenTrabajo){
            int M = min(L + lenTrabajo - 1, N - 1);
            int R = min(L + 2 * lenTrabajo - 1, N - 1);
            combinar(L, M, R, a);
            combinar(L, M, R, b);
        }
    }

    int resultado = 0;
    for (int i = 0; i < N; i++){
        if (a[i] != b[i]){
            resultado = 1; // Los arreglos no son iguales
            printf("Se ha salido en la posicion %d\n", i);
            break; // No es necesario seguir comparando
        }
    }
    #ifdef DEBUG
    for(int i = 0; i < N; i++){
    	printf("%.1f ", a[i]);
    }
    printf("\n");
    for(int i = 0; i < N; i++){
    	printf("%.1f ", b[i]);
    }
    printf("\n");
    #endif
    if (resultado == 0) {
        printf("Los arreglos son iguales\n");
    } else {
        printf("Los arreglos no son iguales\n");
    }

    free(a);
    free(b);
    free(aux);
    return 0;
}

