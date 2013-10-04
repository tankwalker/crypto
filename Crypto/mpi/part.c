/* ============================== part.c ==============================
 *
 * In questo modulo sono sviluppate le infrastrutture funzionali per
 * l'esecuzione dell'attacco a forza bruta. In particolare la
 * funzione per la gestione del partizionamento dell'insieme di
 * sottochiavi, che dovranno essere computate dai singoli processi,
 * è realizzata ricorsivamente e chiamata come sottofunzione della
 * funzione 'key_gen'. L'obiettivo è quello di permette una modularità
 * nella ricerca di password a dimensione variabile.
 *
 * ====================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "part.h"
#include "hash.h"
#include "mem.h"
#include "verbose.h"

// Dichiarazione varibili globali
extern user_input *ui;

int count, min, max, *set, filter, my_rank;
th_parms *audit_t;


/*
 * -------------------------------------------
 * test
 * -------------------------------------------
 */
int test(char *pass) {
	//if(!(count % 5000))	//TODO: varibile sulla dimenzione dello spazio di ricerca
	//debug("KEYGEN", "Processo %d => '%s'\n", my_rank, pass);

	count++;				// Contatore del numero di iterazioni (utilizzato per lo auditing)

	unsigned char new_hash[HASH_SIZE];		//TODO spostare la malloc all'interno della funzione hashMD5?
	hashMD5(pass, new_hash);				//TODO aggiungere controllo di errore

	return hashcmp((char *) ui->hash, (char *) new_hash);
}


/*
 * -------------------------------------------
 * comb
 * -------------------------------------------
 */
int comb(comb_parms *parms, int pos) {

	int i, cs_size = parms->cs->size;
	long chunk = parms->init->chunk;
	char *cs = parms->cs->str;
	char *passwd = parms->passwd->str;

	//printf("Parms:cs_size %d - chunk %lu - cs %s - passwd %s\n", cs_size, chunk, cs, passwd);

	// Combinazioni da tentare esaurite
	if (count == chunk)
		return 0;

	// Se la lunghezza della stringa costruita è uguale a quella della password
	if (pos == parms->passwd->size) {

		// si passa ad eseguire il controllo sul codice hash
		if (test(passwd) == 0) {
			// se i codici sono uguali la password è stata trovata e si esce
			return 1;
		}

			strcpy(audit_t->last_try, passwd);
			audit_t->count = count;
			pthread_cond_broadcast(&audit_t->waiting);

		// altrimenti si prosegue
		return 0;
	}

	// Se count è zero indica che si è nella fase di inizializzazione
	if (!count) {
		// caloclo della combinazione iniziale per la computazione
		for (i = (parms->init->starting_point)[pos];
				i < cs_size && count != chunk; i++) {

			passwd[pos] = cs[i];
			if (comb(parms, pos + 1))
				return 1;
		}
	}

	// altrimenti si prosegue a costruire la stringa da testare
	else {
		for (i = 0; i < cs_size && count != chunk; i++) {
			passwd[pos] = cs[i];
			if (comb(parms, pos + 1))
				return 1;
		}
	}

	return 0;
}


/*
 * -------------------------------------------
 * compute_starting_point
 * -------------------------------------------
 */
int *compute_starting_point(long init, int cs_size, int passlen) {

	int i;
	int *starting_point = malloc(passlen * sizeof(int));

	for (i = 0; i < passlen; i++) {

		int p = STARTING_CHAR(init, cs_size, i);
		starting_point[passlen - i - 1] = p;
	}

	return starting_point;
}


/*
 * -------------------------------------------
 * key_gen
 * -------------------------------------------
 */
int key_gen(int rank, int num_procs, th_parms *_audit) {		//TODO: package dei parametri MPI

	int *starting_point;				/// Identificativo della combinazione da cui avviare l'algoritmo
	int ret, passlen;
	long chunk, disp, init;
	allocation *allocs;					/// Struttura per la gestione della memoria allocata dal thread

	comb_parms *parms;					/// Raggruppamento delle successive strutture
	string_t *cs, *passwd;				/// Stringhe del charset selezionato e della password in chiaro
	comb_settings *settings;			/// Settaggi per l'algoritmo

	debug("KEYGEN", "Avvio partizionamento delle chiavi...\n");

	/* Gestione del layout di memoria interno tramite la struttura proprietaria */
	allocs = init_mem_layout();
	allocate(allocs, &parms, sizeof(parms));
	allocate(allocs, &cs, (2 * sizeof(string_t)));
	passwd = cs + 1;
	allocate(allocs, &settings, sizeof(comb_settings));
	/* fine gestione layout memoria */

	audit_t = _audit;
	count = 0;
	ret = 0;

	pthread_cleanup_push(work_cleanup, allocs);

	cs->str = ui->cs;
	cs->size = strlen(ui->cs);
	passwd->str = malloc((ui->passlen + 1) * sizeof(char)); // alloca spazio di 16 caratteri per la stringa di output
	passwd->size = ui->passlen;
	memset(passwd->str, '\0', (ui->passlen + 1) * sizeof(char)); // inizializza la stringa di lavoro

	/*
	 * Calcola il numero di disposizioni ed individua
	 * il numero progressivo della combinazione di partenza
	 */
	my_rank = rank;
	disp = DISPOSITIONS(cs->size, ui->passlen);		// Numero di disposizioni da calcolare
	chunk = DISP_PER_PROC(disp, num_procs);			// Numero di disposizioni che ogni processo deve calcolare

	init = chunk * my_rank;

	/* Calcola la combinazione di partenza ed imposta i parametri di lavoro */
	starting_point = compute_starting_point(init, cs->size, ui->passlen);

	settings->chunk = chunk;
	settings->starting_point = starting_point;

	parms->cs = cs;
	parms->passwd = passwd;
	parms->init = settings;

	/* Avvia la funzione di brute force */
	passlen = 1;	// TODO: testare il brute-force incrementale
	while (passlen < parms->passwd->size){
		ret = comb(parms, 0);
		if (ret) {
			/* Combinazione trovata; viene scritta nel buffer in chiaro */
			strcpy(_audit->plain, parms->passwd->str);
			break;
		}

		/* altrimenti la combinazione non è stata ancora trovata.
		 * Tale situazione può verificarsi o perché il charset
		 * di ricerca non è quello adeguato o perché la password
		 * provata non è di lunghezza sufficiente, presupponendo di
		 * avviare il brute force con la lunghezza di password minima
		 * rispetto al range impostato. Si procede pertanto ad un
		 * altro brute force con una lunghezza di chiave maggiore.
		 */

		passlen++;
	}

	pthread_cleanup_pop(work_cleanup);

	return ret;
}


/*
 * -------------------------------------------
 * work_cleanup
 * -------------------------------------------
 */
void work_cleanup(allocation *allocs) {
	debug("WRK_CLN", "Processo %d -> Count = %d\n", my_rank, count);

	/* Pulisce tutte le strutture dati allocte internamente */
	destroy_all(allocs);

	debug("WRK_CLN", "Exiting Work_cleanup...\n");
}
