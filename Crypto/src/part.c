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
int count, min, max, *set, filter, my_rank;

/*
 * Funzioni principali
 */

/**
 * Effettua il test sullo hash MD5 per la chiave generata.
 *
 * temporaneamente stampa solo la stringa prodotta
 *
 * @param str: Stringa da verificare
 * @return: 0 in caso di fallimento, >0 altrimenti
 */
int test(char *str) {
	printf("Processo %d => %s\n", my_rank, str);
	count++;
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

int comb(char *cs, int k, int pos, char *current, int n, int *set, int size, int from, int to) {
	int i;

	if (pos >= n) {
		test(current);
		return 0;
	}

	//printf("comb # from=%d, to=%d\n", from ,to);

	/* Se il livello di chiamata ricorsiva, dunque la posizione
	 * all'interno della password, è contenuta all'interno del
	 * set, allora viene avviato il ciclo ridotto al sottoinsieme
	 * di caratteri di competenza del processo MPI
	 */
	if (contains(pos, set, size))
		for (i = from; i < to; i++) {
			current[pos] = cs[i];
			comb(cs, k, pos + 1, current, n, set, size, from, to);
		}

	else
		for (i = 0; i < k; i++) {
			/*if(contains(pos, set, size)){
			 current[pos] = fixed;
			 comb(cs, k, pos + 1, current, n, set, size, fixed, from, to);
			 break;
			 }*/
			current[pos] = cs[i];
			comb(cs, k, pos + 1, current, n, set, size, from, to);
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

int key_gen(int rank, int num_procs, char *cs, int passlen) {

	int index, cs_size, from, to, chunk;
	char *str;

	printf("Avvio programma di partizione...\n");

	count = 0;

	// inizializzazione della stringa di lavoro
	str = malloc(16 * sizeof(char)); // alloca spazio di 16 caratteri per la stringa di output
	memset(str, 'a', 16 * sizeof(char)); // inizializza la stringa con il carattere 'a' (16x)

	// Inizializzazione del set degli indici riservati
	set = malloc(10 * sizeof(int));	// alloca spazio per l'insieme degli indici riservati
	bzero(set, 10 * sizeof(int));// inizializza a zero l'intera area di memoria del set

	my_rank = rank;
	str[passlen] = '\0';		// Terminatore di stringa in posizione n-esima
	cs_size = strlen(cs);
	//TODO OTTIMIZZARE K/NUM_PROCS;
	chunk = (cs_size / num_procs);
	if(chunk < 1){

	}
	from = chunk * my_rank;
	to = chunk + from;
	if (my_rank == (num_procs - 1) && my_rank != 0)
		if ((cs_size % num_procs))
			to++;

	//for (index = 0; index < 1; index++) {
	//	printf("=======================\n");
	//	printf("index = %d\n", index);
	count = 0;
	comb(cs, cs_size, 0, str, passlen, set, 1, from, to);
	//}

	free(str);			// dealloca lo spazio riservato per la stringa di output
	free(set);

	printf("Count = %d\n", count);
	printf("Programma terminato\n");

	return 0;
}
