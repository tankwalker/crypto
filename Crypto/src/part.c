/* ============================== part.c ==============================
 *
 * Rappresenta il modulo per la gestione del partizionamento
 * dell'insieme di sottochiavi che dovranno essere computate dai
 * singoli processi.
 *
 * ====================================================================
 */

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

/**
 * Effettua il test sullo hash MD5 per la chiave generata.
 *
 * @param str: Stringa da verificare
 * @return: 0 in caso di fallimento, >0 altrimenti
 */
int test(char *str) {
	//printf("current=%p", str);
	printf("Iterazione %d => %s\n", count++, str);
	return 0;
}

int contains(int pos, int *set, int k) {
	int i;
	for (i = 0; i < k; i++) {
		if (set[i] == pos)
			return 1;
	}
	return 0;
}

int comb(char *cs, int k, int pos, char *current, int n, int size) {
	int i;

	if (pos >= n) {
		test(current);
		return 0;
	}

	for (i = 0; i < k; i++) {
		if(contains(pos, set, size)){
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
/*int linearComb(char *cs, int k, char *current, int n) {
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
}*/

int key_gen(int *reserved, int k) {
	int n;
	char *str;

	printf("Avvio programma di partizione...\n");

	count = 0;

	// inizializzazione della stringa di lavoro
	str = malloc(16 * sizeof(char));					// alloca spazio di 16 caratteri per la stringa di output
	memset(str, 'a', 16 * sizeof(char));				// inizializza la stringa con il carattere 'a' (16x)

	// Inizializzazione del set degli indici riservati
	set = malloc(10 * sizeof(int));						// alloca spazio per l'insieme degli indici riservati
	bzero(set, 10 * sizeof(int));						// inizializza a zero l'intera area di memoria del set
	set[0] = 2;											//+
	//set[1] = 0;											//+ aggiunge 2 valori di test

	// Inizializzazione del dominio di caratteri
	char cs[] = { 'a', 'b', 'c', 'd', 'e', 'f' };		// charset

	n = 3;								// Numero di caratteri per la stringa finale
	str[n] = '\0';						// Terminatore di stringa in posizione n-esima

	comb(cs, 3, 0, str, 3, 1);

	free(str);							// dealloca lo spazio riservato per la stringa di output

	printf("Count = %d\n", count);
	printf("Programma terminato\n");

	return 0;
}
