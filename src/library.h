#ifdef __BTREE__

/* */
#	define INPUT_MAX 1024

	/* */
	struct Book {
		
		int code;
		int quantity;
		char isAvailable;
		char title[INPUT_MAX];
		
	};
	
	/* */
	struct Library {
		
		struct BTree* db;
		char* dbFileName;
		FILE* dbFileHandler;
		
	};

#endif

#ifndef __LIBRARY__
#define __LIBRARY__


#include "btree.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* */
#define DB_FILE_NAME "db.dat"



/* */
char menu_main(void);


/* */
struct Book menu_insert(void);


/* */
//int menu_remove(void);


/* */
int menu_search(void);


/* */
struct Book menu_update(char type);


/* */
int menu_list(struct Library* lib);


/* */
void menu_clrscr(void);


/* */
static int safeInput(char* str);



/* */
struct Library* library_start(char* dbFileName);

/* */
char library_registerBook(struct Library* lib, struct Book* book);

/* */
//char library_removeBook(struct Library* lib, int code);

/* */
struct Book library_searchBook(struct Library* lib, int code);

/* */
char library_updateBook(struct Library* lib, struct Book* book);

/* */
void library_free(struct Library* lib);

/* */
static char updateDatabase(struct Library* lib, struct Book* book, char type);



#endif