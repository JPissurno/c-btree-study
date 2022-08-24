#ifndef __BTREE__
#define __BTREE__


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "library.h"


/* */
int TREEP;
//#define TREEP 4



/**	@Description
 *		S
 *
 *	@Members
 *		int socketFD:				holds a file descriptor
 *									returned by socket() function;
 *
 *		struct sockaddr_in addr:	structure that holds
 *									connection information
 *
 */
struct BTree {

	char n;
	char isLeaf;
	struct BTree* dd;
	struct BTree** child;
	struct Book* key;
	//struct BTree* child[TREEP + 2];
	//struct Book key[TREEP + 1];
	
};



/* */
struct BTree* btree_create(void);


/* */
void btree_insert(struct BTree* node, struct Book* key);


/* */
//char btree_remove(struct BTree* node, int key);


/* */
struct BTree* btree_search(struct BTree* node, int key, int* i);


/* */
struct Book* btree_traverse(struct BTree* node, int* depth);


/** @Functionality
 *		Sends bytes to host, along with PX_SEND_FINISH if protocol is 0.
 *
 *		Requires an opened socket and host's information,
 *		on a socket_con structure, all given by px_setupExtConnection() function.
 *
 *	@Arguments
 *		struct socket_con* con:	pointer to a socket_con structure
 *								with an opened socket and host information;
 *
 *		void* msg:				pointer to the bytes to send;
 *
 *		long len: 				how many bytes of msg to send;
 *
 *		int protocol:			0 	= for client<->proxy;
 *		          				1 	= for internet, general;
 *		          				80 	= for internet, http header.
 *
 *	@Returns
 *		On success: 0;
 *
 *		On failure: 1 	(if connection is closed when protocol is 0, or
 *						 if any error occurs while sending the bytes)
 *
 */
void btree_free(struct BTree* node);


/* */
static void split(struct BTree* node);


/* */
static int comp(const void* a, const void* b);



#endif