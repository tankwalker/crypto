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

/* Definizione dei tag per i messaggi di lavoro */
#define TAG_COMPLETION 1		// Messaggio di verifica di completamento del lavoro
#define TAG_ABORT 2				// Messaggio di abort
#define TAG_PLAIN 3				// Messaggio di comunicazione della password in chiaro
#define TAG_AUDIT 4				// Messaggio di comunicazione dell'ultima password tentata

#define LOOP_TIMEOUT 500		// Tempo (us) di attesa nel cilco di controllo sulla condizione di terminazione
#define PERM 0755				// Permessi di accesso alla shared memory
#define PERCENTAGE 10			// Percentuale che indica ogni quante password ne viene stampata una dal thread di auditing
#define TH_NUM 3				// Numero di thread utilizzati dal supervisor
#define PRIME 7					// Numero primo per la scelta della chiave del segmento condiviso
#define PASS_CHAR_MEAN 8		// Numero medio di caratteri delle password contenute nel dizionario

int audit(th_parms *parms);

int dictWorker();

int launchMPI();

int listener(th_parms *parms);

int supervisor();

int worker(th_parms *parms);

long computePercentage(int mode);

void abort_mpi();

void init();

void quit();

void term();

void term_worker();

#endif /* CRYPTO_H_ */
