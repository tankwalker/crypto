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
	printf("Iterazione %d => %s\n", count++, str);
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

/**
 * Calcola tutte le disposizione (con ripetizione) per una stringa
 * di lunghezza 'n' su un insieme di caratteri 'cs'.
 * Il cursore 'pos' rappresenta la posizione corrente di lavoro all'interno della stringa.
 * La funzione ignora tutte le permutazioni degli indici cosiddetti 'riservati' presenti
 * all'interno dell'insieme 'set' di dimensione 'size'.
 *
 * @param cs: Set di caratteri di lavoro
 * @param k: Dimensione del set di caratteri
 * @param pos: Cursore di posizione all'intero della stringa
 * @param current: Stringa di lavoro
 * @param n: Dimensione della stringa di lavoro
 * @param set: Insieme contenente tutti gli indici 'riservati'
 * @param size: Dimensione del set degli indici riservati
 */
int comb(char *cs, int k, int pos, char *current, int n, int size) {
	int i;

	if (pos >= n) {
		test(current);
		return 0;
	}

	for (i = 0; i < k; i++) {
		if(contain(pos, set, size)){
			comb(cs, k, pos + 1, current, n, size);
			break;
		}
		current[pos] = cs[i];
		comb(cs, k, pos + 1, current, n, size);
	}

	return 0;
}

/*
 * Versione iterativa della funzione di generazione delle disposizioni
 * sul charset cs nella stringa current
 */
int linearComb(char *cs, int k, char *current, int n) {
	int i, j, q;

	for (i = n-2; i >= 0; i--) {
		for (j = 0; j < k; j++) {

			current[i] = cs[j];

			for (q = 0; q < k; q++) {
				current[n-1] = cs[q];
				test(current);
			}
		}
		current[i] = cs[0];
	}

	return 0;
}

/*
 * Punto di ingresso del programma di prova per la generazione
 * delle disposizioni su più processi/thread.
 */
int provaMain(int argc, char *argv[]) {
	int n;
	char *str;

	printf("Avvio programma di partizione...\n");
	if (argc > 1) {
		min = atoi(argv[1]);
		max = atoi(argv[2]);
		filter = atoi(argv[3]);
		printf("min=%d, max=%d\n", min, max);
	}

	count = 0;

	// inizializzazione della stringa di lavoro
	str = malloc(16 * sizeof(char));
	memset(str, 'a', 16 * sizeof(char));
	test(str);

	// Inizializzazione del set degli indici riservati
	set = malloc(10 * sizeof(int));
	bzero(set, 10 * sizeof(int));
	set[0] = 1;
	set[1] = 2;

	// Inizializzazione del dominio di caratteri
	char cs[] = { 'a', 'b', 'c', 'd', 'e', 'f' };

	n = 3;			// Numero di caratteri per la stringa finale
	str[n] = '\0';	// Terminatore di stringa in posizione n-esima

	comb(cs, 3, 0, str, 3, 2);

	printf("Count = %d\n", count);
	printf("Programma terminato\n");

	return 0;
}
