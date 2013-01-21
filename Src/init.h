#ifndef _INIT_H
#define _INIT_H

#include "gtype.h"



int init(void);
int removeService(service** slist);
int fillDS(FILE* fd);
int getServiceRoute(FILE * fd,service *servi);
location *  addNode(location *);
int addArco(edge *newedge);
service * addService(char[],char[]);
edge * createArco(edge * larchi,char partenza[],char arrivo[],location *next);
location * findNode(char *loc_name);
int getIndexByHash(char *loc_name);
service * putService(service *list,service *n_serv);

int loc_array_index;/* Contiene il numero di nodi inseriti */
int loc_array_size;/* Contiene la dimensione dell'array Alocations	*/
#endif /* _INIT_H */
