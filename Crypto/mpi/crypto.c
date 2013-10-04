/*
 ============================================================================
 Name        : Crypto.c
 Author      : Davide Cingolani, Daniele Ucci
 Version     : 0.9
 Copyright   :
 Description : Modulo principale di infrastruttura generale dal quale è
			   possibile lanciare un attacco crittografico per MD5. Sono
			   disponibili le due modalità di attacco a forza bruta e
			   tramite utilizzo di dizionario.
			   L'attacco verrà lanciato su una griglia computazionale
			   servendosi della libreria MPI per il calcolo distribuito.
 ============================================================================
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stddef.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <fcntl.h>
#include <errno.h>
#include <sched.h>
#include <signal.h>
#include <semaphore.h>
#include <linux/unistd.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/syscall.h>

#include "crypto.h"
#include "part.h"
#include "hash.h"
#include "verbose.h"
#include "sym.h"
#include "cerrno.h"
#include "struct.h"
#include "dictionary.h"


/* Dichiarazione delle varibli Globali */
user_input *ui;			/// Struttura di raccolta dei parametri da utente
th_parms *ibus;			/// Struttura di BUS per la comunicazione inter-processo

int verbose, slow, auditing, attack;
int *wrk_errno, *aud_errno;			/// Codici di terminazione dei sottoprocessi
pthread_t keygen_id, audit_id;		/// Identificativi dei thread interni di lavoro
char last_try[256];				/// Buffer di memorizzazione dell'ultimo tentavio compiuto per l'auditing

int my_rank;					/// Rank del processo
int num_procs;					/// Numero di processi in esecuzione nella griglia
MPI_Datatype MPI_User_input;	/// Tipo di dato MPI custom per la comunicazione dei parametri di brute-force


/*
 * -------------------------------------------
 * main
 * -------------------------------------------
 */
int main(int argc, char *argv[]) {
	int blockslen[UI_FIELDS];	/// Array per la costruzione del tipo di dato MPI personalizzato
	int ret;

	ui = malloc(sizeof(user_input));

	if(argc > 1){	//TODO: # if(argc > _0_)[...]
		strToBin(argv[PARM_HASH], ui->hash, 2 * HASH_SIZE);
		ui->passlen = atoi(argv[PARM_PASSLEN]);
		strcpy(ui->cs, argv[PARM_CS]);
		ui->verbose = atoi(argv[PARM_VERBOSE]);
		ui->auditing = atoi(argv[PARM_AUDITING]);
		ui->attack = atoi(argv[PARM_ATTACK]);
	}

	else
		return -EPARM;

	/* Arma il segnale di terminazione affinché venga catturato dal
	 * gestore interno */
	signal(SIGTERM, abort_mpi);
	signal(SIGQUIT, halt_worker);	//TODO: da vedere come risolvere la gestione del segnale

	auditing = ui->auditing;
	verbose = ui->verbose;
	attack = ui->attack;

	/* start up MPI */
	debug("MPI", "Inizializzazione Libreria MPI...\n");
	MPI_Init(NULL, NULL);

	/* find out process rank */
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

	/* find out number of processes */
	MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

	debug("MPI", "Numero di processi attivi= %d\n", num_procs);
	debug("MPI", "My Rank= %d\n", my_rank);

	/* Definizione del tipo di dato personalizzato */
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

	debug("MPI", "Avvio thread supervisor da parte del processo %d\n", my_rank);
	ret = supervisor();

	debug("MPI", "Supervisor del processo %d terminato con codice %d\n", my_rank, ret);

	/* Deallocazione delle sturtture dati allocate */
	free(ui); // TODO: quit function?

	/* shut down MPI */
	MPI_Type_free(&MPI_User_input);
	MPI_Finalize();

	debug("MPI", "Libreria MPI finalizzata con successo\n");

	return 0;
}

/*
 * -------------------------------------------
 * supervisor
 * -------------------------------------------
 */
