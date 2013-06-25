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

int launchMPI();

void supervisor();

int audit(th_parms *parms);

int listener(char **plain);

int worker(th_parms *parms);

void abort_mpi();


#endif /* CRYPTO_H_ */
