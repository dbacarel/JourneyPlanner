#include "gtype.h"

#ifndef SET_H_
#define SET_H_

location ** get_set(void);

int is_in(location **set,location *el);

location * set_get(location **set);

int set_remove(location **a,location *b);

int set_insert(location **set,location *el);

int is_empty(location **set);

void set_delete(location ** set);

#endif /* SET_H_ */
