/*
 * signal.c
 *
 *  Created on: 17/mag/2013
 *      Author: mpiuser
 */


#include <mpi.h>
#include <pthread.h>

/*
void controlled_shutdown(){
	MPI_Finalize();
}

void halt(){
	int cmd;

	cmd = CMD_ABRT;
	MPI_Bcast(&cmd, 1, MPI_INT, 0, MPI_COMM_WORLD);
}*/
