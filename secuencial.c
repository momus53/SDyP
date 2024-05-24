#include<stdio.h>
#include<stdlib.h>
#include <sys/time.h>

void ordenarIterativo();
void ordenarPar(int, int, int *a);
void combinar(int left, int medio, int right, int *a);


int *aux, *a, *b;
int N;
int resultado = 0;

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

void ordenarIterativo(int *ar){
    int lenTrabajo, L, M, R,i;

    // Ordenar pares
    for (L=0; L < N-1; L+=2){
        ordenarPar(L, L+1, ar);
    }

    for (lenTrabajo=4; lenTrabajo <= N; lenTrabajo *= 2){

        for (L=0; L < N-1; L += lenTrabajo){
            M = L + lenTrabajo/2 - 1;
            //if (M >= N-1) break;    // ya está ordenado 
            R = min(L + lenTrabajo - 1, N-1);
            combinar(L, M, R, ar);
        }
    }
    
    if (lenTrabajo != (N*2)) { // permite que el arreglo se ordene en caso de que N no sea potencia de 2 
        M = (int)(lenTrabajo/2);
        combinar(0, M-1, N-1, ar);
    }
    
}

void ordenarPar(int p1, int p2, int *ar){
    double aux1;
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
    for ( k = left; k <= right; k++) {
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
	int r, i;
	
	a = (int*)malloc(sizeof(int)*N);
	b = (int*)malloc(sizeof(int)*N);
	
	aux = (int*)malloc(sizeof(int)*N);

	//inicializacion de los arreglos iguales 
	for(i=0;i<N;i++){
		a[i] = i;
		aux[i] = 0;
		b[i] = (N-1)-i;        
 	}
   
	timetick = dwalltime(); //arranca a contar 	
 	
 	ordenarIterativo(a);
    ordenarIterativo(b);

    for (int i = 0; i < N; i++) {
        if (a[i] != b[i]) {
            resultado=1; // Los arreglos no son iguales
            break; // No es necesario seguir comparando
        }
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
 return 0;
}
