#include<stdio.h>
#include<stdlib.h>
#include <sys/time.h>
#include <pthread.h>

void* ordenarIterativo(void *arg);
void ordenarPar(int, int, double *a);
void combinar(int left, int medio, int right, double *a);

double *aux, *a, *b;
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
    int iniciobk = inicio;
    int finbk = fin;
    int lenTrabajo, L, M, R,i;    
    printf("Hilo id:%d, inicio:%d, fin: %d \n",id, inicio, fin);
    // Ordenar pares
    for (L=inicio; L < fin; L+=2){
        //printf("Soy el hilo %d, ordenando %d y %d\n", id, L, L+1);
        ordenarPar(L, L+1, a);
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
                combinar(L, M, R, a);
            }

            if (lenTrabajo>=elementos_por_hilo){
                fin *= 2;
                inicio *= 2;
                //printf("Hilo id:%d, incremento fin a %d e inicio a %d \n",id, fin, inicio);
            }
        }//else{
        //    break;
        //}
        pthread_barrier_wait(&barrera);
        //printf("Hilo id:%d, LLEGUE BARREARA\n",id);
        //pthread_barrier_wait(&barrera);
    }
        //--------------------------------B-----------------------------------------------------------
    //ajustamos nuevamente inicio y fin originales
    inicio = iniciobk;
    fin = finbk;

    for (L=inicio; L < fin; L+=2){
        //printf("Soy el hilo %d, ordenando %d y %d\n", id, L, L+1);
        ordenarPar(L, L+1, b);
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
                combinar(L, M, R, b);
            }

            if (lenTrabajo>=elementos_por_hilo){
                fin *= 2;
                inicio *= 2;
                //printf("Hilo id:%d, incremento fin a %d e inicio a %d \n",id, fin, inicio);
            }
        }
        pthread_barrier_wait(&barrera);
        
    } //cuando sale del for lenTrabajo es mayor a N/num_threads

    inicio = iniciobk;
    fin = finbk;
    for (int i = 0; i < N; i++) {
        if (a[i] != b[i]) {
            resultado=1; // Los arreglos no son iguales
            break; // No es necesario seguir comparando
        }
    }

    pthread_exit(NULL);
}

void inline ordenarPar(int p1, int p2, double *ar){
    double aux1;
    if (ar[p1] > ar[p2]){
        aux1 = ar[p1];
        ar[p1] = ar[p2];
        ar[p2] = aux1;
    }
}

void combinar(int left, int medio, int right, double *ar){
    //int len1 = medio - left + 1;
    //int len2 = right - medio;
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
    timetick = dwalltime();
    for(int id=0;id<num_threads;id++){
        threads_ids[id]=id;
        pthread_create(&misThreads[id],NULL,&ordenarIterativo,(void*)&threads_ids[id]);
    }
    for(int id=0;id<num_threads;id++){
        pthread_join(misThreads[id],NULL);
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
