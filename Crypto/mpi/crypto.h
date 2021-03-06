/*
 * crypto.h
 *
 *  Created on: 11/mag/2013
 *      Author: mpiuser
 */

#ifndef CRYPTO_H_
#define CRYPTO_H_

#include <mpi.h>
#include <pthread.h>

#include "struct.h"

/* Definizione dei tag per i messaggi di lavoro */
#define TAG_COMPLETION	1		/// Messaggio di verifica di completamento del lavoro
#define TAG_ABORT		2		/// Messaggio di abort
#define TAG_PLAIN		3		/// Messaggio di comunicazione della password in chiaro
#define TAG_AUDIT		4		/// Messaggio di comunicazione dell'ultima password tentata

/* Posizioni numeriche dei parametri passati al processo MPI */
#define PARM_HASH		1
#define PARM_PASSLEN	2
#define PARM_CS			3
#define PARM_VERBOSE	4
#define PARM_AUDITING	5
#define PARM_ATTACK		6

#define LOOP_TIMEOUT	500		/// Tempo (us) di attesa nel cilco di controllo sulla condizione di terminazione
#define PERM			0755	/// Permessi di accesso alla shared memory
#define PERCENTAGE		10		/// Percentuale che indica ogni quante password ne viene stampata una dal thread di auditing
#define TH_NUM		2			/// Numero di thread utilizzati dal supervisor
#define PRIME		7			/// Numero primo per la scelta della chiave del segmento condiviso
#define PASS_CHAR_MEAN 8		/// Numero medio di caratteri delle password contenute nel dizionario
#define BRUTE_FORCE	0			/// Modalità di attacco a forza bruta
#define DICT_ATTACK	1			/// Modalità di attacco a dizionario


/**
 * Sezione di ascolto dei messaggi che viaggiano sulla libreria MPI.
 * Consente di rilevare la termianzione di un worker-process e di richiedere
 * l'arresto degli altri nel caso sia stata trovata una password.
 * Questa funzione è chiamata all'interno del supervisor.
 */
int listener(th_parms *parms);

/**
 * Main loop per il thread di supervisione sulla computazione corrente del relativo worker thread
 */
int supervisor();

/**
 * Funzione che esegue il lavoro di decrittazione, sia esso in brute-force o come attacco a dizionario.
 */
void worker(th_parms *parms);

/**
 * Rappresenta il gestore delle statistiche sull'attacco per la User Interface.
 * Viene realizzato come thread del processo worker.
 */
void audit(th_parms *parms);

/**
 * Funzione ausliaria necessaria per calcolare ogni quanto aggiornare le statistiche sull'attacco
 * nella UI della shell.
 */
long compute_percentage(int mode);

/**
 * Richiede l'arresto forzato dell'intera infrastruttura
 */
inline void abort_mpi();

/*
 * Diffonde in broadcast il segnale di arresto forzato
 */
inline void term();

/**
 * Gestisce la terminazione del thread locale per il processo worker
 */
inline void halt_worker_thread();

#endif /* CRYPTO_H_ */
