#include "library.h"


struct Library* library_start(char* dbFileName){
	
	struct Book book;
	struct Library* lib = calloc(1, sizeof(struct Library));
	
	lib->db = btree_create();
	lib->dbFileName = dbFileName;
	lib->dbFileHandler = fopen(dbFileName, "r");
	
	if(!lib->dbFileHandler){
		
		if(!(lib->dbFileHandler = fopen(dbFileName, "w")))
			return NULL;
		
		/* If there's no local database ready */
		return lib;
		
	}
	
	/* Reads local database indicated by dbFileName */
	for(char ret; (ret = fscanf(lib->dbFileHandler, "%d %d %c %[^\n]",
					&book.code, &book.quantity,
					&book.isAvailable, book.title
		)) != EOF;){

		if(ret == 4)
			btree_insert(lib->db, &book);

	}

	/* Success */
	return lib;
	
}


char library_registerBook(struct Library* lib, struct Book* book){

	if(book->isAvailable == -1)
		return 1;
	
	if(!(lib->dbFileHandler = freopen(NULL, "a", lib->dbFileHandler)))
		return 1;
	
	/* If book already registered */
	if(library_searchBook(lib, book->code).isAvailable != -2)
		return 2;
	
	/* Registers book in local database */
	btree_insert(lib->db, book);
	fprintf(lib->dbFileHandler, "%d %d %d %s\n",
								book->code, book->quantity,
								book->isAvailable, book->title);

	return 0;
	
}


//char library_removeBook(struct Library* lib, int code)


struct Book library_searchBook(struct Library* lib, int code){

	int idx;
	struct Book book;
	
	if(!code){
		book.isAvailable = -1;
		return book;
	} else if (code < 0){
		book.isAvailable = -3;
		return book;
	}

	struct BTree *node = btree_search(lib->db, code, &idx);
	
	/* Book not found */
	if(!node){
		book.isAvailable = -2;
		return book;
	}
	
	/* Book found */
	return node->key[idx];
	
}


char library_updateBook(struct Library* lib, struct Book* book){
	
	if(book->isAvailable == -1) return 1;
	
	int idx;
	struct BTree *node = btree_search(lib->db, book->code, &idx);
	
	if(!node) return 2;
	
	/* Returns error if it results in a negative quantity */
	if((node->key[idx].quantity + book->quantity) < 0)
		return 3;
	
	node->key[idx].quantity += book->quantity;
	node->key[idx].isAvailable = node->key[idx].quantity ? 1 : 0;

	updateDatabase(lib, &node->key[idx], 0);
	
	/* Success */
	return 0;
	
}


void library_free(struct Library* lib){
	
	/* Cleanup */
	btree_free(lib->db);
	fclose(lib->dbFileHandler);
	free(lib);
	
}


static char updateDatabase(struct Library* lib, struct Book* book, char type){
	
	if(!(lib->dbFileHandler = freopen(NULL, "r", lib->dbFileHandler)))
		return 1;
	
	char buffer[INPUT_MAX];
	FILE *tmpDB = fopen("_db_.dat", "w");
	
	if(!tmpDB) return 1;
	while(fgets(buffer, sizeof(buffer), lib->dbFileHandler)){

		/* Writes updated book to tmpDB if code matches, else
		   rewrites what was there initially */
		if(strtol(strtok(buffer, " "), NULL, 10) == book->code)
			fprintf(tmpDB, "%d %d %d %s\n",
							book->code, book->quantity,
							book->isAvailable, book->title);
		else {
			*strchr(buffer, '\0') = ' ';
			fprintf(tmpDB, "%s", buffer);
		}
		
	}

	fflush(tmpDB);
	if(!freopen(NULL, "r", tmpDB)){
		remove("_db_.dat");
		return 1;
	}

	if(!freopen(NULL, "w", lib->dbFileHandler)){
		remove("_db_.dat");
		return 1;
	}

	while(fgets(buffer, sizeof(buffer), tmpDB))
		fprintf(lib->dbFileHandler, "%s", buffer);

	fclose(tmpDB);
	remove("_db_.dat");
	fflush(lib->dbFileHandler);

	return 0;

}