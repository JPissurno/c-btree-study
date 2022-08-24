#include "btree.h"


struct BTree* btree_create(void){

	struct BTree* btree = calloc(1, sizeof(struct BTree));

	btree->isLeaf = 1;
	btree->dd = btree;
	btree->key = calloc(TREEP+1, sizeof(struct Book));
	btree->child = calloc(TREEP+2, sizeof(struct BTree*));

	return btree;
	
}


void btree_insert(struct BTree* node, struct Book* key){

	if(node->isLeaf){

		node->key[node->n] = *key;
		qsort(node->key, node->n + 1, sizeof(node->key[0]), comp);

		if(node->n >= TREEP)
			split(node);
		else
			node->n++;

		return;

	}

	for(int i = 0; i < node->n; i++){

		if(key->code <= node->key[i].code){

			btree_insert(node->child[i], key);
			return;

		}

	}

	btree_insert(node->child[node->n], key);

}


struct BTree* btree_search(struct BTree* node, int key, int* idx){

	if(!node || key < 0)
		return NULL;

	(*idx) = 0;
	for(int i = 0; i < node->n; i++, (*idx) = i){

		if(key == node->key[i].code)
			return node;
		if(key < node->key[i].code)
			return btree_search(node->child[i], key, idx);
		if(i + 1 >= node->n)
			return btree_search(node->child[i + 1], key, idx);

	}

}


static void split(struct BTree* node){

	int mid = TREEP/2, mid2 = (TREEP+2)/2;

	/* Root */
	if(node->dd == node){

		struct BTree* n1 = btree_create();
		struct BTree* n2 = btree_create();

		/* Transfer of keys */
		for(int i = 0; i < mid; i++)
			n1->key[i] = node->key[i];
		for(int i = 0, j = mid+1; j <= TREEP; i++, j++)
			n2->key[i] = node->key[j];

		node->key[0] = node->key[mid];
		memset(node->key+1, 0, TREEP*sizeof(node->key[0]));

		/* Transfer of pointers */
		for(int i = 0; i < mid2; i++){

			if(node->child[i])
				node->child[i]->dd = n1;

			n1->child[i] = node->child[i];

		}

		for(int i = 0, j = mid2; j <= TREEP + 1; i++, j++){

			if(node->child[j])
				node->child[j]->dd = n2;

			n2->child[i] = node->child[j];

		}

		n1->isLeaf = n1->child[0] ? 0 : 1;
		n2->isLeaf = n2->child[0] ? 0 : 1;
		memset(node->child, 0, (TREEP + 2) * sizeof(node->child[0]));
		node->child[0] = n1;
		node->child[1] = n2;

		/* Setting up new nodes & root */
		n1->n = mid;
		n2->n = mid + TREEP%2;
		n1->dd = node;
		n2->dd = node;
		node->n = 1;
		node->isLeaf = 0;

		return;
		
	}

	/* Mid & leaf nodes */
	struct BTree* dad = node->dd;
	dad->key[dad->n] = node->key[mid];
	qsort(dad->key, dad->n+1, sizeof(dad->key[0]), comp);

	for(int i = 0; i <= dad->n; i++){
		if(dad->child[i] == node){

			/* Pointers right shift */
			for(int j = dad->n + 1; j > i; j--)
				dad->child[j] = dad->child[j - 1];
			
			/* Setting up new node & updating the one splitting */
			node->n = mid;
			dad->child[++i] = calloc(1, sizeof(dad->child[0]));
			dad->child[i]->key = calloc(TREEP+1, sizeof(dad->key[0]));
			dad->child[i]->n = mid + TREEP % 2;
			dad->child[i]->dd = dad;

			/* Transfer of keys */
			for(int j = 0, k = mid + 1; k <= TREEP; j++, k++)
				dad->child[i]->key[j] = node->key[k];
			memset(node->key+mid, 0, (1+mid+TREEP%2)*sizeof(node->key[0]));

			/* Transfer of pointers */
			dad->child[i]->child = calloc(TREEP+2, sizeof(dad->child[0]));
			for(int j = 0, k = mid2; k <= TREEP + 1; j++, k++)
				dad->child[i]->child[j] = node->child[k];

			dad->child[i]->isLeaf = dad->child[i]->child[0] ? 0 : 1;
			memset(node->child + mid2, 0, (mid2 + TREEP % 2) * sizeof(node->child[0]));

			break;
			
		}
	}

	if(dad->n++ >= TREEP)
		split(dad);
	
}


struct Book* btree_traverse(struct BTree* node, int* depth){

	if(!(*depth) || !node)
		return NULL;

	if((*depth) <= node->n){
		
		int tmp = (*depth) - 1;
		
		(*depth) -= node->n;
		if((*depth) < 0)
			(*depth) = 0;
		
		return &node->key[tmp];
		
	}
	
	(*depth) -= node->n;
	if((*depth) < 0)
		(*depth) = 0;
	
	struct Book *book;
	for(long i = 0; i <= node->n; i++){
		
		if(*depth)
			book = btree_traverse(node->child[i], depth);
		
		if(!(*depth))
			return book;
		
	}

	return NULL;
	
}


void btree_free(struct BTree* node){

	if(!node)
		return;

	if(node->isLeaf)
		return free(node);

	for(int i = 0; i <= TREEP + 1; i++)
		btree_free(node->child[i]);

	free(node);
	
}


static int comp(const void* a, const void* b){

	const struct Book* x = a;
	const struct Book* y = b;

	return (x->code < y->code) ? -1 : !(x->code == y->code);
	
}