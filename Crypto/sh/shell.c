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
#include <errno.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "shell.h"
#include "hash.h"
#include "sym.h"
#include "part.h"
#include "csignal.h"
#include "crypto.h"
#include "cerrno.h"
#include "verbose.h"


extern int verbose;
extern char *charsets[];
extern char *help_msg;
// extern threads running;
//extern MPI_Datatype MPI_User_input;
//extern int my_rank;
//extern int num_procs;

int shm_id;
pid_t mpi_process = 0;
user_input *ui;

/**
 * Gestore di segnale di terminazione del processo mpirun
 */
void sh_abort_mpi(){
	int ret = kill(mpi_process, SIGTERM);
	printf("Killing process (%d)= %d\n", mpi_process, ret);
}

void quit(){
	struct shmid_ds ds;

	shmdt(ui);
	shmctl(shm_id, IPC_RMID, &ds);
}

/**
 * Gestore dei segnali di terminazione della shell
 */
void sig_halt(){

	if(mpi_process)
		sh_abort_mpi();

	quit();
	printf("Quitting...\n");
	exit(0);

}

void shell() {

	char shm_str[32];
	char num_procs[32];
	int ret, num;
	unsigned char buffer[256], *token;
	unsigned char hash[HASH_SIZE];
	pthread_t wait_id;
	int mpi_proc_status;


	printf("==============================\n");
	printf("           Crypto\n");
	printf("==============================\n");

	bzero(ui, sizeof(user_input));
	strcpy(num_procs, "1");			/// Usi MPI...almeno lavora con 2 processi, no?!

	/* Arma i segnali di terminazione forzata */
	signal(SIGHUP, sig_halt);
	signal(SIGINT, sig_halt);
	signal(SIGQUIT, sig_halt);
	signal(SIGTERM, sig_halt);

	/* Avvia il main loop di shell per la configurazione */
	while (1) {
		printf(PROMPT);
		fflush(stdout);

		bzero(buffer, sizeof(buffer));
		if (!fgets(buffer, sizeof(buffer), stdin)) {
			printf("Errore sullo stdin! Esco\n");
			exit(-EIO);	//TODO: codice errore
		}

		token = strtok(buffer, " \n");
		if (token == NULL ) {
			continue;
		}

		// -------------- HELP -----------------
		else if (!strcmp(token, "help")) {
			printf("%s\n", help_msg);
		}

		// -------------- SETS -----------------
		else if (!strcmp(token, "set")) {
			token = strtok(NULL, " \n");

			if (!strcmp(token, "passlen")) {
				token = strtok(NULL, " \n");

				ret = (int) strtol(token, NULL, BASE);
				if (ret < 1) {
					printf("La lunghezza della password deve essere maggiore di zero!\n");
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
				printf("\n");

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

			// -------------- NP -----------------
			if(!strcmp(token, "proc")) {
				token = strtok(NULL, " \n");
				num = (int) strtol(token, NULL, BASE);
				sprintf(num_procs, "%d", num);
				printf("Impostato numero processi MPI: '%s'\n", num_procs);
			}
		}

		// -------------- RUN -----------------
		else if (!strcmp(buffer, "run")) {
			pprintf("SHELL", "Avvio procedura decrittazione con parametri:\n");
			printf("\tcharset = '%s'\n", ui->cs);
			printf("\tpasswd: ");
			printHash(ui->hash);
			printf("\n\tpasslen = %d\n", ui->passlen);
			printf("\tNumero processi MPI: '%s'\n", num_procs);
			printf("\tAuditing attivo[T/F]: '%d'\n", ui->auditing);
			printf("\tConferma? (y, n) ");
			fflush(stdout);

			while (!fgets(buffer, sizeof(buffer), stdin)) {
				continue;
			}

			if (buffer[0] == 'y') {
				debug("SHELL", "Avvio MPI...\n");
				ret = 0;

				sprintf(shm_str, "%d", shm_id);
				void *args[] = {"mpirun", "-np", num_procs, "launchMPI", shm_str, NULL};

				debug("SHELL", "exec %s %s %s %s %s\n", args[0], args[1], args[2], args[3], args[4]);

				mpi_process = fork();
				if (!mpi_process){
					ret = execvp(*args, args);
					debug("SHELL", "execvp=%d\n", ret);

					if(ret < 0)
						debug("SHELL", "Errore invocazione '%s': %s\n", args[0], strerror(errno));
				}

				debug("SHELL", "Processo MPI avviato con PID = %d\n", mpi_process);
				pthread_create(&wait_id, NULL, wait_child, NULL);
			}

			else {
				printf("annullato\n");
				continue;
			}
		}


		// -------------- HASH -----------------
		else if (!strcmp(buffer, "hash")) {
			token = strtok(NULL, " \n");
			if(token == NULL){
				printf("usage: hash [plain-text]\n");
				continue;
			}

			hashMD5(token, hash);
			printf("> MD5('%s') = ", buffer);
			printHash(hash);
			printf("\n");
		}

		// -------------- VERBOSE -----------------
		else if (!strcmp(buffer, "verbose")) {
			token = strtok(NULL, " \n");
			ui->verbose = (int) strtol(token, NULL, BASE);
		}

		// -------------- AUDITING -----------------
		else if (!strcmp(buffer, "auditing")) {
			token = strtok(NULL, " \n");
			ui->auditing = (int) strtol(token, NULL, BASE);
			if(ui->auditing) printf("Impostato processo di auditing\n");
			else printf("Annulato processo di auditing\n");
		}

		// -------------- ABORT -----------------
		else if (!strcmp(buffer, "abort")){
			abort();
		}

		// -------------- QUIT -----------------
		else if (!strcmp(token, "quit")) {
			sig_halt();
			return;
		}
	}

}

int wait_child() {
	int mpi_proc_status;

	debug("SHELL", "Attesa della terminazione del processo MPI\n");
	wait(&mpi_proc_status);
	debug("SHELL", "Processo MPI terminato con codice = %d (%s)\n", mpi_proc_status, strerror(mpi_proc_status));

	return mpi_proc_status;
}


/*
 * -------------------------- MAIN --------------------------
 * Punto di ingresso del programma, da questo potranno essere
 * chiamati tutti i sotto moduli di Crypto
 * ----------------------------------------------------------
 */
int main(int argc, char *args[]){

	shm_id = shmget(IPC_PRIVATE, sizeof(user_input), IPC_CREAT | PERMS);
	ui = shmat(shm_id, NULL, 0);

	debug("SHELL", "Shared memory attach sul segmento %d: (%s)\n", shm_id, strerror(errno));

	shell();

	quit();
	return 0;
}

