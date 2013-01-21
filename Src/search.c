#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include "gtype.h"
#include "init.h"
#include "set.h"
#include "errors.h"

#define MAX_TRIP_LENGTH 1440
#define TRACE printf("\nRiga %d: File :%s ---\n\n",__LINE__,__FILE__);
#define RESET_MARKS resetMarks(p1,indice); resetMarks(p2,indice); resetMarks(p3,indice);
#define MAX_ACCURACY_VALUE 1440
#define FREE_N_EXIT(err,a) \
	freeArray((void **) p, loc_array_index);\
	freeArray((void **) d, loc_array_index);\
	free(p);\
	free(d);\
	REPORT_N_EXIT(err,a);

#define CHECK_SET_WDAY(a,b)\
		  t1=tChar2Tm(a);\
		  mktime(t1);\
		  t2=tChar2Tm(b);\
		  mktime(t2);\
		  if(t1->tm_hour > t2->tm_hour || (t1->tm_hour == t2->tm_hour && t1->tm_min > t2->tm_min) )\
		  wday= (wday+1)%7 == 0 ? 1: (wday+1)%7;\
		  free(t1);\
		  free(t2);

#define CLEAN_PATHS(a,b,c)\
	free(a[Lnode->index]->pedge);\
	freeArray((void **) a, loc_array_index);\
	free(a);\
	free(b[Lnode->index]->pedge);\
	freeArray((void **) b, loc_array_index);\
	free(b);\
	free(c[Lnode->index]->pedge);\
	freeArray((void **) c, loc_array_index);\
	free(c);

/**
 * Controlla se il servizio è attivo nel giorno indicato
 * \param serv_days Giorni nei quali il servizio è attivo
 * \param wday Giorno della settimana
 * \retval 1 se wday è contenuto nei giorni di servizio
 * \retval 0 altrimenti
 */
static int isServiceActive(unsigned int serv_days, unsigned int wday)
{

    while (serv_days)
	if ((serv_days % 10) == wday)
	    return 1;
	else
	    serv_days = serv_days / 10;
    return 0;

}

/**
 * Converte una stringa hh:mm in una struttura struct tm
 * \param t_char Stringa hh:mm
 * \retval t Puntatore ad una struttura struct tm
 * \retval NULL In caso di errore
 */
struct tm *tChar2Tm(char *t_char)
{

    struct tm *t;
    time_t t_plain;
    char *hours, *min, tmp[6];

    t = (struct tm *) calloc(1, sizeof(struct tm));
    if (!t) {
	errid = E_MALLOC;
	return NULL;
    }

    strncpy(tmp, t_char, 6);

    hours = strtok(tmp, ":");
    min = strtok(NULL, ":");

    time(&t_plain);
    localtime_r(&t_plain, t);

    t->tm_hour = atoi(hours);
    t->tm_min = atoi(min);
    return t;

}

/**
 * Restituisce il numero di minuti tra un'ora e l'altra
 * Gli orari sono contenuti nelle strutture edge passate
 * come parametri
 * \param arriving_edge arco entrante al nodo
 * \param leaving_edge arco uscente dal nodo
 * \return N minuti di differenza tra l'ora nell'arco uscente e quello entrante.
 * \return -1 in caso di errore
 */
static int getDiffTime(edge * arriving_edge, edge * leaving_edge)
{

    struct tm *t1, *t2;
    time_t rtime1, rtime2;

    t1 = tChar2Tm(arriving_edge->tArrive);
    if (errid == E_MALLOC) {
	REPORT_N_RETURN(E_MALLOC, "getDiffTime->tChar2TM()", -1);
    }

    t2 = tChar2Tm(leaving_edge->tLeave);
    if (errid != OK) {
	free(t1);
	REPORT_N_RETURN(E_MALLOC, "getDiffTime->tChar2TM()", -1);
    }

    if (t2->tm_hour < t1->tm_hour
	|| (t2->tm_hour <= t1->tm_hour && t2->tm_min < t1->tm_min))
	t2->tm_mday += 1;

    rtime1 = mktime(t1);
    rtime2 = mktime(t2);

    free(t2);
    free(t1);

    return (int) (rtime2 - rtime1) / 60;

}

/**
 * Restutisce il numero di archi uscenti
 * \param fs stella archi uscenti
 * \return #n numero archi uscenti.
 */
static int starDegree(edge * fs)
{
    if (!fs)
	return 0;
    else
	return 1 + starDegree(fs->next);
}

