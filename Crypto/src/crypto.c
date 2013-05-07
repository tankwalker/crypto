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
#include <stddef.h>
#include <time.h>

#include "part.h"

struct user_input{

	char cs[CHARSET_SIZE+1];
	char hash[HASH_SIZE+1];
	int passlen;

};

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

void get_user_input(user_input *ui){

	printf("Inserire charset: ");
	fflush(stdout);
	scanf("%s", ui -> cs);
	printf("\nInserire hash della  password: ");
	fflush(stdout);
	scanf("%s", ui -> hash);
	printf("\nInserire lunghezza della  password: ");
	fflush(stdout);
	scanf("%d", &(ui -> passlen));

	printf("\nDEBUG:cs - %s, hash - %s, passlen - %d\n", ui -> cs, ui -> hash, ui -> passlen);

}

int startMPI(int argc, char *argv[]) {
	char message[100]; 	/* storage for message */
	int cs_size;
	int my_rank;		/* rank of process */
	int num_procs;		/* number of processes */
	int source; 		/* rank of sender */
	int dest = 0; 		/* rank of receiver */
	int passlen;
	int tag = 0; 		/* tag for messages */
	long chunk;
	long disp;
	long init;
	MPI_Status status; 	/* return status for receive */

	int blockslen[UI_FIELDS];
	user_input ui;

	int i;
	char *cs;
	//char *cs_fixed = "abcdefghijklmnopqrstuvwxyz";
	char *cs_fixed = "abcd";

	cs_size = strlen(cs_fixed);
	cs = malloc(cs_size+1 * sizeof(char));
	strcpy(cs, cs_fixed);

	/* start up MPI */
	MPI_Init(&argc, &argv);

	/* find out process rank */
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

	/* find aout number of processes */
	MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

	printf("Numero di processi attivi= %d\nmy rank=%d\n", num_procs, my_rank);

	if(my_rank)
		sleep(my_rank);

	passlen = 3;

	disp = DISPOSITIONS(cs_size, passlen); // Numero di disposizioni da calcolare
	chunk = DISP_PER_PROC(disp, num_procs); // Numero di disposizioni che ogni processo deve calcolare

	printf("DEBUG:Rank %d - Numero di disposizioni da calcolare: %lu.\n", my_rank, chunk);

	init = chunk * my_rank;

	 blockslen[0] = CHARSET_SIZE+1;
	 blockslen[1] = HASH_SIZE+1;
	 blockslen[2] = 1;

	 MPI_Datatype types[UI_FIELDS] = {MPI_CHAR, MPI_CHAR, MPI_INT};
	 MPI_Datatype MPI_User_input;
	 MPI_Aint offsets[UI_FIELDS];

	 offsets[0] = 0;
	 offsets[1] = offsets[0] + (CHARSET_SIZE+1) * sizeof(char);
	 offsets[2] = offsets[1] + (HASH_SIZE + 1 + PADDING) * sizeof(char);

	 MPI_Type_create_struct(UI_FIELDS, blockslen, offsets, types, &MPI_User_input);
	 MPI_Type_commit(&MPI_User_input);

	if(my_rank == 0) get_user_input(&ui);

	if(init <= disp){

		printf("DEBUG: Rank %d - I'm waiting\n", my_rank);
		MPI_Bcast(&ui, 1, MPI_User_input, 0, MPI_COMM_WORLD);
		printf("DEBUG: Rank %d - Received Bcast\n", my_rank);

		printf("DEBUG: Rank %d,uip:%p\n", my_rank, &ui);
		printf("DEBUG: Rank %d, cs %s, passwd %s, passlen %d\n", my_rank, ui.cs, ui.hash, ui.passlen);

		key_gen(my_rank, num_procs, &ui);
	}

	MPI_Type_free(&MPI_User_input);
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
