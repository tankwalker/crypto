/*
 * part.h
 *
 *  Created on: 28/apr/2013
 *      Author: davide
 */

#ifndef PART_H_
#define PART_H_

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
 * @return: 0 in caso di fallimento, >0 altrimenti
 */
int contains(int pos, int *set, int k);

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
int comb(char *cs, int k, int pos, char *current, int n, int size);


/**
 * Punto di ingresso del programma di prova per la generazione
 * delle disposizioni su più processi/thread.
 *
 * @param reserved: Insieme degli indici riservati
 * @param k: Dimensione dell'insieme
 */
int key_gen(int *reserved, int k);

#endif /* PART_H_ */
