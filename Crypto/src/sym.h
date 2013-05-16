/*
 * syms.h
 *
 *  Created on: 10/mag/2013
 *      Author: mpiuser
 */

#ifndef SYSM_H_
#define SYSM_H_

#define DEBUG 1

#define CS_SIZE 6
#define MAX_ALLOC 32
#define PASSWD_FOUND 1

char *charsets[] = {"abcdefghijklmnopqrstuvwxyz",
					"ABCDEFEGHIJKLMNOPQRSTUVWXYZ",
					"1234567890",
					"abcdefghijklmnopqrstuvwxyzABCDEFEGHIJKLMNOPQRSTUVWXYZ",
					"1234567890abcdefghijklmnopqrstuvwxyz",
					"1234567890ABCDEFEGHIJKLMNOPQRSTUVWXYZ",
					"1234567890abcdefghijklmnopqrstuvwxyzABCDEFEGHIJKLMNOPQRSTUVWXYZ"};

#endif /* SYMS_H_ */
