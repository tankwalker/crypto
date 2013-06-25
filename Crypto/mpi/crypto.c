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
#include <unistd.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <errno.h>
#include <signal.h>

#include <unistd.h>
#include <linux/unistd.h>
#include <stdlib.h>
#include <sched.h>
#include <syscall.h>
#include <sys/syscall.h>


#include "part.h"
#include "hash.h"
#include "crypto.h"
#include "verbose.h"
#include "sym.h"
#include "cerrno.h"
#include "struct.h"

//threads running;
user_input *ui;

int verbose, slow, auditing;
int my_rank; /* rank of process */
int num_procs; /* number of processes */
char last_try[256];
MPI_Datatype MPI_User_input;

int main(int argc, char *argv[]) {
	char message[100]; /* storage for message */
	int source; /* rank of sender */
	int dest = 0; /* rank of receiver */
	int passlen;
	int tag = 0; /* tag for messages */
	int shm_id;

	pthread_t superv_id; /* identificativo del thread di supervisione */
	int *cerrno;			/* variabile di appoggio per il salvtaggio dello stato di errore */
	MPI_Status status; /* return status for receive */
	int cmd;		/* Comando ricevuto dal master */

	debug("MPI", "Parametri avvio: %s, %s\n", argv[0], argv[1]);

	if(argc > 0){
		shm_id = atoi(argv[1]);
		ui = shmat(shm_id, NULL, SHM_RDONLY);

		debug("MPI", "Shared memory attach sul segmento '%d': (%s)\n", shm_id, strerror(errno));
	}

	else
		return -EPARM;

	signal(SIGTERM, abort_mpi);

	debug("MPI", "UI-> ");
	printHash(ui->hash);
	debug("\nMPI", "passlen='%d', cs='%s, verbose='%d', auditing='%d'\n", ui->passlen, ui->cs, ui->verbose, ui->auditing);

	int blockslen[UI_FIELDS];

	auditing = ui->auditing;
	verbose = ui->verbose;

	/* start up MPI */
	debug("MPI", "Inizializzazione Libreria MPI\n");
	MPI_Init(NULL, NULL );
	debug("MPI", "Inizializzazione terminata\n");

	/* find out process rank */
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

	/* find out number of processes */
	MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

	debug("MPI", "Numero di processi attivi= %d\nMPI:: My rank=%d\n", num_procs, my_rank);

	// ** Definizione del tipo di dato personalizzato **
	blockslen[0] = CHARSET_SIZE + 1;
	blockslen[1] = HASH_SIZE + 1;
	blockslen[2] = 1;

	MPI_Datatype types[UI_FIELDS] = { MPI_CHAR, MPI_CHAR, MPI_INT };
	MPI_Aint offsets[UI_FIELDS];

	offsets[0] = 0;
	offsets[1] = offsets[0] + (CHARSET_SIZE + 1) * sizeof(char);
	offsets[2] = offsets[1] + (HASH_SIZE + 1 + PADDING) * sizeof(char);

	MPI_Type_create_struct(UI_FIELDS, blockslen, offsets, types, &MPI_User_input);
	MPI_Type_commit(&MPI_User_input);
	// ** Fine tipo personalizzato **

	debug("MPI", "Avvio thread supervisor\n");

	pthread_create(&superv_id, NULL, supervisor, NULL);
	pthread_join(superv_id, &errno);

	debug("MPI", "Supervisor terminato con codice %d\n", *cerrno);


	shmdt(ui);

	/* shut down MPI */
	MPI_Type_free(&MPI_User_input);
	MPI_Finalize();

	debug("MPI", "Libreria MPI finalizzata con successo\n");

	return 0;
}

/**
 * Main loop per il thread di supervisione sulla computazione corrente del relativo worker thread
 */
