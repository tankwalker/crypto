/*
 * sym.c
 *
 *  Created on: 11/giu/2013
 *      Author: mpiuser
 */


char *charsets[] = {"abcdefghijklmnopqrstuvwxyz",
					"ABCDEFEGHIJKLMNOPQRSTUVWXYZ",
					"1234567890",
					"abcdefghijklmnopqrstuvwxyzABCDEFEGHIJKLMNOPQRSTUVWXYZ",
					"1234567890abcdefghijklmnopqrstuvwxyz",
					"1234567890ABCDEFEGHIJKLMNOPQRSTUVWXYZ",
					"1234567890abcdefghijklmnopqrstuvwxyzABCDEFEGHIJKLMNOPQRSTUVWXYZ"};

char *help_msg = "===== Help =====\n" \
		"quit - per uscire\n" \
		"hash 'string' - restituisce lo hash della stringa in input\n" \
		"set | passlen, passwd, cs, hfunc - Imposta il valore di lavoro per la variabile indicata:\n" \
		"cs 'num' - imposta il charset di lavoro per la generazione delle password.\n" \
		"\tnum=0 -> sole minuscole\n" \
		"\tnum=1 -> sole maiuscole\n" \
		"\tnum=2 -> soli numeri\n" \
		"\tnum=3 -> maiuscole e minuscole\n" \
		"\tnum=4 -> numeri e minuscole\n" \
		"\tnum=5 -> numeri e maiuscole\n" \
		"\tnum=6 -> completo\n" \
		"passswd 'string' - Rappresenta la password codificata (md5) da decrittare\n" \
		"passlen 'num' - Numero di caratteri della password da trovare\n" \
		"hfunc 'type' - Imposta la funzione di hash da utilizzare nella ricerca della passwd [funzione non ancora abilitata]\n" \
		"run - per avviare la generazione delle password\n" \
		"===============\n";
