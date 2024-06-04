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

void* ordenarIterativo(void *arg){
	int id=*(int*)arg;
	int inicio = id * elementos_por_hilo;
	int fin = (id == num_threads - 1) ? N : (id + 1) * elementos_por_hilo;
	int lenTrabajo, L, M, R,i;   
	// Ordenar pares
	for (L=inicio; L < fin; L+=2){
		ordenarPar(L, L+1, a);
	}
	//Ordenamos el arreglo con merge-sort
	for (lenTrabajo=4; lenTrabajo <= N; lenTrabajo *= 2){
		if (trabajo_hilo[id] >= lenTrabajo){ //Solo ordenan aquellos hilos que tienen la longitud de trabajo suficiente (distribuida previamente en trabajo_hilo[k] en el main)
	            	for (L=inicio; L < fin; L += lenTrabajo){ //Se ordena de a bloques de lenTrabajo la seccion del arreglo que le toco a hilo.
	                	M = L + lenTrabajo/2 - 1;
	                	R = L + lenTrabajo - 1;
	                	combinar(L, M, R, a); //Se ordena el sub-arreglo
	            	}
	
	        	if (lenTrabajo>=elementos_por_hilo){
	                	fin += lenTrabajo; //se incrementa la seccion de trabajo del arreglo
	            	}
        	} 
        pthread_barrier_wait(&barrera); //Espera antes que los demas hilos terminen de trabajar para aumentar la seccion de trabajo
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
	                	R = L + lenTrabajo - 1;
	                	combinar(L, M, R, b);
	        	}
				
	        	if (lenTrabajo>=elementos_por_hilo){
	        		fin += lenTrabajo;
	        	}
		}
	pthread_barrier_wait(&barrera);
	}

	fin = (id == num_threads - 1) ? N : (id + 1) * elementos_por_hilo;
	
	// Comparamos los arreglos ya ordenados
	for (int i = inicio; i < fin; i++) {
        	if (a[i] != b[i]) {
			resultado=1; // Los arreglos no son iguales
			break; // No es necesario seguir comparando
		}
	}
	pthread_exit(NULL);
}

//Funcion para ordenar un sub-arreglo de dos elementos.
void ordenarPar(int p1, int p2, int *ar){
    int aux1;
    if (ar[p1] > ar[p2]){
        aux1 = ar[p1];
        ar[p1] = ar[p2];
        ar[p2] = aux1;
    }
}

//Funcion para ordenar un sub-arreglo en base a los limites y el arreglo pasados por parametro.
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
	trabajo_hilo = (int*)malloc(sizeof(int)*num_threads); //cantidad de trabajo que le tocara a cada hilo

	//inicializacion de los arreglos iguales 
	for(int i=0;i<N;i++){
	b[i] = (N-1)-i;
        aux[i] = 0;
	a[i] = (N-1)-i;        
 	}
//Definimos el tamaño de trabajo para cada hilo.
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
 	free(a);
 	free(b);
  	free(aux);
    pthread_barrier_destroy(&barrera);

 return 0;
}
