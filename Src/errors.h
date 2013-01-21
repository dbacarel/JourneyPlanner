#ifndef ERRORS_H_
#define ERRORS_H_

/* No errors */
#define OK 1000

 /*	Errors MACRO	*/

#define E_FILE_NOT_FOUND 1001	
#define E_NULL_FILE_POINTER 1002	
#define E_MALLOC	1003	
#define E_CFILE_BAD_FORMAT 1004
#define E_NULL_POINTER 1005
#define E_REQUEST_BAD_FORMAT 1006
#define E_LOCATION_NOT_PRESENT 1007
#define E_NOPATH 1008


#define REPORT_N_EXIT(error,descr)\
		errid=error;\
		enotify(descr);\
		exit(EXIT_FAILURE);

#define REPORT_N_RETURN(error,descr,r_value)\
		errid=error;\
		enotify(descr);\
		errid=OK;\
		return r_value;
		
		
int errid;
void enotify(char *err_description);


#endif /* ERRORS_H*/
