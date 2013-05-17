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
#include <pthread.h>
#include <sys/shm.h>
#include <sys/ipc.h>

#include "part.h"
#include "hash.h"
#include "crypto.h"
#include "verbose.h"
#include "io.h"

struct user_input {

	char cs[CHARSET_SIZE + 1];
	unsigned char hash[HASH_SIZE];
	int passlen;

};

struct threads {
	int active[MAX_THREADS];
	pthread_t ids[MAX_THREADS];

};

threads running;
user_input *ui;

extern int verbose, slow;
int my_rank; /* rank of process */
int num_procs; /* number of processes */
char last_try[256];
MPI_Datatype MPI_User_input;

int main(int argc, char *args[]) {
	char message[100]; /* storage for message */
	int source; /* rank of sender */
	int dest = 0; /* rank of receiver */
	int passlen;
	int tag = 0; /* tag for messages */

	MPI_Status status; /* return status for receive */
	int cmd;		/* Comando ricevuto dal master */

	int blockslen[UI_FIELDS];

	if (argc > 1) {
		verbose = atoi(args[1]);
		slow = atoi(args[2]);
	}

	printf("Avvio mainMPI...\n");

	ui = malloc(sizeof(user_input));

	/* start up MPI */
	verbose("MPI", "Inizializzazione MPI_Lib");
	MPI_Init(NULL, NULL );
	verbose("MPI", "Inizializzazione terminata");

	/* find out process rank */
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

	/* find aout number of processes */
	MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

	printf("MPI:: Numero di processi attivi= %d\nMPI:: My rank=%d\n", num_procs,
			my_rank);

	if (slow)
		sleep(my_rank);

	blockslen[0] = CHARSET_SIZE + 1;
	blockslen[1] = HASH_SIZE + 1;
	blockslen[2] = 1;

	MPI_Datatype types[UI_FIELDS] = { MPI_CHAR, MPI_CHAR, MPI_INT };
	MPI_Aint offsets[UI_FIELDS];

	offsets[0] = 0;
	offsets[1] = offsets[0] + (CHARSET_SIZE + 1) * sizeof(char);
	offsets[2] = offsets[1] + (HASH_SIZE + 1 + PADDING) * sizeof(char);

	MPI_Type_create_struct(UI_FIELDS, blockslen, offsets, types,
			&MPI_User_input);
	MPI_Type_commit(&MPI_User_input);


	/* Avvia la shell per il processo master */
	if (!my_rank) {
		shell(ui);
		supervisor();
	}

	/* Altrimenti rimane in attesa di istruzioni dal master */
	else {
		remote_shell(ui);
	}

	MPI_Type_free(&MPI_User_input);

	/* shut down MPI */
	MPI_Finalize();

	free(ui);

	return 0;
}

/**
 * Main loop per il thread di supervisione
 * sulla computazione corrente del relativo worker thread
 */
void supervisor() {
	int flag, i;
	MPI_Status status;
	pthread_t worker_id;
	char *plain;
	char buffer[256];

	plain = NULL;
	pthread_create(&worker_id, NULL, worker, &plain);

	while (1) {
		MPI_Iprobe(0, CMD_ABRT, MPI_COMM_WORLD, &flag, &status);

		if(flag){
			MPI_Recv(&flag, 1, MPI_INT, 0, CMD_ABRT, MPI_COMM_WORLD, &status);
			pthread_cancel(worker_id);
			return;
		}

		/* Controllo messaggi in locale */
		if (plain != NULL ) {
			/* Il worker locale ha trovato la password */
			flag = 1;

			/* Invia a tutti i processi la comunicazione di interrompere la computazione */
		//	printf("Processo %d - Avvio broadcast terminazione...\n", my_rank);
			for (i = 0; i < num_procs; i++) {
				if(i == my_rank)
					continue;
		//		printf("Processo %d - Invio messaggio di terminazione\n", my_rank);
				MPI_Send(&flag, 1, MPI_INT, i, TAG_COMPLETION, MPI_COMM_WORLD );
		//		printf("Processo %d - Messaggio di terminazione inviato\n", my_rank);
			}

			MPI_Send(plain, ui->passlen, MPI_CHAR, 0, TAG_PLAIN,
					MPI_COMM_WORLD );
			break;
		}

		/* Controllo messaggi remoti */
		//verbose("Crypto", "Verifico comunicazione remota");
	//	printf("Processo %d: Verifico comunicazione remota\n", my_rank);

		MPI_Iprobe(MPI_ANY_SOURCE, TAG_COMPLETION, MPI_COMM_WORLD, &flag,
				&status);

		if (flag) {
			/* Un worker remoto ha trovato la password */

			verbose("Crypto", "Messaggio remoto ricevuto");
			//printf("Processo %d: Messaggio remoto ricevuto\n", my_rank);

			MPI_Recv(buffer, 1, MPI_INT, MPI_ANY_SOURCE, TAG_COMPLETION,
					MPI_COMM_WORLD, &status);

			/* Il supervisor chiede la terminazione del worker locale */
			verbose("Crypto", "Arresto worker thread");
			pthread_cancel(worker_id);

			break;
		}

		sleep(1);
	}

	sprintf(buffer, "Password trovata dal processo %d", status.MPI_SOURCE);
	//verbose("Crypto", buffer);
	printf("%s\n", buffer);
}

/**
 * Funzione di lavoro per i processi dedicati alla decrittazione della password (codificata)
 */
void worker(char **plain) {
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL );
	key_gen(my_rank, num_procs, plain);
}

/*
 * #### RIP main ####
 *
 * Punto di ingresso del programma, da questo potranno essere
 * chiamati tutti i sotto moduli di Crypto
 */
