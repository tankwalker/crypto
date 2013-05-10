/*
 * hash.c
 *
 *  Created on: 10/mag/2013
 *      Author: mpiuser
 */

#include <mhash.h>
#include <string.h>

#include "part.h"
#include "syms.h"
#include "hash.h"

/**
 * La funzione prende come parametro un puntatore a carattere già inizializzato!
 */
int hashMD5(char *plain, unsigned char *hash) {
	MHASH td;
	int size, i;

	td = mhash_init(MHASH_MD5);
	if (td == MHASH_FAILED ) {
		return -1;		//TODO: Aggiungere codice di errore
	}

	size = strlen(plain);
	for(i=0; i<size; i++){
		mhash(td, plain+i, 1);
	}

	mhash_deinit(td, hash);

	return 0;

}

void printHash(unsigned char *hash) {
	int i;

	printf("0x");

	for (i = 0; i < mhash_get_block_size(MHASH_MD5); i++) {
		printf("%.2x", hash[i]);
	}

	printf("\n");
}
