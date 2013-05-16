/*
 * hash.h
 *
 *  Created on: 10/mag/2013
 *      Author: mpiuser
 */

#ifndef HASH_H_
#define HASH_H_

#define BASE_16 16
#define X64

#define A_HEX_VALUE 10
#define HIGH_BYTE 4
#define HASH_SIZE 16
#define BASE 10		// Definisce la base di lavoro per la conversione di una stringa in long tramite 'strtol'
#define BASE_HASH 16	// Definisce la base di lavoro per la conversione della sequesnza di byte che rappresenta lo hash target della password


/**
 * Calcola lo hash MD5 per una qualsiasi stringa in input, il risultato
 * viene memorizzato a partire dal puntatore a carattere 'hash' preso come
 * parametro.
 *
 * @param plain - Puntatore a carattere che rappresenta la stringa su cui calcolare lo hash
 * @param hash . Puntatore a carattere che conterrà il valore di hash calcolato
 *
 * @return Zero in caso di successo, un codice di errore altrimenti //TODO: gestione errori
 */
int hashMD5(char *plain, unsigned char *hash);

/**
 * Converte un carattere esadecimale nel corrispettivo valore binario.
 *
 * @param c - Carattere da convertire
 *
 * @return Restituisce un intero corrispondente al valore esadecimale associato
 * al carattere in input; in caso di errore restituisce un valore negativo
 */
int hexToBin(unsigned char c);


/**
 * Funzione ausiliaria per la stampa a console del valore dello hash
 * cui corrisponde la stringa binaria passata in input
 *
 * @param hash - Puntatore a carattere rappresentante la stringa da stampare
 */
void printHash(unsigned char *hash);


/**
 * Converte la rappresentazione esadecimale in formato stringa nella corrispettiva
 * rappresentazione binaria.
 *
 * @param string - Puntatora a carattere della stringa da converire
 * @param buffer - Puntatore alla zona di memoria dove memorizzare il risultato
 * @param size - Numero di caratteri che la funzione si aspetta di dover elaborare
 *
 * @return Zero in caso di successo, un codice di errore altrimenti //TODO: gestione errori
 */
int strToBin(char *string, char *buffer, int size);

#endif /* HASH_H_ */