int supervisor() {
	int ret, status = 1;
	pid_t worker_pid;
	struct shmid_ds shmds;

	/* Allocazione delle strutture dati locali */
	wrk_errno = malloc(TH_NUM * sizeof(pthread_t));
	aud_errno = wrk_errno + 1;

	/* Inizializzazione delle varibli locali */
	*wrk_errno = -1;
	*aud_errno = 0;

	/* Creazione della shared memory */
	int shared;
	key_t key = PRIME*(my_rank+1);
	shared = shmget(key, sizeof(th_parms), IPC_CREAT|IPC_EXCL|PERM);

	/* Se non riesce a creare la shared memory, esce con un codice di errore */
	if(shared < 0){
		shmctl(shared, IPC_RMID, &shmds);
		panic("MPI", ESMEM, "Errore nella creazione della memoria condivisa con chiave %d. (%s)\n", key, strerror(errno));
	}
	ibus = shmat(shared, NULL, 0);
	if(!ibus)
		panic("MPI", ESMEM, "Errore nell'allocazione di memoria sul segmento condiviso %d. (%s)\n", shared, strerror(errno));
	debug("MPI", "Struttura di bus allocata sul segmento condiviso %d da parte del processo %d\n", shared, my_rank);

	/* Inizializzazione della struttura di bus condiviso */
	bzero(ibus, sizeof(th_parms));

	ibus->wterm = -1;
	sem_init(&ibus->mutex, 0, 1);

	if(auditing){
		pthread_mutex_init(&ibus->lock, NULL);
		pthread_cond_init(&ibus->waiting, NULL);
	}

	/* Worker: viene creato un nuovo processo figlio */
	worker_pid = fork();
	if(worker_pid < 0)
		debug("MPI", "Errore nella creazione del worker da parte del processo %d\n", my_rank);

	if(!worker_pid){
		ibus = shmat(shared, NULL, 0);
		if(!ibus)
			panic("WRK", ESMEM, "Errore nell'allocazione di memoria sul segmento condiviso %d. (%s) da parte del processo figlio del processo %d\n", shared, strerror(errno), my_rank);
		debug("WRK", "Struttura di bus allocata sul segmento condiviso %d da parte del processo figlio del processo %d\n", shared, my_rank);

		if(auditing)
			pthread_create(&audit_id, NULL, (void *) audit, ibus);

		pthread_create(&keygen_id, NULL, (void *) worker, ibus);

		ret = pthread_join(keygen_id, (void **) &wrk_errno);
		if(ret != 0)
			debug("WRK", "Oops, qualcosa è andato storto nella cancellazione del thread (keygen)...\n");

		if(auditing){
			ret = pthread_join(audit_id, NULL);
			if(ret != 0)
				debug("WRK", "Oops, qualcosa è andato storto nella cancellazione del thread (auditing)...\n");

			debug("WRK", "Audit del processo %d terminato con codice di errore %d\n", my_rank, *aud_errno);
			debug("WRK", "Deallocazione strutture dati dedicate ai thread da parte del processo %d\n", my_rank);
		}
		/* Il processo worker termina la propria esecuzione */
		exit(0);
	}

	/* Il processo di supervisione continua la propria esecuzione avviando la funzione di
	 * ascolto sul bus condiviso in attesa che il worker trovi (sperabilmente) la password
	 */
	listener(ibus);

	/* a questo punto si è verificata una delle seguenti condizioni di terminazione:
	 * 1. un worker thread remoto ha trovato la password o l'ha trovata quello locale
	 * 2. oppure è stata richiesta la terminazione forzata del programma dall'utente
	 * in entrambi i casi si può procedere con la terminazione forzata del processo worker
	 */

	kill(worker_pid, SIGQUIT);
	waitpid(worker_pid, &status, 0);
	debug("MPI", "Thread worker del processo %d terminato con codice errore %d\n", my_rank, status);

	/* nel caso questo sia il processo master per la libreria MPI, allora
	 * è necessario procedere anche alla lettura del buffer che contiene
	 * la password rilevata, se presente.
	 */
	if(!my_rank){
		if(strlen(ibus->plain) != 0){
			pprintf("SV", "La password trovata è '%s'\n", ibus->plain);
		}
		else
			pprintf("SV", "Password non trovata!!!\n");
	}

	/* Deallocazione strutture dati */
	debug("WRK", "Deallocazione strutture dati dedicate ai thread da parte del processo %d\n", my_rank);
	pthread_mutex_destroy(&ibus->lock);
	pthread_cond_destroy(&ibus->waiting);

	debug("SV", "Deallocazione strutture dati da parte del processo %d\n", my_rank);
	free(wrk_errno);
	sem_destroy(&ibus->mutex);
	shmdt(ibus);
	shmctl(shared, IPC_RMID, &shmds);

	return 0;
}


