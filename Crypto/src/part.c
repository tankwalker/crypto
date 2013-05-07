/* ============================== part.c ==============================
 *
 * Rappresenta il modulo per la gestione del partizionamento
 * dell'insieme di sottochiavi che dovranno essere computate dai
 * singoli processi.
 *
 * ====================================================================
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "part.h"

/*
 * Dichiarazione varibili globali
 */
int count, min, max, *set, filter, my_rank;

struct string_t{

	char *str;
	int size;

};

struct comb_settings{

	int *starting_point;
	long chunk;

};

struct comb_parms{

	string_t *cs, *passwd;
	comb_settings *init;

};

struct user_input{

	char *cs;
	char *hash;
	int passlen;

};

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

int comb(comb_parms *parms, int pos){


	int i, cs_size = parms->cs->size;
	long chunk = parms->init->chunk;
	char *cs = parms->cs->str;
	char *passwd = parms->passwd->str;

	//printf("Parms:cs_size %d - chunk %lu - cs %s - passwd %s\n", cs_size, chunk, cs, passwd);

	if(count == chunk) return 0;

	if(pos == parms->passwd->size){

		test(passwd);
		return 0;
	}

	if(!count){

		for(i=(parms->init->starting_point)[pos]; i < cs_size && count != chunk; i++){

			passwd[pos] = cs[i];
			comb(parms, pos+1);

		}

	}

	else{

		for(i=0; i < cs_size && count != chunk; i++){

			passwd[pos] = cs[i];
			comb(parms, pos+1);

		}

	}
	return 0;
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
		starting_point[passlen-i-1] = p;
		printf("DEBUG:Rank %d - char in pos %d:%d\n",my_rank,i,p);

	}
	return starting_point;
}

int key_gen(int rank, int num_procs, user_input *ui) {

	int *starting_point;
	long chunk, disp, init;

	comb_parms *parms;
	string_t *cs, *passwd;
	comb_settings *settings;

	printf("Avvio programma di partizione...\n");
	//printf("DEBUG: Rank %d, cs %s, passwd %s, passlen %d\n", my_rank, ui->cs, ui->hash, ui->passlen);

	count = 0;

	parms = malloc(sizeof(comb_parms));

	cs = malloc(2*sizeof(string_t));
	passwd = cs+1;
	settings = malloc(sizeof(comb_settings));

	cs->str = ui->cs;
	cs->size = strlen(ui->cs);
	passwd->str = malloc((ui->passlen + 1) * sizeof(char)); // alloca spazio di 16 caratteri per la stringa di output
	passwd->size = ui->passlen;
	memset(passwd->str, '\0', (ui->passlen + 1) * sizeof(char)); // inizializza la stringa di lavoro

	my_rank = rank;
	disp = DISPOSITIONS(cs->size, ui->passlen); // Numero di disposizioni da calcolare
	chunk = DISP_PER_PROC(disp, num_procs); // Numero di disposizioni che ogni processo deve calcolare

	//printf("DEBUG:Rank %d - Numero di disposizioni da calcolare: %lu.\n", my_rank, chunk);

	init = chunk * my_rank;

	starting_point = compute_starting_point(init, cs->size, ui->passlen);

	settings->chunk = chunk;
	settings->starting_point=starting_point;

	parms->cs=cs;
	parms->passwd=passwd;
	parms->init=settings;

	comb(parms, 0);

	free(settings);
	free(cs);
	free(parms);

	printf("Count = %d\n", count);
	printf("Programma terminato\n");

	return 0;
}
