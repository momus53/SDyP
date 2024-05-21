#include<stdio.h>
#include<stdlib.h>
#include <sys/time.h>


void ordenarIterativoA();
void ordenarParA(int, int);
void combinarA(int left, int medio, int right);
void ordenarIterativoB();
void ordenarParB(int, int);
void combinarB(int left, int medio, int right);

double *aux, *a, *b;
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

void ordenarIterativoA(){
    int lenTrabajo, L, M, R,i;

    // Ordenar pares
    for (L=0; L < N-1; L+=2){
        ordenarParA(L, L+1);
    }
    
    // 4, 8, 16
    for (lenTrabajo=4; lenTrabajo <= N; lenTrabajo *= 2){
        // En ultimo len: L = 0, 16 // M = 7, 23 // R = 15, 29
        for (L=0; L < N-1; L += lenTrabajo){
            M = L + lenTrabajo/2 - 1;
            //if (M >= N-1) break;    // ya está ordenado // por que sale por aca y no poniendo lentTrabajo < N en lugar de <=???
            R = min(L + lenTrabajo - 1, N-1);
            combinarA(L, M, R);
        }
    }
    int P=(N);
    if (lenTrabajo != (P*2)) { //POR QUE SI EN LUGAR DE COMPARAR CON P*2 COMPARO CON N*2 TARDA 44 SEUGNDOS!!!!!
        M = (int)(lenTrabajo/2);
        printf("M: %d\n", M);
        combinarA(0, M-1, N-1);
    }

}

void ordenarParA(int p1, int p2){
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
    i = left;
    j = medio + 1;
    for ( k = left; k <= right; k++) {
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


void ordenarIterativoB(){
    int lenTrabajo, L, M, R,i;

    // Ordenar pares
    for (L=0; L < N-1; L+=2){
        ordenarParB(L, L+1);
    }
    
    // 4, 8, 16
    for (lenTrabajo=4; lenTrabajo <= N; lenTrabajo *= 2){
        // En ultimo len: L = 0, 16 // M = 7, 23 // R = 15, 29
        for (L=0; L < N-1; L += lenTrabajo){
            M = L + lenTrabajo/2 - 1;
            //if (M >= N-1) break;    // ya está ordenado

            R = min(L + lenTrabajo - 1, N-1);
            combinarB(L, M, R);
        }
    }
    int P=(N);
    if (lenTrabajo != (P*2)) {
        M = (int)(lenTrabajo/2);
        printf("M: %d\n", M);
        combinarB(0, M-1, N-1);
    }

}

void ordenarParB(int p1, int p2){
    double aux1;
    
    if (b[p1] > b[p2]){
        aux1 = b[p1];
        b[p1] = b[p2];
        b[p2] = aux1;
    }
}

void combinarB(int left, int medio, int right){
    int len1 = medio - left + 1;
    int len2 = right - medio;
    int i = 0, j = 0, k;
    i = left;
    j = medio + 1;
    for (k = left; k <= right; k++) {
        if (i > medio) aux[k] = b[j++];
        else if (j > right) aux[k] = b[i++];
        else if (b[i] < b[j]) aux[k] = b[i++];
        else aux[k] = b[j++];
    }

    // Copiar de vuelta a 'b'
    for (k = left; k <= right; k++) {
        b[k] = aux[k];
    }
}

int main(int argc, char*argv[]){
	double timetick;
	N = atol(argv[1]);
	int r, i;
	
	a = (double*)malloc(sizeof(double)*N);
	b = (double*)malloc(sizeof(double)*N);
	
	aux = (double*)malloc(sizeof(double)*N);

	//inicializacion de los arreglos iguales 
	for(i=0;i<N;i++){
		a[i] = i;
		aux[i] = i;
		b[i] = (N-1)-i;        
 	}

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
   
	timetick = dwalltime(); //arranca a contar 	
 	
 	ordenarIterativoA();
    ordenarIterativoB();

    for (int i = 0; i < N; i++) {
        if (a[i] != b[i]) {
            resultado=1; // Los arreglos no son iguales
            printf("Se ha salido en la posicion %d\n", i);
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
 return 0;
}
