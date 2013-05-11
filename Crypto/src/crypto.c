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
#include "hash.h"
#include "crypto.h"

struct user_input{

	char cs[CHARSET_SIZE+1];
	unsigned char hash[HASH_SIZE];
	int passlen;

};

void get_user_input(user_input *ui){

	printf("Inserire charset: ");
	fflush(stdout);
	scanf("%s", ui -> cs);
	printf("\nInserire la  password: ");
	fflush(stdout);
	scanf("%s", ui -> hash);
	printf("\nInserire lunghezza della  password: ");
	fflush(stdout);
	scanf("%d", &(ui -> passlen));

	//printf("\nDEBUG:cs - %s, hash - %s, passlen - %d\n", ui -> cs, ui -> hash, ui -> passlen);

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
	//char *cs_fixed = "abcd";

	//cs_size = strlen(cs_fixed);
	//cs = malloc(cs_size+1 * sizeof(char));
	//strcpy(cs, cs_fixed);

	printf("Avvio mainMPI\n");
	printf("parametri di init:: argc=%d\n", argc);
	for(i=0; i<argc; i++){
		printf("argv[%d]= %s", i, argv[i]);
	}
	printf("\n");

	/* start up MPI */
	MPI_Init(&argc, &argv);

	printf("Inizializzazione terminata");

	/* find out process rank */
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

	/* find aout number of processes */
	MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

	printf("Numero di processi attivi= %d\nmy rank=%d\n", num_procs, my_rank);

	if(my_rank)printf("\nInserire hash della  password: ");

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

		//printf("DEBUG: Rank %d,uip:%p\n", my_rank, &ui);
		//printf("DEBUG: Rank %d, cs %s, passwd %s, passlen %d\n", my_rank, ui.cs, ui.hash, ui.passlen);

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
int main(int argc, char *argv[]){
	int ret, passlen, num, i;
	long phash;
	unsigned char buffer[256], *token, **tmp;
	unsigned char hash[HASH_SIZE];
	user_input ui;

	printf("==============================\n");
	printf("\t\tMAIN\n");
	printf("==============================\n");


	/* Avvia il main loop di shell per la configurazione */
	bzero(&ui, sizeof(ui));

	while(1){
		printf(">> ");

		bzero(buffer, sizeof(buffer));
		ret = fgets(buffer, sizeof(buffer), stdin);
		if(ret == NULL){
			printf("Errore sullo stdin! Esco\n");
			exit(-1);
		}

		token = strtok(buffer, " \n");
		if(token == NULL){
			continue;
		}

		/* Chiude il programma */
		if(!strcmp(token, "quit")){
			printf("Esco...\n");
			break;
		}

		if(!strcmp(token, "help")){
			printf("===== Help =====\n");
			printf("quit - per uscire\n");
			printf("hash 'string' - restituisce lo hash della stringa in input\n");
			printf("set | passlen, passwd, cs, hfunc - Imposta il valore di lavoro per la variabile indicata:\n");
			printf("cs 'num' - imposta il charset di lavoro per la generazione delle password.\n");
			printf("\tnum=0 -> sole minuscole\n");
			printf("\tnum=1 -> sole maiuscole\n");
			printf("\tnum=2 -> soli numeri\n");
			printf("\tnum=3 -> maiuscole e minuscole\n");
			printf("\tnum=4 -> numeri e minuscole\n");
			printf("\tnum=5 -> numeri e maiuscole\n");
			printf("\tnum=6 -> completo\n");
			printf("passswd 'string' - Rappresenta la password codificata (md5) da decrittare\n");
			printf("passlen 'num' - Numero di caratteri della password da trovare\n");
			printf("hfunc 'type' - Imposta la funzione di hash da utilizzare nella ricerca della passwd [funzione non ancora abilitata]\n");
			printf("run - per avviare la generazione delle password\n");
			printf("===============\n");
		}

		if(!strcmp(token, "set")){
			token = strtok(NULL, " ");

			if(!strcmp(token, "passlen")){
				token = strtok(NULL, "");
				ui.passlen = (int) strtol(token, NULL, BASE);
				printf("Impostata una lunghezza di password di %d\n", ui.passlen);
			}

			if(!strcmp(token, "passwd")){
				token = strtok(NULL, " ");

				printf("token = %p\n", token);

				for(i = 15; i >= 0; i--){
					phash = strtol(token, NULL, BASE_HASH);
					memcpy(ui.hash+i, (((char *) &phash) - i), 1);
				}

				printf("phash = %ul, token=%p\n", phash, token);

				printf("Impostata la password target = ");
				printHash(ui.hash);
			}

			if(!strcmp(token, "cs")){
				token = strtok(NULL, " ");

				num = (int) strtol(token, NULL, BASE);

				memcpy(charsets[num], ui.cs, strlen(charsets[num])+1);
				printf("Impostato il charset = '%s'\n", ui.cs);
			}
		}

		if(!strcmp(buffer, "run")){
			startMPI(0, NULL);
		}

		else if(!strcmp(buffer, "hash")){
			scanf("%s", buffer);
			hashMD5(buffer, hash);
			printf("> MD5('%s') = ", buffer);
			printHash(hash);
			printf("Impostato il valore dello hash\n");
		}

		puts("");
	}

	return 0;
}
