#include <stdio.h>
#include "errors.h"

/**
 * Stampa un messaggio di errore su stderr
 * \param err_description Una descrizione dell'errore
 */
void enotify(char *err_description)
{

    switch (errid) {

    case OK:
	fprintf(stderr, "SUCCESS: %s\n", err_description);
	break;

    case E_NOPATH:
	fprintf(stderr, "%d Any path from %s has been found\n", E_NOPATH,
		err_description);
	break;

    case E_FILE_NOT_FOUND:
	fprintf(stderr, "%d File not found: %s\n", E_FILE_NOT_FOUND,
		err_description);
	break;

    case E_NULL_FILE_POINTER:
	fprintf(stderr, "%d Null file pointer: %s\n",
		E_NULL_FILE_POINTER, err_description);
	break;

    case E_MALLOC:
	fprintf(stderr, "ABORT %d out of memory: %s\n", E_MALLOC,
		err_description);
	break;

    case E_CFILE_BAD_FORMAT:
	fprintf(stderr,
		"%d Configuration file with bad format: %s\n",
		E_CFILE_BAD_FORMAT, err_description);
	break;

    case E_NULL_POINTER:
	fprintf(stderr,
		"%d Null pointer: %s\n", E_NULL_POINTER, err_description);
	break;

    case E_REQUEST_BAD_FORMAT:
	fprintf(stderr, "%d Bad format request: %s\n",
		E_REQUEST_BAD_FORMAT, err_description);
	break;

    case E_LOCATION_NOT_PRESENT:
	fprintf(stderr,
		"%d One or more locations specified in the request haven't been loaded: %s\n",
		E_LOCATION_NOT_PRESENT, err_description);
	break;


    }
}
