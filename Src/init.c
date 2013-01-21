#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include "init.h"
#include "search.h"
#include "errors.h"


#define TRACE printf("\nRiga %d: File :%s ---\n\n",__LINE__,__FILE__);
#define TRACE_N_REPORT(a) TRACE  printf("'%s'\n\n",a);
#define GET_SERVICE_DATA(c,a,b)	sscanf(c,"%[^'|']|%s",a,b) == 2 ? 0 : 1
#define GET_STOPS_DETAILS(d,a,b,c)	sscanf(d,"%[^'|']|%[^'|']|%s",a,b,c) == 3 ? 0 : 1



location *locations[HASH_ARRAY_SIZE];
location **Alocations;
service *services;

/**
 * Inizializza le SD
 * \return 0
 */
int init(void)
{

    services = NULL;
    loc_array_index = 0;
    loc_array_size = LOC_ARR_SIZE;
    Alocations = (location **) malloc(sizeof(location) * LOC_ARR_SIZE);
    if (!Alocations) {
	REPORT_N_EXIT(E_MALLOC, "init()");
    }

    return 0;

}

/**
 * Aumenta la dimensione della struttura dati Alocations
 * \retval 1 In caso di errori
 * \retval 0 Nessun errore
 */
static int enlarge_loc_array(void)
{


    location **tmp;
    int i;
    tmp =
	(location **) calloc(1, sizeof(location) *
			     (loc_array_size + LOC_ARR_SIZE));
    if (!tmp) {
	errid = E_MALLOC;
	return 1;
    }

    for (i = 0; i < loc_array_size; i++)
	tmp[i] = Alocations[i];
    loc_array_size += LOC_ARR_SIZE;
    free(Alocations);
    Alocations = tmp;

    return 0;
}

/**
 * Cancella un servizio
 * \param slist lista di servizi
 * \retval 0
 */
int removeService(service ** slist)
{

    service *t;
    t = *slist;
    if (!t)
	return 0;
    if (!t->next) {
	free(t);
	t = NULL;
    } else {
	for (; t->next && t->next->next; t = t->next);
	free(t->next);
	t->next = NULL;
    }
    return 0;
}

/**
 * Calcolo il costo di un arco, espresso come
 * minuti di differenza tra l'orario di arrivo e quello
 * di partenza
 * \param time_l Ora di partenza
 * \param time_a Ora di arrivo
 * \retval -1 In caso di errore
 * \retval costo Altrimenti
 */
static int getCost(char time_l[], char time_a[])
{
    time_t s1, s2;
    struct tm *t1, *t2;


    t1 = tChar2Tm(time_l);
    t2 = tChar2Tm(time_a);
    if (t2->tm_hour < t1->tm_hour
	|| (t2->tm_hour <= t1->tm_hour && t2->tm_min < t1->tm_min))
	t2->tm_mday += 1;

    s2 = mktime(t2);
    s1 = mktime(t1);
    free(t2);
    free(t1);
    if (s1 == -1 || s2 == -1)
	return -1;

    return (difftime(s2, s1) / 60);
}

/**
 * Carica i dati nelle SD dal file di configurazione
 * 	\param fd File descriptor
 *  \retval 1 In caso di errori
 *  \retval 0 Altrimenti
 */
int fillDS(FILE * fd)
{

    char riga[MAX_RECORD_SIZE + 1], sname[SERVICE_SIZE + 1],
	days[SERVICE_DAYS_SIZE + 1];
    service *p_service;


    if (!fd) {
	errid = E_NULL_FILE_POINTER;
	return 1;
    }

    while (!feof(fd)) {

	if (!fgets(riga, 100, fd) || !strcmp(riga, "\n"))
	    break;


	if (GET_SERVICE_DATA(riga, sname, days)) {
	    errid = E_CFILE_BAD_FORMAT;
	    return 1;
	}

	strcpy(riga, "");
	p_service = addService(sname, days);
	if (errid != OK) {
	    enotify("addService()");
	    errid = OK;
	    continue;		/*Salto il servizio */
	}
	getServiceRoute(fd, p_service);
	if (errid != OK)
	    return 1;
    }
    return 0;
}


/**
 * Carica in memoria le informazioni relative
 * ai percorsi effettuati da un servizio
 * \param fd File descriptor
 * \param serv Puntatore al servizio
 * \retval 1 In caso di errori
 * \retval 0 Altrimenti
 */
