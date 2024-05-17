#include<stdio.h>
#include<stdlib.h>
#include <sys/time.h>

void ordenarIterativo();
void ordenarPar(int, int);
void combinar(int left, int medio, int right);

double *aux, *a, *b;
int N;

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

void ordenarIterativo(){
    int lenTrabajo, L, M, R,i;

    // Ordenar pares
    for (L=0; L < N-1; L+=2){
        ordenarPar(L, L+1);
    }
    
    // 4, 8, 16
    for (lenTrabajo=4; lenTrabajo <= N; lenTrabajo *= 2){
        // En ultimo len: L = 0, 16 // M = 7, 23 // R = 15, 29
        for (L=0; L < N-1; L += lenTrabajo){
            M = L + lenTrabajo/2 - 1;
            if (M >= N-1) break;    // ya está ordenado

            R = min(L + lenTrabajo - 1, N-1);
            combinar(L, M, R);
        }
    }

}

void ordenarPar(int p1, int p2){
    double aux1;
    
    if (a[p1] > a[p2]){
        aux1 = a[p1];
        a[p1] = a[p2];
        a[p2] = aux1;
    }
}

void combinar(int left, int medio, int right){
    	int len1 = (medio - left) + 1;
    	int len2 = (right - medio);
    	int i = 0, j = 0, k;

    	for (k=left; k<=right; k++){
        if (i >= len1) aux[k] = a[j++];       // si se acabaron los de L, copiar los de R
        else if (j >= len2) aux[k] = a[i++];  // si se acabaron los de R, copiar los de L
        else if (a[i] < a[j]) aux[k] = a[i++];    // copiar el menor entre L y R
        	else aux[k] = a[j++];

    }

}


int main(int argc, char*argv[]){
	double timetick;
	N = atol(argv[1]);
	int r, i;
	
	a = (double*)malloc(sizeof(double)*N);
	b = (double*)malloc(sizeof(double)*N);
	
	aux = (double*)malloc(sizeof(double)*N);

	//inicializacion
	for(i=0;i<N;i++){
		a[i] = i;
		aux[i] = i;
		b[i] = (N-1)-i;
		printf("%.1f ",a[i]);
 	}
 	
	timetick = dwalltime(); //arranca a contar 	
 	
 	ordenarIterativo();
 	
 	printf("Tiempo %f \n", dwalltime() - timetick);

	for(i=0;i<N;i++){
		printf("%.1f ",aux[i]);
	}
	printf("\n");
	
 	free(a);
 	free(b);
 	free(aux);
 return 0;
}