/*
 ============================================================================
 Name        : Crypto.c
 Author      : Davide
 Version     :
 Copyright   :
 Description : Calculate Pi in MPI
 ============================================================================
 */

#include <mpi.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

void calc_pi(int rank, int num_procs) {
	int i;
	int num_intervals;
	double h;
	double mypi;
	double pi;
	double sum;
	double x;

	/* set number of intervals to calculate */
	if (rank == 0) {
		num_intervals = 100000000;
	}

	/* tell other tasks how many intervals */
	MPI_Bcast(&num_intervals, 1, MPI_INT, 0, MPI_COMM_WORLD );

	/* now everyone does their calculation */

	h = 1.0 / (double) num_intervals;
	sum = 0.0;

	for (i = rank + 1; i <= num_intervals; i += num_procs) {
		x = h * ((double) i - 0.5);
		sum += (4.0 / (1.0 + x * x));
	}

	mypi = h * sum;

	/* combine everyone's calculations */
	MPI_Reduce(&mypi, &pi, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD );

	if (rank == 0) {
		printf("PI is approximately %.16f\n", pi);
	}
}

int startMPI(int argc, char *argv[]) {
	int my_rank;		/* rank of process */
	int num_procs;		/* number of processes */
	int source; 		/* rank of sender */
	int dest = 0; 		/* rank of receiver */
	int tag = 0; 		/* tag for messages */
	char message[100]; 	/* storage for message */
	MPI_Status status; 	/* return status for receive */

	char *cs;
	//char *cs_fixed = "abcdefghijklmnopqrstuvwxyz";
	char *cs_fixed = "abcd";
	int passlen;

	cs = malloc((strlen(cs_fixed)+1) * sizeof(char));
	strcpy(cs, cs_fixed);

	/* start up MPI */
	MPI_Init(&argc, &argv);

	/* find out process rank */
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

	/* find aout number of processes */
	MPI_Comm_size(MPI_COMM_WORLD, &num_procs);


	printf("Numero di processi attivi= %d\nmy rank=%d\n", num_procs, my_rank);

	if(my_rank)
		sleep(4*my_rank);

	passlen = 4;

	key_gen(my_rank, num_procs, cs, passlen);

	/*if(rank==0){
		MPI_Recv()
	}*/

	/* shut down MPI */
	MPI_Finalize();

	return 0;
}


/*
 * #### main ####
 *
 * Punto di ingresso del programma, da questo potranno essere
 * chiamati tutti i sotto moduli di Crypto
 */
int main(){
	startMPI(0, NULL);
	return 0;
}
