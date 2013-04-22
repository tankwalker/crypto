#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int count, min, max, *set, filter;

/*
 * Alloca una matrice di interi di dimensioni n*m
 */
int matrix_populate(int n, int m, void **ptr){
	int *matrix;

	ptr = malloc((n * m) * (sizeof(int)));
	if(!ptr)
		return -1;

	return 0;
}

/*
 * Stampa la matrice
 */
//void print_matrix(int n, int m, int *matrix){
//	int i, j;
//
//	for(i = 0; i<n; i++){
//		for(j = 0; j<m; j++){
//			printf("%d | ", *matrix[j][i]);
//		}
//		printf("\n");
//	}
//}

int pivot(int n, int m, int *matrix){

}

int test(char *str){
	//printf("current=%p", str);
	printf("Iterazione %d => %s\n", count, str);
	return 0;
}

int contain(int pos, int *set, int k){
	int i;
	for(i=0; i<k; i++){
		if(set[i] == pos) return 1;
	}
	return 0;
}

int comb(char *cs, int k, int pos, char *current, int n, int count){
	int i;
	//count++;

	if(pos == n){
		test(current);
		return 0;
	}

	for(i=0; i<k; i++){
		current[pos] = cs[i];
		//printf("i=%d, pos=%d, current=%s (%p)", i, pos, current, current);
		comb(cs, k, pos+1, current, n, count+1);
		if(contain(pos, set, 10)) break;
	}

	return 0;}

int swap(char *cs, int s, int i){
	char tmp;
	tmp = cs[s];
	cs[s] = cs[i];
	cs[i] = tmp;
	return 0;
}

int main(int argc, char *argv[]){
	int *matrix;
	char *str;

	printf("Avvio programma di partizione...\n");
	if(argc){
		min = atoi(argv[1]);
		max = atoi(argv[2]);
		filter = atoi(argv[3]);
		printf("min=%d, max=%d\n", min, max);
	}

	count = 0;

	str = malloc(16 * sizeof(char));
	bzero(str, 16);

	set = malloc(10 * sizeof(int));
	bzero(str, 10 * sizeof(int));
	set[0] = min;
	set[1] = max;

	char cs[] = {'a', 'b', 'c', 'd', 'e', 'f'};

	int i;
	for(i=0; i<3; i++){
		printf("swap!!");
		comb(cs, 3, 0, str, 3, 0);
		swap(cs, 0, i+1);
	}
	//matrix_populate(2, 2, matrix);			// Crea una matrice 2*2

	printf("Count = %d\n", count);
	printf("Programma terminato\n");

	return 0;
}
