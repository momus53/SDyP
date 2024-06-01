#include<stdio.h>
#include<stdlib.h>
#include <sys/time.h>
#include <pthread.h>

void* ordenarIterativo(void *arg);
void ordenarPar(int, int, int *a);
void combinar(int left, int medio, int right, int *a);

int *aux, *a, *b, *trabajo_hilo;
long N;
int num_threads ;
int elementos_por_hilo;
int resultado = 0;
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

void* ordenarIterativo(void *arg){
    int id=*(int*)arg;
    int inicio = id * elementos_por_hilo;
    int fin = (id == num_threads - 1) ? N : (id + 1) * elementos_por_hilo;
    int lenTrabajo, L, M, R,i;   
    // Ordenar pares
    for (L=inicio; L < fin; L+=2){
        ordenarPar(L, L+1, a);
    }

    for (lenTrabajo=4; lenTrabajo <= N; lenTrabajo *= 2){
        if (trabajo_hilo[id] >= lenTrabajo){
            for (L=inicio; L < fin; L += lenTrabajo){ //los hilos seran cada vez menos ejecutando a medida que lenTrabajo aumente
                M = L + lenTrabajo/2 - 1;
                R = min(L + lenTrabajo - 1, N-1);
                combinar(L, M, R, a);
            }

            if (lenTrabajo>=elementos_por_hilo){
            //printf("soy hilo %d mi trabajo es %d y lenTrabajo es %d \n",id,trabajo_hilo[id],lenTrabajo);
                fin *= 2;
            }
        } //else no se puede salir por la barrera
        pthread_barrier_wait(&barrera);
    }
    
//--------------------------------------------------- arreglo B -----------------------------------------------------------
    //ajustamos nuevamente fin original
    fin = (id == num_threads - 1) ? N : (id + 1) * elementos_por_hilo;

    for (L=inicio; L < fin; L+=2){
        ordenarPar(L, L+1, b);
    }

    for (lenTrabajo=4; lenTrabajo <= N; lenTrabajo *= 2){
        if (trabajo_hilo[id] >= lenTrabajo){
            for (L=inicio; L < fin; L += lenTrabajo){ //los hilos seran cada vez menos ejecutando a medida que lenTrabajo aumente
                M = L + lenTrabajo/2 - 1;
                R = min(L + lenTrabajo - 1, N-1);
                combinar(L, M, R, b);
            }

            if (lenTrabajo>=elementos_por_hilo){
                fin *= 2;
            }
        }
        pthread_barrier_wait(&barrera);

    }

    // Comparamos los arreglos
    fin = (id == num_threads - 1) ? N : (id + 1) * elementos_por_hilo;
    
    for (int i = inicio; i < fin; i++) {
        if (a[i] != b[i]) {
            resultado=1; // Los arreglos no son iguales
            break; // No es necesario seguir comparando
        }
    }
    
        pthread_exit(NULL);
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

    if (argc < 3){
        printf("\n Falta un argumento:: N dimension del arreglo, T cantidad de hilos \n");
        return 0;
    }
    N = atol(argv[1]);
    num_threads = atoi(argv[2]);

    double timetick;
    elementos_por_hilo = N / num_threads;

    pthread_barrier_init(&barrera,NULL, num_threads);
    pthread_t misThreads[num_threads];
    int threads_ids[num_threads];

	a = (int*)malloc(sizeof(int)*N);
	b = (int*)malloc(sizeof(int)*N);
	aux = (int*)malloc(sizeof(int)*N);
	trabajo_hilo = (int*)malloc(sizeof(int)*num_threads);

	//inicializacion de los arreglos iguales 
	for(int i=0;i<N;i++){
		b[i] = i;
                aux[i] = 0;
		a[i] = (N-1)-i;        
 	}
    for (int k=0; k<num_threads; k++){
            if ( k == 0){
            trabajo_hilo[k] = N;
            }else{
                for (int i=num_threads/2; i>0; i/=2){
                    if (k % i == 0){
                        trabajo_hilo[k] = N / (num_threads/i);
                        break;
                    }
                }

            }
    }
    //printf("ordenando\n");
    timetick = dwalltime();

    for(int id=0;id<num_threads;id++){
        threads_ids[id]=id;
        pthread_create(&misThreads[id],NULL,&ordenarIterativo,(void*)&threads_ids[id]);
    }
    for(int id=0;id<num_threads;id++){
        pthread_join(misThreads[id],NULL);
    } 	

    printf("Tiempo en segundos %f\n", dwalltime() - timetick);

    if (resultado==0) {
        printf("Los arreglos son iguales\n");
    } else {
        printf("Los arreglos no son iguales\n");
    }
    
    /*
    printf("A: ");
    for(int i=0;i<N;i++){
      printf("%d ",a[i]);      
    }
    printf("\nB: ");
    for(int i=0;i<N;i++){
      printf("%d ",b[i]);      
    }*/
    
 	free(a);
 	free(b);
 	free(aux);
    pthread_barrier_destroy(&barrera);

 return 0;
}
