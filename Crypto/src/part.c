/* ============================== part.c ==============================
 *
 * Rappresenta il modulo per la gestione del partizionamento
 * dell'insieme di sottochiavi che dovranno essere computate dai
 * singoli processi.
 *
 * ====================================================================
 */


#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DISP_PER_PROC(cs_len, pass_len, num_procs) powl((cs_len),(pass_len))/(num_procs);
#define STARTING_CHAR(init, cs_size, pos) (((int)((init)/(pow(cs_size,pos))))%(cs_size));


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
int test(char *pass) {
	printf("Processo %d => %s\n", my_rank, pass);
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

int disp2char(char *str, int m, char *cs, int n){

	int i,j,index=0;

	for(i=0; i<n; i++){

		str[index] = cs[i];

		for(j=0; j<n; j++){

			str[index+1] = cs[j];
			test(str);

		}
	}
	return 0;
}

int disp3char(char *str, int m, char *cs, int n){

	int i,j,k,index = 0;

	for(i=0; i<n; i++){

		str[index] = cs[i];

		for(j=0; j<n; j++){

			str[index+1] = cs[j];

			for(k=0; k<n; k++){

				str[index+2] = cs[k];
				test(str);

			}
		}
	}
	return 0;
}

int disp4char(char *str, int m, char *cs, int n){

	int x,y,w,z, index = 0;

	for(x=0; x<n; x++){

		str[index] = cs[x];

		for(y=0; y<n; y++){

			str[index+1] = cs[y];

			for(w=0; w<n; w++){

				str[index+2] = cs[w];

				for(z=0; z<n; z++){

					str[index+3] = cs[z];
					test(str);

				}

			}
		}
	}
	return 0;
}

int comb(char *pass, int passlen, char *cs,int cs_size, int *starting_point, long chunk, int pos){

	int i;

	if(count == chunk) return 0;

	if(pos == passlen){

		test(pass);
		return 0;
	}

	for(i=starting_point[pos]; i < cs_size; i++){

		pass[pos] = cs[i];
		comb(pass, passlen, cs, cs_size, starting_point, chunk, pos+1);

	}

}

/*int comb(char *cs, int k, int pos, char *current, int n, int *set, int size, int from, int to) {
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
	/*if (contains(pos, set, size))
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
		/*	current[pos] = cs[i];
			comb(cs, k, pos + 1, current, n, set, size, from, to);
		}

	return 0;
}*/

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

int *compute_starting_point(long init, int cs_size, int passlen){

	int i;
	int *starting_point = malloc(passlen * sizeof(int));

	for(i = 0; i < passlen; i++){

		int p = STARTING_CHAR(init, cs_size, i);
		starting_point[i] = p;
		printf("DEBUG: char in pos %d:%d\n",i,p);

	}
	return starting_point;
}

int key_gen(int rank, int num_procs, char *cs, int passlen) {

	char *pass;
	int index, cs_size, from, to;
	int *starting_point;
	long chunk;

	printf("Avvio programma di partizione...\n");

	count = 0;
	cs_size = strlen(cs);
	pass = malloc(16 * sizeof(char)); // alloca spazio di 16 caratteri per la stringa di output
	memset(pass, '\0', 16 * sizeof(char)); // inizializza la stringa con il carattere 'a' (16x)

	// Numero di disposizioni che ogni processo deve calcolare
	chunk = DISP_PER_PROC(cs_size, passlen, num_procs);

	starting_point = compute_starting_point(chunk * rank, cs_size, passlen);

	comb(pass, passlen, cs, cs_size, starting_point, chunk, 0);


	// inizializzazione della stringa di lavoro

	// Inizializzazione del set degli indici riservati
	set = malloc(10 * sizeof(int));	// alloca spazio per l'insieme degli indici riservati
	bzero(set, 10 * sizeof(int));// inizializza a zero l'intera area di memoria del set

	my_rank = rank;
	pass[passlen] = '\0';		// Terminatore di stringa in posizione n-esima
	//TODO OTTIMIZZARE K/NUM_PROCS;
	/*chunk = (cs_size / num_procs);
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
	//}*/

	//disp4char(pass, passlen, cs, cs_size);

	free(pass);			// dealloca lo spazio riservato per la stringa di output
	free(set);

	printf("Count = %d\n", count);
	printf("Programma terminato\n");

	return 0;
}

int partitionPwds(int num_procs, int cs_len, int disp_per_proc){

	int i,j=0;
	long disp_i_char=0;






	for(i=0; i<num_procs; i++){

		do{
			j++;
			disp_i_char = powl(cs_len, j);

		}while(disp_i_char < disp_per_proc);

		//key_gen();
	}

	return 0;
}
