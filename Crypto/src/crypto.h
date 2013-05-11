/*
 * crypto.h
 *
 *  Created on: 11/mag/2013
 *      Author: mpiuser
 */

#ifndef CRYPTO_H_
#define CRYPTO_H_

#define BASE 10		// Definisce la base di lavoro per la conversione di una stringa in long tramite 'strtol'
#define BASE_HASH 16	// Definisce la base di lavoro per la conversione della sequesnza di byte che rappresenta lo hash target della password

char *charsets[] = {"abcdefghijklmnopqrstuvwxyz",
		"ABCDEFEGHIJKLMNOPQRSTUVWXYZ",
		"1234567890",
		"abcdefghijklmnopqrstuvwxyzABCDEFEGHIJKLMNOPQRSTUVWXYZ",
		"1234567890abcdefghijklmnopqrstuvwxyz",
		"1234567890ABCDEFEGHIJKLMNOPQRSTUVWXYZ",
		"1234567890abcdefghijklmnopqrstuvwxyzABCDEFEGHIJKLMNOPQRSTUVWXYZ"};

#endif /* CRYPTO_H_ */