int getServiceRoute(FILE * fd, service * serv)
{

    char riga[2][MAX_RECORD_SIZE + 1], partenza[2][TIME_SIZE + 1],
	arrivo[2][TIME_SIZE + 1], station[2][STATION_SIZE + 1];
    location *loc1, *loc2, *last;


    if (!fd) {
	errid = E_NULL_FILE_POINTER;
	return 1;
    }

    /*      Reads first 2 lines             */
    fgets(riga[0], MAX_RECORD_SIZE, fd);
    fgets(riga[1], MAX_RECORD_SIZE, fd);

    /*      Get fields      */
    if (GET_STOPS_DETAILS(riga[0], arrivo[0], station[0], partenza[0])) {
	errid = E_CFILE_BAD_FORMAT;
	return 1;
    }
    if (GET_STOPS_DETAILS(riga[1], arrivo[1], station[1], partenza[1])) {
	errid = E_CFILE_BAD_FORMAT;
	return 1;
    }

    /* Create 2 new nodes   */

    if (!(loc1 = findNode(station[0]))) {

	loc1 = (location *) calloc(1, sizeof(location));
	if (!loc1) {
	    REPORT_N_EXIT(E_MALLOC, "getServiceRoute()");
	}
	loc1->fs = NULL;
	loc1->lservice = NULL;
	loc1->next = NULL;
    }

    if (!(loc2 = findNode(station[1]))) {

	loc2 = (location *) calloc(1, sizeof(location));
	if (!loc2) {
	    REPORT_N_EXIT(E_MALLOC, "getServiceRoute()");
	}
	loc2->fs = NULL;
	loc2->lservice = NULL;
	loc2->next = NULL;
    }

    /*      Fill them with datas    */
    !strlen(loc1->name) ? strcpy(loc1->name,
				 station[0]) : strcpy(loc1->name,
						      loc1->name);
    !strlen(loc2->name) ? strcpy(loc2->name,
				 station[1]) : strcpy(loc2->name,
						      loc2->name);

    loc1->lservice = putService(loc1->lservice, serv);
    /*      
     * Se putService ritorna NULL non creo l'edge tra i 2 nodi
     *  che vengono tuttavia inseriti
     */

    /*      Create an edge */
    loc1->fs = createArco(loc1->fs, partenza[0], arrivo[1], loc2);
    if (errid != OK) {
	removeService(&(loc1->lservice));
	enotify("getServiceRoute->createArco()");
	errid = OK;
    }

    last = addNode(loc1);
    if (!last) {
	REPORT_N_EXIT(errid, "addNode()");
    }

    last = addNode(loc2);
    if (!last) {
	REPORT_N_EXIT(errid, "addNode()");
    }


    /*      Riga[1] holds the last line that's been read */
    while (fgets(riga[0], MAX_RECORD_SIZE, fd) && strcmp(riga[0], "\n")) {

	if (GET_STOPS_DETAILS(riga[0], arrivo[0], station[0], partenza[0])) {
	    errid = E_CFILE_BAD_FORMAT;
	    return 1;
	}

	if (!(loc1 = findNode(station[0]))) {
	    loc1 = (location *) calloc(1, sizeof(location));
	    if (!loc1) {
		REPORT_N_EXIT(E_MALLOC, "getServiceRoute()");
	    }
	    loc1->next = NULL;
	    loc1->fs = NULL;
	    loc1->lservice = NULL;
	}


	!strlen(loc1->name) ? strcpy(loc1->name,
				     station[0]) : strcpy(loc1->name,
							  loc1->name);


	loc2->lservice = putService(loc2->lservice, serv);
	/*      
	 * Se putService ritorna NULL non creo l'edge tra i 2 nodi
	 *  che vengono tuttavia inseriti
	 */

	/*      Create an edge */
	loc2->fs = createArco(loc2->fs, partenza[1], arrivo[0], loc1);
	if (errid != OK) {
	    removeService(&(loc2->lservice));
	    enotify("createArco()");
	    errid = OK;
	}


	last = addNode(loc1);
	if (!last) {
	    REPORT_N_EXIT(errid, "addNode()");
	}

	/* Stores info of the last line */
	loc2 = loc1;
	strcpy(partenza[1], partenza[0]);
	strcpy(arrivo[1], arrivo[0]);
	strcpy(station[1], station[0]);

    }

    return 0;
}

/**
 * Aggiunge un servizio
 * \param nameService Nome del servizio
 * \param char bdays Giorno di servizio
 * \retval serv Il servizio inserito
 * \retval NULL in caso di errori
 */