/**
 * Effettua free di locazioni di memoria occupate
 * \param a puntatore da liberare
 * \param i contatore locazioni da liberare
 */
static void freeArray(void **a, int i)
{
    int j = 0;
    if (a)
	for (; j < i; j++)
	    free(a[j]);

}

/*
 * Alla fine dell' algoritmo, basta accedere alla posizione p[destinazione->index]
 * e percorrere a ritroso per ottenere il percorso 
 */
/**
 * Esegue l'algoritmo per calcolare un percorso
 * Ritorna un array di puntatori.
 * Per visualizzare il percorso, occorre partire dalla
 * locazione individuata dall'indice del nodo destinazione.
 * 
 * \param root puntatore al nodo di partenza
 * \param wday giorno di partenza
 * \param root_tleave orario di partenza
 * \retval p array di puntatori parent
 * 
 */
static parentNode **makeSPT(location * root, unsigned int wday,
			    char *root_tleave)
{

    parentNode **p;
    costi **d;
    location **set, *el;
    edge *tarco;
    service *tmp_serv;
    struct tm *t1, *t2;

    int i, time_diff;

    /*      Init strutture dati     */
    p = (parentNode **) calloc(loc_array_index, sizeof(parentNode *));
    if (!p) {
	REPORT_N_EXIT(E_MALLOC, "makeSPT()");
    }
    d = (costi **) calloc(loc_array_index, sizeof(costi *));
    if (!d) {
	free(p);
	REPORT_N_EXIT(E_MALLOC, "makeSPT()");
    }

    for (i = 0; i < loc_array_index; i++) {
	p[i] = (parentNode *) calloc(1, sizeof(parentNode));
	if (!p[i]) {
	    freeArray((void **) p, i);
	    free(p);
	    free(d);
	    REPORT_N_EXIT(E_MALLOC, "makeSPT()");
	}
	p[i]->parent = NULL;
	p[i]->pedge = NULL;

	strcpy(p[i]->tArrive, "");
	d[i] = (costi *) calloc(1, sizeof(costi));
	if (!d[i]) {
	    FREE_N_EXIT(E_MALLOC, "makeSPT()");
	}
	d[i]->time_cost = MAX_TRIP_LENGTH;
	d[i]->accuracy = MAX_ACCURACY_VALUE;
    }

    /*      Fine init       */

    /*
     * Controlli preliminari
     */
    for (i = 0; i < loc_array_index; i++)
	assert(d[i]->time_cost == MAX_TRIP_LENGTH);
    for (i = 0; i < loc_array_index; i++)
	assert(p[i]->parent == NULL);

    p[root->index]->parent = NULL;
    p[root->index]->pedge = (edge *) calloc(1, sizeof(edge));	/*creo un edge fittizio per inserire orario di partenza */
    if (!p[root->index]->pedge) {
	FREE_N_EXIT(E_MALLOC, "makeSPT()");
    }
    strcpy(p[root->index]->pedge->tArrive, root_tleave);
    p[root->index]->pedge->node = NULL;

    strcpy(p[root->index]->tArrive, "");
    d[root->index]->accuracy = 0;
    d[root->index]->time_cost = 0;

    /*      Genero l'insieme        */
    set = get_set();
    if (!set) {
	free(p[root->index]->pedge);
	FREE_N_EXIT(E_MALLOC, "get_set()");

    }

    /* Inserisco l'elmento nell'insieme */
    if (set_insert(set, root)) {
	free(p[root->index]->pedge);
	FREE_N_EXIT(E_MALLOC, "set_insert()");

    }

    /*
     * Itero fin quando non è vuoto
     */
    while (!is_empty(set)) {

	/* Prendo un elemento */

	el = set_get(set);

	/* Lo rimuovo dall'insieme      */

	set_remove(set, el);

	/*      Memorizzo i puntatori alla lista dei servizi e alla stella uscente
	 *      del nodo
	 */
	tarco = el->fs;
	tmp_serv = el->lservice;

	/* Itero fin quando ho visitato tutti i gli archi della stella uscente  */
	while (tarco) {

	    time_diff = getDiffTime(p[el->index]->pedge, tarco);


	    if (time_diff == -1) {
		free(p[root->index]->pedge);
		set_delete(set);
		FREE_N_EXIT(E_MALLOC, "getDiffTime()");
	    }



	    if ((d[el->index]->time_cost + tarco->cost) <= d[tarco->node->index]->time_cost	/* Condizioni sul costo, inteso come durata della tratta    */
		&& time_diff <= d[tarco->node->index]->accuracy	/*      Condizione per selezionare la partenza con orario più vicino all'ora di arrivo al nodo attuale */
		&& (time_diff >= 10
		    || (time_diff < 10
			&& strcmp(p[el->index]->s_name,
				  tmp_serv->lservice->name) == 0))
		/* Almeno 10 min tra cambi di mezzo, anche <10 altrimenti */
		&&!tarco->marked	/* Sono già passato da qui? */
		&& isServiceActive(tmp_serv->lservice->working_days, wday)	/*Il servizio è attivo ora? */
		) {
		/* Update dei valori */
		CHECK_SET_WDAY(tarco->tLeave, tarco->tArrive);

		d[tarco->node->index]->time_cost =
		    d[el->index]->time_cost + tarco->cost;
		d[tarco->node->index]->accuracy = time_diff;


		p[tarco->node->index]->parent = el;
		strcpy(p[tarco->node->index]->s_name,
		       tmp_serv->lservice->name);
		p[tarco->node->index]->pedge = tarco;
		strcpy(p[tarco->node->index]->tArrive, tarco->tArrive);



		if (!is_in(set, tarco->node))
		    if (set_insert(set, tarco->node)) {
			free(p[root->index]->pedge);
			set_delete(set);
			FREE_N_EXIT(E_MALLOC, "set_insert()");
		    }

	    }
	    /* Al prossimo arco/servizio */
	    tmp_serv = tmp_serv->next;
	    tarco = tarco->next;

	}
    }

    set_delete(set);

    freeArray((void **) d, loc_array_index);
    free(d);
    return p;
}





