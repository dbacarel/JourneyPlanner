#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "init.h"
#include "search.h"
#include "set.h"
#include "errors.h"





int main(int argc, char **argv)
{
    int i;

    FILE *fd;
    errid = OK;
    init();

    if (errid != OK) {
	enotify("init()");
	return EXIT_FAILURE;
    }

    for (i = 1; i < argc; i++) {
	if (!(fd = fopen(argv[i], "r"))) {
	    if (errno == ENOENT) {
		errid = E_FILE_NOT_FOUND;
		enotify(argv[i]);
		return EXIT_FAILURE;
	    }
	}

	/*
	 * Raccolgo i dati dal file di configurazione
	 */
	fillDS(fd);
	if (errid == E_NULL_FILE_POINTER) {
	    enotify("fillDS()");
	    exit(EXIT_FAILURE);
	} else if (errid == E_CFILE_BAD_FORMAT) {
	    enotify(argv[i]);
	    errid = OK;
	    /* Ignoro il file con formato errato e vado avanti      */
	}

	fclose(fd);
    }

    /*      Se non è stata letta nessuna città termino il programma       */
    if (!loc_array_index)
	return EXIT_FAILURE;

    /*
     * I dati sono stati caricati in memoria, mi metto in attesa di richieste
     */

    acceptRequests();
    return EXIT_SUCCESS;
}