/*
 * -------------------------------------------
 * listener
 * -------------------------------------------
 */
int listener(th_parms *parms) {
	MPI_Status status;
	char buffer[MAX_PASSWD_LEN];
	int flag, quorum;

	quorum = num_procs;

	/* Finché esistono worker attivi, il valore della varibile 'quorum' è positivo*/
	while(quorum > 0){

		/* Controllo messaggio terminazione worker */
		MPI_Iprobe(MPI_ANY_SOURCE, TAG_COMPLETION, MPI_COMM_WORLD, &flag, &status);
		if(flag){
			/* Ricezione del messaggio di completamento */
			MPI_Recv(&flag, 1, MPI_INT, status.MPI_SOURCE, TAG_COMPLETION, MPI_COMM_WORLD, &status);
			quorum--;		// Decrementa il numero dei worker ancora in attività

			debug("LST", "Processo %d ha terminato con codice %d\n", status.MPI_SOURCE, flag);
			debug("LST", "Quorum residuo: %d\n", quorum);

			/* la variabile flag ora contiene l'informanzione se la password è stata rilevata */
			if(flag){
				/* Un worker ha effettivamente trovato la password */
				MPI_Recv(parms->plain, MAX_PASSWD_LEN, MPI_CHAR, status.MPI_SOURCE, TAG_PLAIN, MPI_COMM_WORLD, &status);
				flag = status.MPI_SOURCE;

				debug("LST", "Il processo %d ha trovato la password (%s)\n", status.MPI_SOURCE, parms->plain);

				/* Broadcast dell'evento di arresto. Non è necessario segnalare un evento
				 * specifico ma è sufficiente inviare una richiesta di terminazione. Se la
				 * password è stata realmente trovata, questo evento verrà successivamente
				 * verificato dal solo processo master al ritorno da questa funzione. */
				term();
			}
		}

		/* Controllo terminazione worker locale */
		sem_wait(&parms->mutex);
		flag = parms->wterm;
		sem_post(&parms->mutex);

		if(flag >= 0){
			debug("LST", "Processo %d ha terminato con %d\n", my_rank, flag);
			MPI_Send(&flag, 1, MPI_INT, 0, TAG_COMPLETION, MPI_COMM_WORLD);
			if(my_rank) quorum--;

			if(flag){
				/*
				 * Il worker thread non solo ha terminato la propria computazione
				 * ma anche con successo, pertanto invia la password trovata al
				 * master che si preoccuperà di fermare gli altri worker.
				 */
				debug("LST", "Processo %d: Invio password al master '%s'\n", my_rank, parms->plain);
				MPI_Send(parms->plain, MAX_PASSWD_LEN, MPI_CHAR, 0, TAG_PLAIN, MPI_COMM_WORLD);
			}

			parms->wterm = -1;
		}

		/* Controllo richiesta terminazione asincrona */
		MPI_Iprobe(0, TAG_ABORT, MPI_COMM_WORLD, &flag, &status);
		if (flag) {
			/* La shell ha effettivamente richiesto la terminazione forzata */
			MPI_Recv(&flag, 1, MPI_INT, 0, TAG_ABORT, MPI_COMM_WORLD, &status);
			flag = 0;
			MPI_Send(&flag, 1, MPI_INT, 0, TAG_COMPLETION, MPI_COMM_WORLD);

			debug("LST", "Processo %d: Segnale di terminazione...\n", my_rank);

			if(my_rank)
				break;
		}

		/* Attesa prima del prossimo polling per limitare l'uso delle risorse */
		usleep(LOOP_TIMEOUT);
	}

	debug("LST", "Processo %d: Break listening loop\n", my_rank);
	return parms->wterm;
}


/*
 * -------------------------------------------
 * worker
 * -------------------------------------------
 */
