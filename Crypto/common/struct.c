/*
 * struct.c
 *
 * Contiene definizioni e metodi per l'utilizzo delle strutture dati interne al
 * software.
 *
 *  Created on: 10/giu/2013
 *      Author: mpiuser
 */

#include "struct.h"

// TODO:UNUSED
struct threads {
	int active[MAX_THREADS];
	pthread_t ids[MAX_THREADS];
};


