/*
 * io.h
 *
 *  Created on: 16/mag/2013
 *      Author: mpiuser
 */

#ifndef IO_H_
#define IO_H_

#include "part.h"
#include "hash.h"
#include "struct.h"

#define cmd_ready(flag) ((flag) & 0x1h)
#define passwd_ready(flag) ((flag) & 0x2h)

#define CMD_QUIT 0
#define CMD_EXEC 1
#define CMD_ABRT 2
#define PERMS 0666


/**
 * Reppresenta il main loop di I/O per l'impostazione delle variabili di lavoro
 * e l'esecuzione dei comandi di ricerca delle chiavi
 */
void shell();

void abort_mpi();

void quit();

void sig_halt();

int wait_child();

#endif /* IO_H_ */
