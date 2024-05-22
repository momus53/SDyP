#include<stdio.h>
#include<stdlib.h>
#include <sys/time.h>
#include <pthread.h>

void* ordenarIterativoA(void *arg);
void ordenarParA(int, int);
void combinarA(int left, int medio, int right);
void ordenarIterativoB(int *);
void ordenarParB(int, int);
void combinarB(int left, int medio, int right);

double *aux, *a, *b;
long N;
int num_threads ;
int elementos_por_hilo;

pthread_barrier_t barrera;

double dwalltime(){
	double sec;
	struct timeval tv;

	gettimeofday(&tv,NULL);
	sec = tv.tv_sec + tv.tv_usec/1000000.0;
	return sec;
}

static inline int min(int n1, int n2){
    return (n1 < n2) ? n1 : n2;
}

void* ordenarIterativoA(void *arg){
    int id=*(int*)arg;
    int inicio = id * elementos_por_hilo;
    int fin = (id == num_threads - 1) ? N : (id + 1) * elementos_por_hilo;
    int lenTrabajo, L, M, R,i;    
    printf("Hilo id:%d, inicio:%d, fin: %d \n",id, inicio, fin);
    // Ordenar pares
    for (L=inicio; L < fin; L+=2){
        //printf("Soy el hilo %d, ordenando %d y %d\n", id, L, L+1);
        ordenarParA(L, L+1);
    }
    //pthread_barrier_wait(&barrera);

    for (lenTrabajo=4; lenTrabajo <= N; lenTrabajo *= 2){
        // En ultimo len: L = 0, 16 // M = 7, 23 // R = 15, 29
        if (fin <= N){
            for (L=inicio; L < fin; L += lenTrabajo){ //los hilos seran cada vez menos ejecutando a medida que lenTrabajo aumente de forma que id*elementos_por_hilo sea mayor que elementos_por_hilo-1 desde el primer valor
                M = L + lenTrabajo/2 - 1;
                //M = min(L + lenTrabajo / 2 - 1, fin);
                //if (M >= lenTrabajo-1) break;    // ya está ordenado
                R = min(L + lenTrabajo - 1, N-1);
                //printf("Hilo id:%d, inicio:%d, fin: %d, lenTrabajo: %d, M:%d, R:%d, L:%d \n",id, inicio, fin, lenTrabajo, M, R, L);
                combinarA(L, M, R);
            }
            if (lenTrabajo>=elementos_por_hilo){
                fin *= 2;
                inicio *= 2;
                //printf("Hilo id:%d, incremento fin a %d e inicio a %d \n",id, fin, inicio);
            }
        }
        //printf("Hilo id:%d, LLEGUE BARREARA\n",id);
        pthread_barrier_wait(&barrera);
    } //cuando sale del for lenTrabajo es mayor a N/num_threads
    pthread_exit(NULL);
}

void inline ordenarParA(int p1, int p2){
    double aux1;
    if (a[p1] > a[p2]){
        aux1 = a[p1];
        a[p1] = a[p2];
        a[p2] = aux1;
    }
}

void combinarA(int left, int medio, int right){
    //int len1 = medio - left + 1;
    //int len2 = right - medio;
    int i = 0, j = 0, k;

    for (i = left, j = medio + 1, k = left; k <= right; k++) {
        if (i > medio) aux[k] = a[j++];
        else if (j > right) aux[k] = a[i++];
        else if (a[i] < a[j]) aux[k] = a[i++];
        else aux[k] = a[j++];
    }

    // Copiar de vuelta a 'a'
    for (k = left; k <= right; k++) {
        a[k] = aux[k];
    }
}

int main(int argc, char*argv[]){
    int resultado = 0;
	double timetick;
	N = atol(argv[1]);
    int i;
	num_threads = atoi(argv[2]);
    elementos_por_hilo = N / num_threads;
    pthread_barrier_init(&barrera,NULL, num_threads);
    pthread_t misThreads[num_threads];
    int threads_ids[num_threads];
	a = (double*)malloc(sizeof(double)*N);
	b = (double*)malloc(sizeof(double)*N);
	aux = (double*)malloc(sizeof(double)*N);

	//inicializacion de los arreglos iguales 
    printf("inicializando arreglos\n");
	for(i=0;i<N;i++){
		b[i] = i;
		aux[i] = i;
		a[i] = (N-1)-i;        
 	}
    /*
    printf("Arreglos originales:\n");
    for (i = 0; i < N; i++) {
        printf("%.1f ", a[i]);
    }
    printf("\n");
    for (i = 0; i < N; i++) {
        printf("%.1f ", b[i]);
    }
    printf("\n");
    */
    printf("Creando hilos:\n");
    printf("Ordenando arreglos:\n");
    for(int id=0;id<num_threads;id++){
        threads_ids[id]=id;
        pthread_create(&misThreads[id],NULL,&ordenarIterativoA,(void*)&threads_ids[id]);
    }
    
    timetick = dwalltime();
    for(int id=0;id<num_threads;id++){
        pthread_join(misThreads[id],NULL);
    } 	
    
    for (int i = 0; i < N; i++) {
        if (a[i] != b[i]) {
            resultado=1; // Los arreglos no son iguales
            break; // No es necesario seguir comparando
        }
    }
    printf("Tiempo en segundos %f\n", dwalltime() - timetick);
    /*
    for (i = 0; i < N; i++) {
        printf("%.1f ", a[i]);
    }
 	printf("\n");
    for (i = 0; i < N; i++) {
        printf("%.1f ", b[i]);
    }
 	printf("\n");
	*/

    if (resultado==0) {
        printf("Los arreglos son iguales\n");
    } else {
        printf("Los arreglos no son iguales\n");
    }

 	free(a);
 	free(b);
 	free(aux);
    pthread_barrier_destroy(&barrera);
 return 0;
}
