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


struct string_t {
	char *str;
	int size;
};

struct comb_settings {
	int *starting_point;
	long chunk;
};

struct comb_parms {
	string_t *cs, *passwd;
	comb_settings *init;
};

struct user_input {
	char cs[CHARSET_SIZE + 1];
	unsigned char hash[HASH_SIZE];
	int passlen;
};

// TODO:UNUSED
struct threads {
	int active[MAX_THREADS];
	pthread_t ids[MAX_THREADS];
};


