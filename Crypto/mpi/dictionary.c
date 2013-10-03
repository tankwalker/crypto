/*
 * dictionary.c
 *
 * Sorgente del modulo operativo per la realizzazione
 * di un attacco a dizionario.
 *
 * Note:
 * Path del file di dizionario ./dict/resources/wordlist.txt
 *
 *  Created on: 22/lug/2013
 *      Author: mpiuser
 */

#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include "dictionary.h"
#include "hash.h"
#include "verbose.h"

FILE *fp = 0;
char *dict;	/* Buffer contenente il chunk di password da testare */
extern user_input *ui;

int initDict(int my_rank, int num_procs, th_parms *parm){
	int ret;

	debug("DAK","Apertura dizionario da parte del processo %d...\n", my_rank);
	ret = openDict(DICT_PATH);

	if(ret)
		return ret;

	sem_wait(&parm->mutex);

	dict = dictInRAM(my_rank, num_procs);

	sem_post(&parm->mutex);

	if(!dict)
		return 1;

	//debug("DAK","Dictionary: %s", dict);

	return 0;
}

/* Aprire e leggere il file di dizionario */
int openDict(char *path){

	fp = fopen(path, "r");
	if(!fp){
		debug("DAK", errno, "Errore nell'apertura del dizionario '%s' (%s)\n", path, strerror(errno));
		return 1;
	}
	return 0;
}

int closeDict(){

	free(dict);
	return fclose(fp);

}

char* dictInRAM(int my_rank, int num_procs){

	char *buffer;
	int count, fd, last, n, rem, ret, to_read;
	long chunk;
	long int size;
	struct stat file_st;


	if(!fp){
		debug("DAK", "File pointer non valido (%s)\n", strerror(errno));
		return NULL;
	}

	buffer = NULL;
	fd = fileno(fp);
	fstat(fd, &file_st);
	size = file_st.st_size;
	chunk = size / num_procs;

	debug("DAK", "Ogni processo leggerà dal dizionario %ld byte\n", chunk);

	dict = malloc((chunk + RED_ZONE) * sizeof(char));
	if(!dict){
		debug("DAK", "Impossibile allocare un buffer sufficientemente grande (%ld byte)\n", chunk + RED_ZONE);
		return NULL;
	}

	ret = fseek(fp, chunk*my_rank, SEEK_SET);
	if(ret){
		debug("DAK","Spostamento del file pointer non consentito (%s)\n", strerror(errno));
		return NULL;
	}
	debug("DAK","Lettura dizionario in corso da parte del processo %d...\n", my_rank);

	to_read = chunk;
	do {

		ret = read(fd, dict, chunk);
		to_read -= ret;

	} while(ret > 0 && to_read > 0);

	if(ret < 0)
		debug("DAK", "Errore nella lettura del dizionario da parte del processo %d (%s)...\n", my_rank, strerror(errno));

	last = strlen(dict)-1;

	if(dict[last] != '\n' && dict[last] != '\0'){
		ret = getline(&buffer, &n, fp);
		if(ret == -1){
			debug("DAK","Errore nella lettura dell'ultima password (%s)\n", strerror(errno));
			return NULL;
		}
		strcpy(dict+last+1, buffer);
		//if(my_rank==1) printf("%s",dict);
	}
	debug("DAK","Lettura dizionario completata da parte del processo %d...\n", my_rank);

	return dict;
}

// TODO Check se ho caricato già il dizionario in RAM
int dictAttack(int my_rank, int num_procs, th_parms *parms){

	char password[MAX_PASSWD_LEN];
	char trm = '\0';
	int i=0, j=0, ret;
	long count = 0;

	bzero(password, MAX_PASSWD_LEN * sizeof(char));
	//pthread_cleanup_push(dictWork_cleanup, NULL);
	debug("DAK","Inizializzazione del dizionario da parte del processo %d...\n", my_rank);

	//TODO Implementazione scelta dizionario "custom"
	ret = initDict(my_rank, num_procs, parms);

	/* Vi sono stati errori nell'inizializzazione del dizionario */
	if(ret){
		debug("DAK","Errore durante l'inizializzazione del dizionario da parte del processo %d...\n", my_rank);
		return ret;
	}


	/* Gestione caso dizionario vuoto */
	ret = 0;
	if(!dict[i])
		return 0;

	do{
		char c = dict[i++];
		if(c == '\n'){
			count++;
			memcpy(password+j-1, &trm, sizeof(char));
			ret = dakTest(password);
			if(!ret){
				strcpy(parms->plain, password);
				closeDict();
				//pthread_cleanup_pop(dictWork_cleanup);
				return 1;
			}
			strcpy(parms->last_try, password);
			parms->count = count;
			pthread_cond_broadcast(&parms->waiting);
			j = 0;
			bzero(password, MAX_PASSWD_LEN * sizeof(char));
		}
		else memcpy(password+j++, &c, sizeof(char));
	}while(dict[i] != '\0');

	closeDict();
	return 0;
}

int dakTest(char *pass) {
	unsigned char new_hash[HASH_SIZE];		//TODO spostare la malloc all'interno della funzione hashMD5?
	hashMD5(pass, new_hash);

	return hashcmp((char *) ui->hash, (char *) new_hash);
}

void dictWork_cleanup(){
	int ret = closeDict();
	if(!ret)
		debug("DAK_CLN","Chiusura dizionario correttamente eseguita\n");
	else
		debug("DAK_CLN","Chiusura dizionario fallita (Errore: %s)\n", strerror(errno));
}