int worker(th_parms *parms) {
	int flag;

	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
	debug("WRK", "Avvio worker thread da parte del processo %d...\n", my_rank);

	switch(attack){
		case BRUTE_FORCE:
			flag = key_gen(my_rank, num_procs, parms);
			break;

		case DICT_ATTACK:
		default:
			flag = dictAttack(my_rank, num_procs, parms);
			break;
	}

	sem_wait(&parms->mutex);
	parms->wterm = flag;
	sem_post(&parms->mutex);

	*wrk_errno = flag;
	pthread_exit(wrk_errno);
}

/*
 * -------------------------------------------
 * audit
 * -------------------------------------------
 */
int audit(th_parms *parms){
	int state;
	char tempt[MAX_PASSWD_LEN];
	int percentage;

	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &state);
	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);

	/* Calcola ogni quanto indicare l'avanzamento nella UI */
	if(attack == DICT_ATTACK)
		percentage = compute_percentage(0);
	else {
		percentage = compute_percentage(1);
		if(percentage == -1L){
			*aud_errno = 1;
			pthread_exit(aud_errno);
		}
	}

	debug("AUDIT", "Thread auditing avviato sul processo %d\n", my_rank);
	debug("AUDIT", "Verrà stampata una password ogni %ld password\n", percentage);

	while(1){
		/* Vengono acquisiti il lock e la condizione di attesa (rilasciata dal worker) */
		pthread_mutex_lock(&parms->lock);
		pthread_cond_wait(&parms->waiting, &parms->lock);	// CS_PTHREAD INIT

		if(parms->count % percentage){
			// Se la percentuale non è pari al valore computato come soglia, ignora
			pthread_mutex_unlock(&parms->lock);
			continue;
		}

		//todo: formattazione dell'output di processo
		debug("LST", "L'ultima password provata dal processo %d è '%s' (%ld)\n", my_rank, parms->last_try, parms->count);

		pthread_mutex_unlock(&parms->lock); // CS_PTHREAD END
	}

	*aud_errno = 0;
	pthread_exit(aud_errno);
}

/**
 * Calcola ogni quanto inviare l'informazione dell'ultimo
 * tentativo al master.
 */

/*
 * long compute_percentage(){
	long chunk, disp;
	disp = DISPOSITIONS(strlen(ui->cs), ui->passlen); 	// Numero di disposizioni da calcolare
	chunk = DISP_PER_PROC(disp, num_procs); 			// Numero di disposizioni che ogni processo deve calcolare

	return (long)(chunk / PERCENTAGE);
}
*/
long compute_percentage(int mode){
	long ret;

	if(!mode){
		long chunk, disp;

		disp = DISPOSITIONS(strlen(ui->cs), ui->passlen); 	// Numero di disposizioni da calcolare
		chunk = DISP_PER_PROC(disp, num_procs); 			// Numero di disposizioni che ogni processo deve calcolare

		ret = (long)(chunk / PERCENTAGE);
	}
	else {
		int fd;
		long int size;
		struct stat file_st;

		fd = open(DICT_PATH, O_RDONLY);
		if(fd == -1){
			debug("PRC","Errore nell'apertura del dizionario\n");
			return -1L;
		}

		fstat(fd, &file_st);
		size = file_st.st_size;
		ret = ((size / num_procs)/PERCENTAGE)/PASS_CHAR_MEAN;
	}
	return ret;
}

/**
 * Gestore di terminazione forzata
 */
inline void abort_mpi(){
	debug("MPI", "Processo %d: Richiesta abort...\n", my_rank);
	term();
}

/**
 * Invare in broadcast ascincrona il segnale di arresto a tutti i processi attivi.
 */
inline void term(){
	int i, flag = 1;
	for (i = 0; i < num_procs; i++) {
		debug("MPI", "Processo %d: Invio comando terminazione al processo %d\n", my_rank, i);
		MPI_Send(&flag, 1, MPI_INT, i, TAG_ABORT, MPI_COMM_WORLD);
	}
}

/*
 * Arresta il thread del processo worker
 */
inline void halt_worker(){
	/*if(!(pthread_cancel(keygen_id))){
		debug("TRM", "Keygen del processo %d terminato con codice di errore %d\n", my_rank, *wrk_errno);
	}*/
}