void supervisor() {
	int flag, i, cancel, *lst_errno;
	int wrk_errno;
	char *plain;
	th_parms parms;
	pthread_t worker_id, shell_id, listener_id, audit_id;

	/* Se si tratta del processo master viene avviata anche la shell
	 * di interazione con l'utente, altrimenti si entra nel ciclo di polling
	 * direttamente.
	 */

	/*if(!my_rank)
		pthread_create(shell_id, NULL, shell, NULL);	//TODO: Parametri?

	else
		pthread_create(rshell_id, NULL, remote_shell, &cancel);
*/
	plain = malloc(MAX_PASSWD_LEN);


	//parms.last_try = plain;

	bzero(plain, MAX_PASSWD_LEN);

	pthread_create(&listener_id, NULL, listener, &plain);

	if(auditing)
		pthread_create(&audit_id, NULL, audit, &parms);

	pthread_create(&worker_id, NULL, worker, &parms);

	debug("SV", "attesa listener...\n");

	pthread_join(listener_id, &lst_errno);

	debug("SV", "listener terminato!\n");

	/* a questo punto si è verificata una delle seguenti condizioni di terminazione:
	 * un worker thread remoto ha trovato la password o l'ha trovata quello locale
	 * oppure è stata richiesta la terminazione forzata del programma dall'utente */
	pthread_cancel(worker_id);
	if(auditing)
		pthread_cancel(audit_id);

	pthread_join(worker_id, &wrk_errno);
	debug("SV", "Worker thread del processo %d terminato con codice %d\n", my_rank, wrk_errno);

	if(!my_rank){
		if(strlen(plain) != 0)
			printf("SV:: La password trovata è '%s'\n", plain);

		else
			printf("SV:: Password non trovata!!!\n");
	}
}

int listener(char **plain) {
	MPI_Status status;
	char buffer[MAX_PASSWD_LEN];
	int flag, quorum;

	quorum = num_procs;

	/* Il master controlla l'attività degli altri worker */
	if(!my_rank) {

		/* Finché esistono worker attivi */
		while(quorum){

			/* Controllo messaggio terminazione worker */
			MPI_Iprobe(MPI_ANY_SOURCE, TAG_COMPLETION, MPI_COMM_WORLD, &flag, &status);
			if(flag){
				MPI_Recv(&flag, 1, MPI_INT, status.MPI_SOURCE, TAG_COMPLETION, MPI_COMM_WORLD, &status);
				quorum--;		// Decrementa il numero dei worker ancora in attività

				if(flag){
					/* Un worker ha effettivamente trovato la password */
					MPI_Recv(*plain, MAX_PASSWD_LEN, MPI_CHAR, status.MPI_SOURCE, TAG_PLAIN, MPI_COMM_WORLD, &status);
					flag = status.MPI_SOURCE;
					break;
				}
			}

			/* Controllo auditing */
			MPI_Iprobe(MPI_ANY_SOURCE, TAG_AUDIT, MPI_COMM_WORLD, &flag, &status);
			if (flag) {
				/* Ricezione ultimo tentativo da parte di un worker */
				//debug("LST", "Messaggio remoto ricevuto\n");
				MPI_Recv(buffer, MAX_PASSWD_LEN, MPI_CHAR, status.MPI_SOURCE, TAG_AUDIT, MPI_COMM_WORLD, &status);

				debug("LST", "L'ultima password provata dal processo %d è '%s'\n", status.MPI_SOURCE, buffer);
			}


			/* Controllo richiesta terminazione asincrona */
			MPI_Iprobe(MPI_ANY_SOURCE, TAG_ABORT, MPI_COMM_WORLD, &flag, &status);
			if (flag) {
				/* La shell ha effettivamente richiesto la terminazione forzata */
				//debug("LST", "Messaggio remoto ricevuto\n");
				MPI_Recv(&flag, 1, MPI_INT, status.MPI_SOURCE, TAG_ABORT, MPI_COMM_WORLD, &status);

				debug("LST", "Processo %d: Shutdown forzato...\n", my_rank);
				break;
			}

			/* Attesa prima del prossimo polling per limitare l'uso delle risorse */
			usleep(LOOP_TIMEOUT);
		}
	}

	MPI_Bcast(&flag, 1, MPI_INT, 0, MPI_COMM_WORLD);

	return flag;
}



/**
 * Funzione di lavoro per i processi dedicati alla decrittazione della password (codificata)
 */
