/*
 * part.h
 *
 *  Created on: 28/apr/2013
 *      Author: davide
 */

#ifndef PART_H_
#define PART_H_

#include <math.h>

#include "mem.h"
#include "struct.h"

#define DISPOSITIONS(cs_len, pass_len) (pow((cs_len),(pass_len)));
#define DISP_PER_PROC(disp, num_procs) (((disp)/(num_procs))+1);
#define STARTING_CHAR(init, cs_size, pos) (((long)((init)/(pow(cs_size, pos))))%(cs_size));

#define BCAST_TAG 0


/**
 * Effettua il test sullo hash MD5 per la chiave generata.
 *
 * temporaneamente stampa solo la stringa prodotta
 *
 * @param str: Stringa da verificare
 *
 * @return: 0 in caso di fallimento, >0 altrimenti
 */
int test(char *pass);

/**
 * Verifica che l'intero 'pos' sia presente all'interno dell'insisme
 * 'set' di dimensione 'k'.
 *
 * La verifica, all'interno del programma 'Crypto' viene ad essere
 * utilizzata per vincolare determinati indici di caratteri della stringa
 * di output al valore originale, con l'intento di ridurre il numero di
 * permutazioni da calcolare per ciascun processo.
 *
 * @param pos: Intero da cercare
 * @param set: Puntatore (array) ad intero rappresentante il set
 * @param k: Dimensione dell'insieme
 *
 * @return: 0 in caso di fallimento, >0 altrimenti
 */
//int contains(int pos, int *set, int k);

/**
 * Calcola tutte le disposizione (con ripetizione) per una stringa
 * di lunghezza 'n' su un insieme di caratteri 'cs'.
 * Il cursore 'pos' rappresenta la posizione corrente di lavoro all'interno della stringa.
 * La funzione ignora tutte le permutazioni degli indici cosiddetti 'riservati' presenti
 * all'interno dell'insieme 'set' di dimensione 'size'.
 *
 * @param cs: Set di caratteri di lavoro
 * @param k: Dimensione del set di caratteri
 * @param pos: Cursore di posizione all'intero della stringa
 * @param current: Stringa di lavoro
 * @param n: Dimensione della stringa di lavoro
 * @param set: Insieme contenente tutti gli indici 'riservati'
 * @param size: Dimensione del set degli indici riservati
 */
int comb(comb_parms *combparms, int pos);


/**
 * Calcola la combinazione iniziale da cui il processo ennesimo
 * del gruppo di lavoro MPI deve cominciare la propria esecuzione.
 *
 * Il calcolo è legato al rank del processo all'interno dello stesso
 * gruppo di comunicazione MPI; nel caso si utilizzino gruppi differenti
 * non è garantito che il punto di partenza sia diverso per ogni
 * processo, pertanto non è possibile che venga eseguito lavoro
 * in eccesso.
 *
 * @parm init: Interno rappresentante il numero progressivo della combinazione iniziale
 * 		tale numero è calcolato a partire dal renk del processo
 * @parm cs_size: Dimensione del charset di lavoro
 * @parm passlen: Dimenesione della password che si vuole decrittare
 *
 * @return Zero in caso di successo, un interno negativo che rappresenta il codice
 * di errore relativo
 */
int *compute_starting_point(long init, int cs_size, int passlen);

/**
 * Punto di ingresso del programma di prova per la generazione
 * delle disposizioni su più processi/thread.
 *
 * @param reserved: Insieme degli indici riservati
 * @param k: Dimensione dell'insieme
 */
int key_gen(int rank, int num_procs, char **plain, th_parms *audit);


/**
 * Si occupa di pulire le strutture dati utilizzate dal thread, al
 * momento della richiesta di canellazione di quest'ultimo.
 */
void work_cleanup(allocation *alloc);

#endif /* PART_H_ */
