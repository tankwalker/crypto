/*
 * signal.c
 *
 *  Created on: 17/mag/2013
 *      Author: mpiuser
 */


#include <mpi.h>
#include <pthread.h>

#include "io.h"

struct threads {
	int active[MAX_THREADS];
	pthread_t ids[MAX_THREADS];

};

extern threads running;

void controlled_shutdown(){
	printf("Shutdown\n");

	MPI_Finalize();
}

void halt(){
	int errno, i;

	printf("Ricevuto segnale di arresto forzato\n");

	for(i=0; i<MAX_THREADS; i++){
		if(running.active[i])
			pthread_cancel(running.ids[i]);
	}

	/*
	 * Ricevuto un segnale di interrupt e procede
	 * all'arresto di tutti i processi in esecuzione
	 */
	errno = MPI_Abort(MPI_COMM_WORLD, MPI_ERR_OTHER);

	MPI_Finalize();

	printf("Crypto:: MPI_Abort eseguita con codice %d\n", errno);
}
