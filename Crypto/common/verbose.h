/*
 * verbose.h
 *
 * Definisce le funzioni di debugging
 *
 *  Created on: 16/mag/2013
 *      Author: mpiuser
 */

#ifndef VERBOSE_H_
#define VERBOSE_H_

#include <stdio.h>

#define DEBUG 1
#define VERBOSE 1
#define HEAD_PMT "%s:: "

#ifdef DEBUG

#define debug(head, ...) do {		\
	printf(HEAD_PMT, (head));		\
	printf(__VA_ARGS__);			\
	} while(0)

#else

#define debug(...) do {} while(0)

#endif

/*#define debug(msg) #if DEBUG \
	printf("DEBUG:: %s\n", (msg)) \
	#endif*/

#define verbose(head, ...) do { 	\
	if(VERBOSE){ 					\
		printf(HEAD_PMT, (head)); 	\
		printf(__VA_ARGS__); 		\
	}								\
	} while(0)


#define pprintf(head, ...) do {		\
		printf(HEAD_PMT, (head));	\
		printf(__VA_ARGS__);		\
	} while(0);

#endif /* VERBOSE_H_ */