int worker(th_parms *parms) {
	int flag;
	char *cracked = malloc(MAX_PASSWD_LEN);

	debug("WRK", "Avvio worker thread da parte del processo %d...\n", my_rank);

	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
	flag = key_gen(my_rank, num_procs, &cracked, parms);

		/*
		 * Se si arriva a questo punto il worker thread
		 * ha terminato la propria esecuzione.
		 * E' possibile, allora, terminare il thread listener
		 */
	MPI_Send(&flag, 1, MPI_INT, 0, TAG_COMPLETION, MPI_COMM_WORLD);

	if(flag){
		// Invia la password al master
		debug("WRK", "Processo %d: Invio password al master '%s'\n", my_rank, cracked);
		MPI_Send(cracked, MAX_PASSWD_LEN, MPI_CHAR, 0, TAG_PLAIN, MPI_COMM_WORLD );
	}


	return flag;
}

int audit(th_parms *parms){

	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
	char tempt[MAX_PASSWD_LEN];
	int count;

	debug("AUDIT", "Thread auditing avviato sul processo %d", my_rank);

	while(1){

		pthread_mutex_lock(&parms->mutex);

		if(parms->count % 10485)
			continue;

		//debug("AUDIT", "Accesso last_try\n");
		//TODO:count
		strcpy(tempt, parms->last_try);

		//debug("AUDIT", "last_try='%s', count='%d'\n", tempt, parms->count);

		pthread_mutex_unlock(&parms->mutex);

		MPI_Send(tempt, MAX_PASSWD_LEN, MPI_CHAR, 0, TAG_AUDIT, MPI_COMM_WORLD);
	}

	return 0;
}

/**
 * Gestore di terminazione forzata
 */
void abort_mpi(){
	int i, flag;

	debug("MPI", "Processo %d: Gestore abort...\n", my_rank);

	flag = 1;
	for (i = 0; i < num_procs && !my_rank; i++) {
		debug("MPI", "Processo %d: Invio comando abort al processo %d\n", my_rank, i);
		MPI_Send(&flag, 1, MPI_INT, i, TAG_ABORT, MPI_COMM_WORLD );
	}
}

//////////////////////////////////////////////////////////////////////////////////

void listener2(char **_plain){
	MPI_Status status;
	int flag, i, quorum;
	//char *plain;
	char buffer[256];
	char *plain;

	plain = *_plain;
	quorum = num_procs;
	/* Ciclo di polling */

	while (1) {

		if(!my_rank && !quorum)


		/* Controllo messaggi in locale */
		if (*plain != '\0' ) {
			/* Il worker locale ha trovato la password:
			 * Invia a tutti i processi la comunicazione di interrompere la computazione */

			flag = 1;
			for (i = 0; i < num_procs; i++) {
				if(i == my_rank)
					continue;
				MPI_Send(&flag, 1, MPI_INT, i, TAG_COMPLETION, MPI_COMM_WORLD );
			}


			break;
		}

		/* Controllo messaggi remoti */
		MPI_Iprobe(MPI_ANY_SOURCE, TAG_COMPLETION, MPI_COMM_WORLD, &flag, &status);

		if (flag) {
			/* Un worker è terminato */
			verbose("MPI", "Messaggio remoto ricevuto");
			MPI_Recv(&flag, 1, MPI_INT, MPI_ANY_SOURCE, TAG_COMPLETION, MPI_COMM_WORLD, &status);

			/* Il supervisor arresta il worker locale *
				verbose("Crypto", "Arresto worker thread");
				pthread_cancel(worker_id);*/

			if(flag){
				/* La password è stata trovata da un worker */
				printf("Password trovata dal processo %d\n", status.MPI_SOURCE);

				break;
			}

			if(!flag){
				printf("Il processo %d ha terminato infruttuosamente la sua ricerca\n", my_rank);

				if(!my_rank)
					quorum--;
				else
					break;

			}
		}

		MPI_Iprobe(MPI_ANY_SOURCE, TAG_ABORT, MPI_COMM_WORLD, &flag, &status);

		if (flag) {
			/* La shell ha richiesto la terminazione forzata */
			verbose("MPI", "Messaggio remoto ricevuto");
			MPI_Recv(buffer, 1, MPI_INT, MPI_ANY_SOURCE, TAG_ABORT, MPI_COMM_WORLD, &status);

			printf("Processo %d: Shutdown forzato...\n", my_rank);
			break;
		}

		// Attesa prima del prossimo polling per limitare l'uso delle risorse
		usleep(LOOP_TIMEOUT);
	}
}

