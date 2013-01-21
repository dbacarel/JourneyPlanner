#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "gtype.h"
#include "set.h"
#include "errors.h"

#define TRACE printf("\nRiga %d: File :%s ---\n\n",__LINE__,__FILE__);

/**
 * Somma le lettere di una stringa
 * \param name La stringa
 * \return La somma delle lettere
 */
static int hash(char *name)
{

    int i, hash;


    for (i = 0, hash = 0; name[i]; i++)
	hash += name[i];
    return hash % HASH_ARRAY_SIZE;


}

/**
 * Elimina un insieme
 * \param set L'insieme da eliminare
 */
void set_delete(location ** set)
{
    location *p;

    if (is_empty(set))
	free(set);
    else {
	while (!is_empty(set)) {
	    p = set_get(set);
	    set_remove(set, p);
	}
	free(set);
    }

}

/**
 * Genera un insieme
 * \return L'insieme appena creato
 */
location **get_set(void)
{

    location **set;
    int i;
    set = (location **) calloc(HASH_ARRAY_SIZE, sizeof(location *));
    if (errid != OK) {
	errid = E_MALLOC;
	return NULL;
    }
    for (i = 0; i < HASH_ARRAY_SIZE; i++)
	set[i] = NULL;
    return set;
}

/**
 * Controlla se un elemento è presente nell'insieme
 * \param set L'insieme nel quale cercare
 * \param el L'elemento da inserire
 * \retval 1 Se è stato trovato
 * \retval 0 Altrimenti
 */
int is_in(location ** set, location * el)
{
    location *t;
    int i;
    t = set[hash(el->name)];
    for (i = 0; t; t = t->next)
	if (!strcmp(el->name, t->name))
	    return 1;

    return 0;

}

/**
 * Elimina l'elemento dall'insieme
 * \param set L'insieme
 * \param L'elmento da eliminare
 * \return 0 Se eliminato
 */
int set_remove(location ** set, location * el)
{

    location *t, *b;
    int i;

    if (is_empty(set))
	return 0;
    else if (!is_in(set, el))
	return 0;

    t = set[hash(el->name)];
    /*      L'elemento è in testa? */
    if (!strcmp(t->name, el->name)) {
	b = t;
	set[hash(el->name)] = t->next;
	free(b);
	return 0;
    } else {
	for (i = 0; t && t->next && strcmp(el->name, t->next->name);
	     i++, t = t->next);
	b = t->next;
	t->next = t->next->next;
	free(b);

	return 0;
    }
}

/**
 * Inserisce un elemento nell'insieme
 * \param set L'insieme
 * \param el L'elemento da inserire
 * \retval 0 Se inserito correttamente
 * \retval 1 In caso di errore
 */
int set_insert(location ** set, location * el)
{
    location *t, *a;
    if (is_in(set, el))
	return 0;

    t = (location *) calloc(1, sizeof(location));
    if (!t) {
	errid = E_MALLOC;
	return 1;
    }
    memcpy(t, el, sizeof(location));

    t->next = NULL;

    a = set[hash(t->name)];
    if (!a)
	set[hash(t->name)] = t;
    else {
	while (a->next)
	    a = a->next;
	a->next = t;
    }

    return 0;

}

/**
 * Controlla se l'insieme è vuoto
 * \param set L'insieme
 * \retval 0 Se non è vuoto
 * \retval 1 Altrimenti
 */
int is_empty(location ** set)
{

    int i;
    for (i = 0; i < HASH_ARRAY_SIZE; i++)
	if (set[i])
	    return 0;

    return 1;

}

/**
 * Estrae un elemento dall'insieme
 * \param set L'insieme
 * \return L'elemento estratto
 */
location *set_get(location ** set)
{
    int rand_index;

    rand_index = rand() % HASH_ARRAY_SIZE;

    while (!set[rand_index])
	rand_index = rand() % HASH_ARRAY_SIZE;

    return Alocations[set[rand_index]->index];



}
