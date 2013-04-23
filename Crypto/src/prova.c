#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*
 * Dichiarazione varibili globali
 */
int count, min, max, *set, filter;

/*
 * Funzioni principali
 */
int test(char *str) {
	//printf("current=%p", str);
	printf("Iterazione %d => %s\n", count, str);
	return 0;
}

int contain(int pos, int *set, int k) {
	int i;
	for (i = 0; i < k; i++) {
		if (set[i] == pos)
			return 1;
	}
	return 0;
}

/*
 * Versione ricorsiva della funzione di generazione delle disposizioni
 * sul charset cs nella stringa current
 */
int comb(char *cs, int k, int pos, char *current, int n, int count) {
	int i;
	//count++;

	if (pos == n) {
		test(current);
		return 0;
	}

	for (i = 0; i < k; i++) {
		current[pos] = cs[i];
		//printf("i=%d, pos=%d, current=%s (%p)", i, pos, current, current);
		comb(cs, k, pos + 1, current, n, count + 1);
		if (contain(pos, set, 10))
			break;
	}

	return 0;
}

/*
 * Scambia di posto due caratteri in una stringa nelle posizioni specificate
 */
int swap(char *cs, int s, int i) {
	char tmp;
	tmp = cs[s];
	cs[s] = cs[i];
	cs[i] = tmp;
	return 0;
}

/*
 * Versione iterativa della funzione di generazione delle disposizioni
 * sul charset cs nella stringa current
 */
int linearComb(char *cs, int k, char *current, int n) {
	int i, j, q;

	for (i = n; i > 0; i--) {
		for (j = 0; j < k; j++) {
			for (q = 0; q < k; q++) {
				current[i] = cs[q];
				test(current);
			}
			current[i] = cs[j];
		}
	}

	return 0;
}

/*
 * Punto di ingresso del programma di prova per la generazione
 * delle disposizioni su più processi/thread.
 */
int provaMain(int argc, char *argv[]) {
	int *matrix;
	char *str;

	printf("Avvio programma di partizione...\n");
	if (argc > 1) {
		min = atoi(argv[1]);
		max = atoi(argv[2]);
		filter = atoi(argv[3]);
		printf("min=%d, max=%d\n", min, max);
	}

	count = 0;

	str = malloc(16 * sizeof(char));
	memset(str, 'a', 16*sizeof(char));
	test(str);

	set = malloc(10 * sizeof(int));
	bzero(set, 10 * sizeof(int));
	set[0] = min;
	set[1] = max;

	char cs[] = { 'a', 'b', 'c', 'd', 'e', 'f' };

	linearComb(cs, 3, str, 3);

/*	int i;
	for (i = 0; i < 3; i++) {
		printf("swap!!");
		comb(cs, 3, 0, str, 3, 0);
		swap(cs, 0, i + 1);
	}*/

	printf("Count = %d\n", count);
	printf("Programma terminato\n");

	return 0;
}
