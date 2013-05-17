/* =============================================================
 * io.c
 *
 * Raccoglie le funzioni di I/O con l'utente.
 * In particolare espone una piccola shell di interazione al fine
 * di imposare le variabili di controllo per la ricerca della
 * password.
 *
 *  Created on: 16/mag/2013
 *      Author: mpiuser
 * =============================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "io.h"
#include "hash.h"
#include "sym.h"
#include "part.h"
#include "signal.h"
#include "crypto.h"

struct user_input {

	char cs[CHARSET_SIZE + 1];
	unsigned char hash[HASH_SIZE];
	int passlen;

};

struct threads {
	int num;
	int active[MAX_THREADS];
	pthread_t ids[MAX_THREADS];

};

extern int verbose;
extern threads running;
extern MPI_Datatype MPI_User_input;
extern int my_rank;
extern int num_procs;

void shell(user_input *ui) {

	int ret, passlen, num, cmd;
	long phash;
	unsigned char buffer[256], *token;
	unsigned char hash[HASH_SIZE];

	printf("==============================\n");
	printf("\tMAIN\n");
	printf("==============================\n");

	/* Avvia il main loop di shell per la configurazione */
	bzero(ui, sizeof(ui));

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
			cmd = CMD_QUIT;
			MPI_Bcast(&cmd, 1, MPI_INT, 0, MPI_COMM_WORLD);
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
				if (ret < 1) {
					printf(
							"La lunghezza della password deve essere maggiore di zero!\n");
					continue;
				}

				ui->passlen = ret;
				printf("Impostata una lunghezza di password di %d\n",
						ui->passlen);
			}

			if (!strcmp(token, "passwd")) {
				token = strtok(NULL, " \n");

				ret = strToBin(token, ui->hash, 2 * HASH_SIZE);
				if (ret < 0) {
					printf("Stringa non valida\n");
					continue;
				}

				printf("Impostata la password target = ");
				printHash(ui->hash);
				fflush(stdout);

			}

			if (!strcmp(token, "cs")) {
				token = strtok(NULL, " \n");

				num = (int) strtol(token, NULL, BASE);
				if (num < 0 || num > CS_SIZE) {
					printf("Range non valido [0, 6]\n");
					continue;
				}

				memcpy(ui->cs, charsets[num], strlen(charsets[num]) + 1);
				printf("Impostato il charset = '%s'\n", ui->cs);
			}
		}

		if (!strcmp(buffer, "run")) {
			printf("Avvio procedura decrittazione con parametri:\n");
			printf("charset = '%s'\n", ui->cs);
			printf("passwd: ");
			printHash(ui->hash);
			printf("passlen = %d\n", ui->passlen);
			printf("Conferma (y, n): ");
			fflush(stdin);

			while (!fgets(buffer, sizeof(buffer), stdin)) {
				continue;
			}

			if (buffer[0] == 'y') {
				cmd = CMD_EXEC;
				MPI_Bcast(&cmd, 1, MPI_INT, 0, MPI_COMM_WORLD);
				return;
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

		if (!strcmp(buffer, "verbose")) {
			token = strtok(NULL, " \n");
			verbose = atoi(token);
		}
	}

}

void remote_shell(user_input *ui) {
	long chunk;
	long disp;
	long init;
	int i, cs_size, cmd;
	char *cs;

	MPI_Bcast(&cmd, 1, MPI_INT, 0, MPI_COMM_WORLD);

	switch (cmd) {

	case CMD_QUIT:
		break;

	case CMD_ABRT:
		break;

	case CMD_EXEC:

		printf("DEBUG: Rank %d - I'm waiting\n", my_rank);

			MPI_Bcast(ui, 1, MPI_User_input, 0, MPI_COMM_WORLD );

			printf("DEBUG: Rank %d - Received Bcast\n", my_rank);
			printf("DEBUG: cs %s - Received cs\n", ui->cs);


			cs_size = strlen(ui->cs);
			disp = DISPOSITIONS(cs_size, ui->passlen)
			; // Numero di disposizioni da calcolare
			chunk = DISP_PER_PROC(disp, num_procs)
			; // Numero di disposizioni che ogni processo deve calcolare

			printf("DEBUG:Rank %d - Numero di disposizioni da calcolare: %lu.\n",
					my_rank, chunk);

			init = chunk * my_rank;

			if (init <= disp)
				supervisor();
			else
				printf("Nessuna computazione da eseguire.\n");
		break;

	default:
		break;
	}
}
