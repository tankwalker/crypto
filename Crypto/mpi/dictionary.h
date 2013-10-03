/*
 * dictionary.h
 *
 *  Created on: 22/lug/2013
 *      Author: mpiuser
 */

#ifndef DICTIONARY_H_
#define DICTIONARY_H_
#define PERM 0755
#define MAX_RSIZE 256
#define RED_ZONE 256
#define DICT_PATH "/home/mpiuser/git/crypto/Crypto/mpi/resources/wordlist.txt"

#include "struct.h"

char* dictInRAM(int my_rank, int num_procs);

int closeDict();

int dakTest(char *pass);

int dictAttack(int my_rank, int num_procs, th_parms *parms);

int initDict(int my_rank, int num_procs, th_parms *parms);

int openDict(char *path);

void dictWork_cleanup();

#endif /* DICTIONARY_H_ */
