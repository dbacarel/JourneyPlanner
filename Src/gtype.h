#ifndef _GTYPE_H
#define _GTYPE_H

#define HASH_ARRAY_SIZE 100
#define MAX_RECORD_SIZE 100
#define TIME_SIZE 5
#define SERVICE_SIZE 50
#define STATION_SIZE 50
#define DATE_SIZE 10
#define LOC_ARR_SIZE 100
#define SERVICE_DAYS_SIZE 7




 struct tService{
	 
	struct tService * lservice;
	struct tService * next;
	unsigned int working_days;
	char name[SERVICE_SIZE+1];
	
};


struct tEdge{
	

    struct tLocation *node;
	struct tEdge *next;
	char tLeave[TIME_SIZE+1];
	char tArrive[TIME_SIZE+1];
	char marked;
	unsigned int cost;
	
}; 

 struct tLocation{
	struct tService *lservice;
	char  name[STATION_SIZE+1];
	struct tEdge * fs;
	struct tLocation *next;
	int index;
	
}; 

struct tParentNode{
	
	struct tLocation *parent;
	struct tEdge *pedge;
	char tArrive[TIME_SIZE+1];
	char s_name[SERVICE_SIZE+1];

	};
	
	struct tCosti{
	unsigned int time_cost;
	unsigned int accuracy;
	};


typedef struct tEdge edge;
typedef struct tLocation location;
typedef struct tService service;
typedef struct tParentNode parentNode;
typedef struct tCosti costi;

extern location * locations[HASH_ARRAY_SIZE];
extern service  * services;
extern location	** Alocations;

#endif /*_GTYPE_H*/
