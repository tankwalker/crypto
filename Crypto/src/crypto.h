/*
 * crypto.h
 *
 *  Created on: 11/mag/2013
 *      Author: mpiuser
 */

#ifndef CRYPTO_H_
#define CRYPTO_H_

#include <mpi.h>

/* Definizione dei tag per i messaggi di lavoro */
#define TAG_COMPLETION 1		// Messaggio di verifica di completamento del lavoro
#define TAG_PLAIN 2				// Messaggio di comunicazione della password in chiaro

#define LOOP_TIMEOUT 1		// Tempo (s) di attesa nel cilco di controllo sulla condizione di terminazione

void supervisor();

void worker();

#endif /* CRYPTO_H_ */