/**
 * Estrae i campi da una richiesta
 * \param request La richiesta
 * \param date[out] Puntatore ad area di memoria per mem il campo data
 * \param time[out] Puntatore ad area di memoria per mem il campo ora
 * \param lLoc[out] Puntatore ad area di memoria per mem il campo luogo di partenza
 * \param aLoc[out] Puntatore ad area di memoria per mem il campo luogo di arrivo
 * \retval 0 Estrazione corretta
 * \retval 1 In caso di errore
 */
static int getRequestField(char *request, char *date, char *time,
			   char *lLoc, char *aLoc)
{

    int x;
    char *tmp, *head_p, *p;
    x = 0;
    tmp = (char *) calloc(1, sizeof(char) * (MAX_RECORD_SIZE + 1));
    if (!tmp) {
	REPORT_N_EXIT(E_MALLOC, "getRequestedField()");
    }

    strcpy(tmp, request);
    head_p = tmp;
    p = strtok(tmp, "|");

    x = p && strcpy(date, p) ? 1 : 0;
    if (!x) {
	free(head_p);
	REPORT_N_RETURN(E_REQUEST_BAD_FORMAT, request, 1);
    }
    p = strtok(NULL, "|");

    x = x && p && strcpy(time, p) ? 1 : 0;
    if (!x) {
	free(head_p);
	REPORT_N_RETURN(E_REQUEST_BAD_FORMAT, request, 1);
    }
    p = strtok(NULL, "|");

    x = x && p && strcpy(lLoc, p) ? 1 : 0;
    if (!x) {
	free(head_p);
	REPORT_N_RETURN(E_REQUEST_BAD_FORMAT, request, 1);
    }
    p = strtok(NULL, "|");

    x = x && p && strcpy(aLoc, p) ? 1 : 0;
    if (!x) {
	free(head_p);
	REPORT_N_RETURN(E_REQUEST_BAD_FORMAT, request, 1);
    }

    if (strtok(NULL, "|")) {
	free(head_p);
	REPORT_N_RETURN(E_REQUEST_BAD_FORMAT, request, 1);
    }
    free(head_p);
    return 0;
}

/**
 * Check formato ora
 * \param time stringa ora
 * \retval 0 formato corretto
 * \reval  1 formato errato
 */
static int checkTime(char *time)
{

    char *hours, *min, t[6];
    int h, m;

    if (!time) {
	REPORT_N_RETURN(E_NULL_POINTER, "checkTime()", 1);
    };

    strcpy(t, time);

    hours = strtok(t, ":");
    min = strtok(NULL, ":");

    if (!(hours && min))
	return 1;

    h = atoi(hours);
    m = atoi(min);


    return isdigit(hours[0]) && h >= 0 && h < 24 && isdigit(hours[1])
	&& !isalpha(hours[0]) && !isalpha(hours[1])
	&& isdigit(min[0]) && m >= 0 && m <= 59 && isdigit(min[1])
	&& !isalpha(min[0]) && !isalpha(min[1]) ? 0 : 1;

}

