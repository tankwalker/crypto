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

//TODO: metodi set/get
struct user_input {

	char cs[CHARSET_SIZE+1];
	char hash[HASH_SIZE];
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

/*
 * Funzioni GET dei membri della struttura user_input
 */

/**
 * Ritorna il charset su cui effettuare l'attacco
 *
 * @param ui: Puntatore a una struct user_input
 * @return: Puntatore al charset
 */

/*char *getCs(user_input *ui){

	return ui->cs;

}

/*
 * Funzioni SET dei membri della struttura user_input
 */

/**
 * Fissa il charset su cui effettuare l'attacco
 *
 * @param ui: Puntatore a una struct user_input
 * @return: Puntatore al charset
 */

/*char *setCs(user_input *ui, char *){

	return ui->cs;

}*/
