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

#include <fcntl.h>
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
#include <readline/readline.h>
#include <readline/history.h>

#include "shell.h"
#include "hash.h"
#include "sym.h"
#include "part.h"
#include "csignal.h"
#include "crypto.h"
#include "cerrno.h"
#include "verbose.h"

#define STR_INT_SIZE 16

extern char *charsets[];
extern char *help_msg;

int shm_id;
pid_t mpi_process = 0;
user_input *ui;

/**
 * Gestore di segnale di terminazione del processo mpirun
 */
void sh_abort_mpi(){
	int ret = kill(mpi_process, SIGUSR1);
	printf("Killing process (%d)= %d\n", mpi_process, ret);
}

void quit(){
	free(ui);
}

/**
 * Gestore dei segnali di terminazione della shell
 */
void sig_halt(){

	if(mpi_process)
		sh_abort_mpi();

	quit();
	printf("Exit...\n");
	exit(0);

}

void shell() {

	char num_procs[32];
	char spasslen[STR_INT_SIZE], sverbose[STR_INT_SIZE], sauditing[STR_INT_SIZE], sdictionary[STR_INT_SIZE];
	int ret, num;
	unsigned char buffer2[256], *token;
	unsigned char hash[HASH_SIZE];
	pthread_t wait_id;
	char *buffer;

	printf("==============================\n");
	printf("           Crypto\n");
	printf("==============================\n");

	/* Azzeramento aree di memoria */
	bzero(ui, sizeof(user_input));
	strcpy(num_procs, "1");
	strcpy(ui->cs, charsets[0]);
	ui->passlen = 1;
	buffer = malloc(32 * sizeof(char));

	/* Arma i segnali di terminazione forzata */
	signal(SIGHUP, sig_halt);
	signal(SIGINT, sig_halt);
	signal(SIGQUIT, sig_halt);
	signal(SIGTERM, sig_halt);

	/* Avvia il main loop di shell per la configurazione */
	while (1) {
		//printf(PROMPT);
		//fflush(stdout);

		bzero(buffer, sizeof(buffer));
		/*if (!fgets(buffer, sizeof(buffer), stdin)) {
			printf("Errore sullo stdin! Esco\n");
			exit(-EIO);	//TODO: codice errore
		}*/
		buffer = readline(PROMPT);


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

			if(token == NULL){
				printf("usage: set [%s | %s | %s | %s] {value}\n", CMD_SET_HASH, CMD_SET_SIZE, CMD_SET_CS, CMD_SET_PROC);
				continue;
			}

			if (!strcmp(token, CMD_SET_SIZE)) {
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

			if (!strcmp(token, CMD_SET_HASH)) {
				token = strtok(NULL, " \n");

				if(token == NULL){
					printf("usage: set %s {MD5-hash}\n", CMD_SET_HASH);
					continue;
				}

				if(strchr(token, 'x') != NULL)
					token = strchr(token, 'x')+1;

				if (strlen(token) - 2*HASH_SIZE) {
					printf("Stringa non valida\n");
					continue;
				}
				strncpy(ui->hash, token, 2*HASH_SIZE+1);

				printf("Impostata l'hash della password target = ");
				printf("%s\n", ui->hash);

			}

			if (!strcmp(token, CMD_SET_CS)) {
				token = strtok(NULL, " \n");

				if(token == NULL){
					printf("usage: set %s [0, 6]\n", CMD_SET_CS);
					continue;
				}

				num = (int) strtol(token, NULL, BASE);
				if (num < 0 || num > CS_SIZE) {
					printf("Range non valido [0, 6]\n");
					continue;
				}

				memcpy(ui->cs, charsets[num], strlen(charsets[num]) + 1);
				printf("Impostato il charset = '%s'\n", ui->cs);
			}

			// -------------- NP -----------------
			if(!strcmp(token, CMD_SET_PROC)) {
				token = strtok(NULL, " \n");
				if(token == NULL){
					printf("usage: set %s {N > 0}\n", CMD_SET_PROC);
					continue;
				}

				num = (int) strtol(token, NULL, BASE);
				if (num <= 0) {
					printf("Valore non valido [1, N]\n");
					continue;
				}

				sprintf(num_procs, "%d", num);
				printf("Impostato numero processi MPI: '%s'\n", num_procs);
			}
		}

		// -------------- RUN -----------------
		else if (!strcmp(token, "run")) {
			pprintf("SHELL", "Avvio procedura decrittazione con parametri:\n");
			printf("\tcharset = '%s'\n", ui->cs);
			printf("\thash: %s", ui->hash);
			printf("\n\tpasslen = %d\n", ui->passlen);
			printf("\tNumero processi MPI: %s\n", num_procs);
			printf("\tAuditing %s\n", ui->auditing ? "abilitato" : "disabilitato");
			//	printf("\tVerbose mode %s\n", ui->verbose ? "abilitata" : "disabilitata");
			printf("\tAttacco %s\n", ui->attack ? "a dizionario" : "brute force");
			printf("\tConferma? (y, n) ");
			fflush(stdout);

			while (!fgets(buffer, sizeof(buffer), stdin)) {
				continue;
			}

			if (buffer[0] == 'y') {
				if(ui->cs <= 0 || ui->passlen <= 0 || ui->hash <= 0){
					printf("Parametri non corretti!\nEsecuzione annullata\n");
					continue;
				}

				printf("\n\n");
				debug("SHELL", "Avvio dell'infrastruttura MPI...\n");
				ret = 0;

				sprintf(spasslen, "%d", ui->passlen);
				sprintf(sverbose, "%d", ui->verbose);
				sprintf(sauditing, "%d", ui->auditing);
				sprintf(sdictionary, "%d", ui->attack);

				void *args[] = {"mpirun", "-np", num_procs, "--hostfile", "runners.host",
						"launchMPI", ui->hash, spasslen, ui->cs, sverbose,
						sauditing, sdictionary, NULL};

				mpi_process = fork();
				if (!mpi_process){
					ret = execvp(*(args), args);
					debug("SHELL", "execvp=%d\n", ret);

					if(ret < 0){
						debug("SHELL", "Errore invocazione '%s': %s\n", args[0], strerror(errno));
						exit(-1); //todo: add error type
					}
				}

				debug("SHELL", "Processo MPI avviato con PID = %d\n", mpi_process);
				pthread_create(&wait_id, NULL, wait_child, NULL);
			}

			else {
				printf("Esecuzione annullata\n");
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
			printHash(hash, buffer2);
			printf("> MD5('%s') = %s\n", token, buffer2);

			// Autosets
			strcpy(ui->hash, buffer2);
			ui->passlen = strlen(token);
			printf("Impostato automaticamente:\n");
			printf("passwd hash = %s\n", ui->hash);
			printf("passlen = %d\n", ui->passlen);
		}

		// -------------- VERBOSE -----------------
		else if (!strcmp(buffer, CMD_VERBOSE)) {
			token = strtok(NULL, " \n");
			if(token == NULL){
				printf("usage: %s {0,1}\n", CMD_VERBOSE);
				continue;
			}

			ui->verbose = (int) strtol(token, NULL, BASE);
			if(ui->auditing) printf("Verbose mode attiva\n");
			else printf("Verbose mode disabilitata\n");
		}

		// -------------- AUDITING -----------------
		else if (!strcmp(buffer, CMD_AUD)) {
			token = strtok(NULL, " \n");
			if(token == NULL){
				printf("usage: % {0,1}\n", CMD_AUD);
				continue;
			}
			ui->auditing = (int) strtol(token, NULL, BASE);
			if(ui->auditing) printf("Abilitato il processo di auditing\n");
			else printf("Disabilitato il processo di auditing\n");
		}

		// -------------- ATTACK TYPE -----------------
		//TODO Scelta dizionario da usare
		else if (!strcmp(buffer, CMD_DICT)) {
			token = strtok(NULL, " \n");
			if(token == NULL){
				printf("usage: %s {0,1}\n", CMD_DICT);
				continue;
			}

			ui->attack = (int) strtol(token, NULL, BASE);
			if(ui->attack) printf("Impostato attacco a dizionario\n");
			else printf("Impostato attacco brute force\n");
		}

		// --------------- TEST -----------------
		else if (!strcmp(buffer, CMD_TEST_PASSWD)){
			token = strtok(buffer, " \n");


		}

		// -------------- ABORT -----------------
		else if (!strcmp(buffer, CMD_ABORT)){
			sh_abort_mpi();
		}

		// -------------- QUIT -----------------
		else if (!strcmp(token, "quit") ||! strcmp(token, "exit")) {
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

	ui = malloc(sizeof(user_input));

	shell();

	quit();
	return 0;
}