/**
 * Check formato data
 * \param date stringa data
 * \param t Puntatore a struct tm per il calcolo del giorno della settimana
 * \param wday Usata per memorizzare il giorno della settimana
 * \retval 0 formato corretto
 * \reval  1 formato errato
 */
static int checkDate(char *date, struct tm *t, int *wday)
{
    char *day, *month, *year;
    int d, m, y;

    if (!date || !t) {
	REPORT_N_RETURN(E_NULL_POINTER, "checkDate()", 1);
    }


    day = strtok(date, "/");
    month = strtok(NULL, "/");
    year = strtok(NULL, "/");

    if (!day || !month || !year || strtok(NULL, "/")) {
	REPORT_N_RETURN(E_NULL_POINTER, "checkDate()", 1);
    }


    d = atoi(day);
    m = atoi(month);
    y = atoi(year);

    /*
     * Isdigit and Isalpha are both applied to avoid a mistake in case of a char '00' which it can be  a value 0 returned by isdigit with a not digit param
     * */
    if (!
	(d > 0 && d <= 31 && !isalpha(day[0]) && !isalpha(day[1]) && m > 0
	 && m <= 12 && !isalpha(month[0]) && !isalpha(month[1])
	 && y >= 2009 && !isalpha(year[0]) && !isalpha(year[1])
	 && !isalpha(year[2])
	 && !isalpha(year[3]))) {

	REPORT_N_RETURN(E_NULL_POINTER, "checkDate()", 1);
    }

    t->tm_year = y - 1900;
    t->tm_mon = m - 1;
    t->tm_mday = d;
    t->tm_hour = 0;
    t->tm_min = 0;
    t->tm_sec = 60;
    t->tm_isdst = -1;

    if (mktime(t) > 0) {
	*wday = t->tm_wday + 1;
	return 0;
    } else
	return 1;




}

/**
 * Stampa il percorso
 * 
 * \param path array dei parent
 * \param root puntatore al nodo di partenza
 * \param index indice del nodo di arrivo
 * \param trip_length costo della tratta
 */
static void printRoute(parentNode ** path, location * root, int index,
		       unsigned int trip_length)
{

    if (path[index]->parent) {

	index = path[index]->parent->index;

	printRoute(path, root, index,
		   trip_length + path[index]->pedge->cost);

	if (path[index]->parent) {
	    printf("%s|%s|%s|%s|%s\n", path[index]->s_name,
		   path[index]->parent->name, path[index]->pedge->tLeave,
		   path[index]->pedge->node->name,
		   path[index]->pedge->tArrive);

	    if (path[index]->parent == root);	/*&& starDegree(root->fs) == 1) ; */
	    else
		path[index]->pedge->marked = 1;
	}

    } else {
	printf("%d:%02d\n", trip_length / 60, trip_length % 60);
	return;
    }


}

/**
 * Azzerra i marchi sugli arco
 * \param p array nodi genitori
 * \param index indice nodo di partenza
 */
static void resetMarks(parentNode ** p, int index)
{
    if (!p)
	return;

    if (p[index]->parent) {
	index = p[index]->parent->index;
	resetMarks(p, index);

	if (p[index]->parent)
	    p[index]->pedge->marked = 0;

    } else
	return;

}

/**
 * Avvia la ricerca dei possibili percorsi
 * \param name Nome città di partenza
 * \param arrivo Nome città di arrivo
 * \param wday Giorno della settimana
 * \param root_tleave Ora di partenza
 * \retval 0 Termina correttamente
 * \retval 1 In caso di errori
 */
