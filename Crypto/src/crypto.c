/*
 ============================================================================
 Name        : Crypto.c
 Author      : Davide
 Version     :
 Copyright   :
 Description : Calculate Pi in MPI
 ============================================================================
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <time.h>

#include "part.h"
#include "hash.h"
#include "crypto.h"

struct user_input {

	char cs[CHARSET_SIZE + 1];
	unsigned char hash[HASH_SIZE];
	int passlen;

};

user_input ui;

void shell() {

	int ret, passlen, num;
	long phash;
	unsigned char buffer[256], *token;
	unsigned char hash[HASH_SIZE];

	printf("==============================\n");
	printf("\tMAIN\n");
	printf("==============================\n");

	/* Avvia il main loop di shell per la configurazione */
	bzero(&ui, sizeof(ui));

	while (1) {
		printf(">> ");

		bzero(buffer, sizeof(buffer));
		if (!fgets(buffer, sizeof(buffer), stdin)) {
			printf("Errore sullo stdin! Esco\n");
			exit(-1);
		}

		token = strtok(buffer, " \n");
		if (token == NULL ) {
			continue;
		}

		/* Chiude il programma */
		if (!strcmp(token, "quit")) {


			printf("Esco...\n");
			break;
		}

		if (!strcmp(token, "help")) {
			printf("===== Help =====\n");
			printf("quit - per uscire\n");
			printf(
					"hash 'string' - restituisce lo hash della stringa in input\n");
			printf(
					"set | passlen, passwd, cs, hfunc - Imposta il valore di lavoro per la variabile indicata:\n");
			printf(
					"cs 'num' - imposta il charset di lavoro per la generazione delle password.\n");
			printf("\tnum=0 -> sole minuscole\n");
			printf("\tnum=1 -> sole maiuscole\n");
			printf("\tnum=2 -> soli numeri\n");
			printf("\tnum=3 -> maiuscole e minuscole\n");
			printf("\tnum=4 -> numeri e minuscole\n");
			printf("\tnum=5 -> numeri e maiuscole\n");
			printf("\tnum=6 -> completo\n");
			printf(
					"passswd 'string' - Rappresenta la password codificata (md5) da decrittare\n");
			printf(
					"passlen 'num' - Numero di caratteri della password da trovare\n");
			printf(
					"hfunc 'type' - Imposta la funzione di hash da utilizzare nella ricerca della passwd [funzione non ancora abilitata]\n");
			printf("run - per avviare la generazione delle password\n");
			printf("===============\n");
		}

		if (!strcmp(token, "set")) {
			token = strtok(NULL, " \n");

			if (!strcmp(token, "passlen")) {
				token = strtok(NULL, "");

				ret = (int) strtol(token, NULL, BASE);
				if(ret < 1){
					printf("La lunghezza della password deve essere maggiore di zero!\n");
					continue;
				}

				ui.passlen = ret;
				printf("Impostata una lunghezza di password di %d\n",
						ui.passlen);
			}

			if (!strcmp(token, "passwd")) {
				token = strtok(NULL, " \n");

				ret = strToBin(token, ui.hash, 2 * HASH_SIZE);
				if(ret < 0){
					printf("Stringa non valida\n");
					continue;
				}

				printf("Impostata la password target = ");
				printHash(ui.hash);
				fflush(stdout);

			}

			if (!strcmp(token, "cs")) {
				token = strtok(NULL, " \n");

				num = (int) strtol(token, NULL, BASE);
				if(num < 0 || num > CS_SIZE){
					printf("Range non valido [0, 6]\n");
					continue;
				}

				memcpy(ui.cs, charsets[num], strlen(charsets[num]) + 1);
				printf("Impostato il charset = '%s'\n", ui.cs);
			}
		}

		if (!strcmp(buffer, "run")) {
			printf("Avvio procedura decrittazione con parametri:\n");
			printf("charset = '%s'\n", ui.cs);
			printf("passwd: ");
			printHash(ui.hash);
			printf("passlen = %d\n", ui.passlen);
			printf("Conferma (y, n): ");
			fflush(stdin);

			while(!fgets(buffer, sizeof(buffer), stdin)){
				continue;
			}

			if (buffer[0] == 'y') {
				break;
			}

			else {
				printf("annullato\n");
				continue;
			}
		}

		else if (!strcmp(buffer, "hash")) {
			scanf("%s", buffer);
			hashMD5(buffer, hash);
			printf("> MD5('%s') = ", buffer);
			printHash(hash);
		}

		puts("");
	}

}

int main(int argc, char *args[]) {
	char message[100]; /* storage for message */
	int my_rank; /* rank of process */
	int num_procs; /* number of processes */
	int source; /* rank of sender */
	int dest = 0; /* rank of receiver */
	int passlen;
	int tag = 0; /* tag for messages */
	long chunk;
	long disp;
	long init;
	MPI_Status status; /* return status for receive */

	int blockslen[UI_FIELDS];

	int i, cs_size;
	char *cs;

	printf("Avvio mainMPI\n");

	/* start up MPI */
	MPI_Init(NULL, NULL);

	printf("MPI:: Inizializzazione terminata");

	/* find out process rank */
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

	/* find aout number of processes */
	MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

	printf("MPI:: Numero di processi attivi= %d\nMPI:: My rank=%d\n", num_procs, my_rank);

	sleep(my_rank);
	if(!my_rank) shell(&ui);


	blockslen[0] = CHARSET_SIZE + 1;
	blockslen[1] = HASH_SIZE + 1;
	blockslen[2] = 1;

	MPI_Datatype types[UI_FIELDS] = { MPI_CHAR, MPI_CHAR, MPI_INT };
	MPI_Datatype MPI_User_input;
	MPI_Aint offsets[UI_FIELDS];

	offsets[0] = 0;
	offsets[1] = offsets[0] + (CHARSET_SIZE + 1) * sizeof(char);
	offsets[2] = offsets[1] + (HASH_SIZE + 1 + PADDING) * sizeof(char);

	MPI_Type_create_struct(UI_FIELDS, blockslen, offsets, types,
			&MPI_User_input);
	MPI_Type_commit(&MPI_User_input);


	printf("DEBUG: Rank %d - I'm waiting\n", my_rank);
	MPI_Bcast(&ui, 1, MPI_User_input, 0, MPI_COMM_WORLD );
	printf("DEBUG: Rank %d - Received Bcast\n", my_rank);

	cs_size = strlen(ui.cs);
	disp = DISPOSITIONS(cs_size, ui.passlen); // Numero di disposizioni da calcolare
	chunk = DISP_PER_PROC(disp, num_procs); // Numero di disposizioni che ogni processo deve calcolare

	printf("DEBUG:Rank %d - Numero di disposizioni da calcolare: %lu.\n",
			my_rank, chunk);

	init = chunk * my_rank;

	if (init <= disp)
		key_gen(my_rank, num_procs);
	else
		printf("Nessuna computazione da eseguire.\n");

	MPI_Type_free(&MPI_User_input);

	/* shut down MPI */
	MPI_Finalize();

	return 0;
}

/*
 * #### RIP main ####
 *
 * Punto di ingresso del programma, da questo potranno essere
 * chiamati tutti i sotto moduli di Crypto
 */
