/*
 * io.h
 *
 *  Created on: 16/mag/2013
 *      Author: mpiuser
 */

#ifndef IO_H_
#define IO_H_

#include "part.h"

#define MAX_THREADS 8

#define CMD_QUIT 0
#define CMD_EXEC 1
#define CMD_ABRT 2

typedef struct threads threads;

/**
 * Reppresenta il main loop di I/O per l'impostazione delle variabili di lavoro
 * e l'esecuzione dei comandi di ricerca delle chiavi
 */
void shell(user_input *ui);

void remote_shell();

#endif /* IO_H_ */
