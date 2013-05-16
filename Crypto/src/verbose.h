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

int verbose = 0,
	slow = 0;

/*#define debug(msg) #if DEBUG \
	printf("DEBUG:: %s\n", (msg)) \
	#endif*/

#define verbose(head, msg) do { \
	if(verbose) \
		printf("%s:: %s\n", (head), (msg)); \
		} while(0)

#endif /* VERBOSE_H_ */