service *addService(char nameService[], char bdays[])
{

    service *serv, *p;
    serv = (service *) calloc(1, sizeof(service));
    if (!serv) {
	errid = E_MALLOC;
	return NULL;
    }

    /* Fills data fields */
    serv->next = NULL;
    strcpy(serv->name, nameService);
    serv->working_days = atoi(bdays);

    if (!services)
	services = serv;

    else {
	for (p = services; p && p->next; p = p->next);
	p->next = serv;
    }

    return serv;
}

/**
 * Crea un nuovo arco
 * \param larchi La stella uscente del nodo
 * \param tleaving L'ora di partenza
 * \param tarrivig L'ora di arrivo
 * \param next Il nodo al quale dobbiamo connetterci
 * \return la stella aggiornata
 */
edge *createArco(edge * larchi, char tleaving[], char tarriving[],
		 location * next)
{

    edge *newArco, *tmp_a;

    newArco = (edge *) calloc(1, sizeof(edge));
    if (!newArco) {
	REPORT_N_EXIT(E_MALLOC, "createArco()");
    }

    /*      Fills data fields       */
    newArco->next = NULL;
    strcpy(newArco->tLeave, tleaving);
    strcpy(newArco->tArrive, tarriving);


    newArco->cost = getCost(tleaving, tarriving);

    if (errid != OK) {
	free(newArco);
	return larchi;
    }
    newArco->node = next;
    newArco->marked = 0;

    if (!larchi)
	larchi = newArco;
    else {

	for (tmp_a = larchi; tmp_a && tmp_a->next; tmp_a = tmp_a->next);

	tmp_a->next = newArco;
    }
    return larchi;
}

/**
 * Aggiunge un nuovo nodo
 * \param newNode Il nuovo nodo
 * \return Il nodo appena aggiunto
 * \retval NULL In caso di errori
 * */
location *addNode(location * newNode)
{


    int hash;
    location *node;

    if (newNode) {
	/* Ottengo l'hash del nome */
	hash = getIndexByHash(newNode->name);
	/*      Si trova in testa? */
	if (!locations[hash]) {



	    if (loc_array_index >= loc_array_size)
		enlarge_loc_array();
	    if (errid != OK) {
		return NULL;
	    }
	    newNode->index = loc_array_index;
	    Alocations[loc_array_index] = newNode;

	    locations[hash] = newNode;
	    loc_array_index++;
	    return locations[hash];

	} else {
	    /* Esiste già? */
	    node = findNode(newNode->name);

	    if (node)
		return node;
	    else {
		/*In caso non ci sia più spazio nella SD, aumento le sue dimensioni */
		if (loc_array_index >= loc_array_size)
		    enlarge_loc_array();
		if (errid != OK) {
		    enotify("addNode->enlarge_loc_array()");
		    errid = OK;
		    return NULL;
		}
		newNode->index = loc_array_index;
		/* Memorizzo il nodo in un array */
		Alocations[loc_array_index] = newNode;

		/* Nell'array di liste:  Scorro la lista e lo aggiungo in coda  */
		for (node = locations[hash]; node && node->next;
		     node = node->next);
		node->next = newNode;
		loc_array_index++;
		return node->next;
	    }
	}
    } else
	return NULL;

}

/**
 * Cerca un nodo
 * \param loc_name Il nome del nodo
 * \return Se trovato ,ritorna il nood
 * \retval NULL Se non è stato trovato
 */
location *findNode(char *loc_name)
{
    location *p;

    for (p = locations[getIndexByHash(loc_name)];
	 p && strcmp(loc_name, p->name); p = p->next);

    return p;
}

/**
 * Somma le lettere di una stringa
 * \param loc_name La stringa
 * \return La somma delle lettere
 */
int getIndexByHash(char *loc_name)
{
    int i, hash;

    for (i = 0, hash = 0; loc_name[i]; i++)
	hash += loc_name[i];
    return hash % HASH_ARRAY_SIZE;
}

/**
 * Aggiunge un servizio all'interno di una lista di servizi
 * di un nodo
 * \param list La lista dei servizi del nodo
 * \param n_serv Il servizio da aggiungere
 * \return La lista aggiornata dei servizi del nodo
 */
service *putService(service * list, service * n_serv)
{

    service *serv_p, *tmp;
    serv_p = (service *) calloc(1, sizeof(service));
    if (!serv_p) {
	REPORT_N_EXIT(E_MALLOC, "putService()");
    }

    serv_p->lservice = n_serv;
    serv_p->next = NULL;

    if (!list)
	list = serv_p;
    else {

	for (tmp = list; tmp && tmp->next; tmp = tmp->next);

	tmp->next = serv_p;

    }
    return list;

}