static int segugio(char *name, char *arrivo, int wday, char *root_tleave)
{

    location *Anode, *Lnode;
    parentNode **path, **p1, **p2, **p3;
    int indice;

    Lnode = findNode(name);
    Anode = findNode(arrivo);

    p1 = NULL;
    p2 = NULL;
    p3 = NULL;

    if (Lnode == Anode)
	return 0;

    if (!Anode) {
	REPORT_N_RETURN(E_LOCATION_NOT_PRESENT, arrivo, 0);
    } else if (!Lnode) {
	REPORT_N_RETURN(E_LOCATION_NOT_PRESENT, arrivo, 0);
    } else if (!Lnode->fs) {
	REPORT_N_RETURN(E_NOPATH, Lnode->name, 0);
    } else {

	path = makeSPT(Lnode, wday, root_tleave);
	if (!path) {
	    enotify("makeSPT()");
	    errid = OK;
	    return 1;
	}

	p1 = path;
	indice = Anode->index;

	if (p1[indice]->parent == NULL) {
	    printf("\nEND\n");
	    return 0;
	}

	printf("Opzione 1|");
	printRoute(p1, Lnode, indice, p1[indice]->pedge->cost);

	printf("%s|%s|%s|%s|%s\n\n", p1[indice]->s_name,
	       p1[indice]->parent->name, p1[indice]->pedge->tLeave,
	       p1[indice]->pedge->node->name, p1[indice]->pedge->tArrive);

	if (p1[indice]->parent == Lnode);	/*&& starDegree(root->fs) == 1) ; */
	else
	    p1[indice]->pedge->marked = 1;


	path = makeSPT(Lnode, wday, root_tleave);
	p2 = path;

	indice = Anode->index;

	if (p2[indice]->parent == NULL) {
	    printf("\nEND\n");
	    RESET_MARKS p1[indice]->pedge->marked = 0;

	    return 0;
	}


	printf("Opzione 2|");
	printRoute(p2, Lnode, indice, p2[indice]->pedge->cost);
	printf("%s|%s|%s|%s|%s\n\n", p2[indice]->s_name,
	       p2[indice]->parent->name, p2[indice]->pedge->tLeave,
	       p2[indice]->pedge->node->name, p2[indice]->pedge->tArrive);
	if (p2[indice]->parent == Lnode);	/*&& starDegree(root->fs) == 1) ; */
	else
	    p2[indice]->pedge->marked = 1;


	path = makeSPT(Lnode, wday, root_tleave);
	p3 = path;

	indice = Anode->index;

	if (p3[indice]->parent == NULL) {
	    printf("\nEND\n");
	    RESET_MARKS p1[indice]->pedge->marked = 0;
	    p2[indice]->pedge->marked = 0;
	    return 0;
	}

	printf("Opzione 3|");
	printRoute(p3, Lnode, indice, p3[indice]->pedge->cost);

	printf("%s|%s|%s|%s|%s\n\nEND\n\n", p3[indice]->s_name,
	       p3[indice]->parent->name, p3[indice]->pedge->tLeave,
	       p3[indice]->pedge->node->name, p3[indice]->pedge->tArrive);

	if (p3[indice]->parent == Lnode);	/*&& starDegree(root->fs) == 1) ; */
	else
	    p3[indice]->pedge->marked = 1;

	/*
	 * Reset all path nodes mark
	 */
	RESET_MARKS p1[indice]->pedge->marked = 0;
	p2[indice]->pedge->marked = 0;
	p3[indice]->pedge->marked = 0;


	CLEAN_PATHS(p1, p2, p3);
	return 0;
    }
}

/**
 * Legge una richiesta da stdin ed esegue la ricerca
 * \retval 1 In caso di errore
 */
int acceptRequests(void)
{

    char request[MAX_RECORD_SIZE + 1];
    char date[MAX_RECORD_SIZE + 1];
    char time[TIME_SIZE + 1];
    char leavingFrom[STATION_SIZE + 1];
    char arrivingTo[STATION_SIZE + 1];
    int weekday, a;
    struct tm t;

    a = 0;

    while (1) {

	fflush(stdin);
	strcpy(request, "");

	if (!feof(stdin)) {
	    fflush(stdin);
	}


	if (!fgets(request, MAX_RECORD_SIZE, stdin))
	    continue;
	request[strlen(request) - 1] = '\0';

	/*      Estraggo le informazioni        */
	if (getRequestField(request, date, time, leavingFrom, arrivingTo)) {
	    if (errid == E_MALLOC) {
		enotify("agetRequestField()");
		exit(EXIT_FAILURE);
	    }
	    if (errid == E_REQUEST_BAD_FORMAT) {
		enotify(request);
		errid = OK;
	    }

	    continue;
	}

	/* Controlli di formato */
	if (checkDate(date, &t, &weekday))
	    continue;


	if (checkTime(time)) {
	    errid = E_REQUEST_BAD_FORMAT;
	    enotify(request);
	    errid = OK;
	    continue;
	}

	if (findNode(leavingFrom) && findNode(leavingFrom)) {
	    if (segugio(leavingFrom, arrivingTo, weekday, time)) {
		return 1;
	    }
	} else {
	    errid = E_LOCATION_NOT_PRESENT;
	    enotify(request);
	    errid = OK;
	};

    }

    return 1;
}
