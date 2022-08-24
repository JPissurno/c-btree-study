#include <stdio.h>
#include <string.h>
#include "library.h"


extern int TREEP;
enum{SUCC, ERR_FIND = 2, ERR_EMPTY};
enum{EXIT, INS, RMV, SRC, INC, DEC, CTL, HLP};


void help(void);
void print_info(struct Book* book);
int comp(const void* a, const void* b);
char get_arguments(int argc, char** argv);
void mode_interactive(struct Library* catalog);
char report(struct Library* lib, FILE* outputFile);
char mode_files(int argc, char** argv, struct Library* catalog);


int main(int argc, char** argv){
	
	TREEP = 4;
	char ret;
	struct Library *catalog = library_start(DB_FILE_NAME);

	if(!catalog) return 1;
	
	if(!(ret = get_arguments(argc, argv)))
		mode_interactive(catalog);
	else if(ret == 1){
		
		if((ret = mode_files(argc, argv, catalog)) == 1)
			printf("Argumentos inválidos. Execute com -h para exemplos de uso.\n");
		else if(ret == 2)
			printf("Arquivo de entrada com um padrão inválido. Execute com -h para exemplos de uso.\n");
		
	} else help();
	
	return ret;
	
}


char get_arguments(int argc, char** argv){
	
	if(argc <= 1) return 0;						//Interactive interface
	if(!strncmp(argv[1], "-f", 2)) return 1;	//Files interface
	if(!strncmp(argv[1], "-o", 2)) return 1;	//Files interface
	if(!strncmp(argv[1], "-h", 2)) return 2;	//Help operation
	
	/* Interactive interface by default */
	return 0;
	
}


void mode_interactive(struct Library* catalog){
	
	char ret;
	struct Book book;
	
	menu_clrscr();
	while(1){
		switch(menu_main()){

			case INS:
				book = menu_insert();
				if(!(ret = library_registerBook(catalog, &book)))
					printf("||Inserção realizada com sucesso\n");
				else if(ret == ERR_FIND)
					printf("||Já existe um livro registrado com o código escolhido\n");
				break;

			case RMV:
				/*
				if(!library_removeBook(catalog, menu_remove()))
					printf("||Remoção realizada com sucesso\n");
				*/
				break;

			case SRC:
				if((book = library_searchBook(catalog, menu_search())).isAvailable >= SUCC)
					print_info(&book);
				else if(book.isAvailable == ERR_FIND*-1)
					printf("||Livro de código \"%d\" não registrado no sistema\n", book.code);
				break;

			case INC:
				book = menu_update(1);
				if(!(ret = library_updateBook(catalog, &book)))
					printf("||Atualização realizada com sucesso\n");
				else if(ret == ERR_FIND)
					printf("||O código indicado não existe\n");
				break;
				
			case DEC:
				book = menu_update(0);
				if(!(ret = library_updateBook(catalog, &book)))
					printf("||Atualização realizada com sucesso\n");
				else if(ret == ERR_FIND)
					printf("||O código indicado não existe\n");
				else
					printf("||A quantidade indicada para decremento\n\
							||é superior à quantidade de livros disponível\n");
				break;

			case CTL:
				if((book = library_searchBook(catalog, menu_list(catalog))).isAvailable >= SUCC)
					print_info(&book);
				else if(book.isAvailable == ERR_EMPTY*-1)
					printf("||Não há livros registrados no sistema\n");
				break;

			case HLP:
				help();
				break;

			case EXIT:
				menu_clrscr();
				printf("Fechando... obrigado por visitar a livraria\n");
				return;

		}

	}
	
}


char mode_files(int argc, char** argv, struct Library* catalog){
	
	if(argc < 5) return 1;
	
	struct Book book;
	FILE *inputFile, *outputFile;
	int incount = 0, outcount = 0;
	char buffer[INPUT_MAX], *substring;
	char infirst = !strncmp(argv[1], "-f", 2) ? 1 : 0; // 1 -> input first | 0 -> output first
	
	for(int i = 2, *aux = infirst ? &incount : &outcount; i < argc; i++){

		if(!strncmp(argv[i], infirst ? "-o" : "-f", 2)){
			aux = infirst ? &outcount : &incount;
			continue;
		}
		(*aux)++;

	}
	if(outcount != incount) return 1;
	
	for(int i = 2; i < argc; i++){
		
		/* Second argument index */
		if(i == (infirst ? incount : outcount)+2)
			break;
		
		/* Returns error if it fails to open indicated files */
		if(!(inputFile = fopen(argv[infirst ? i : i+incount+1], "r")))
			return 1;
		if(!(outputFile = fopen(argv[infirst ? i+incount+1 : i], "w"))){
			fclose(inputFile);
			return -1;
		}
		
		/* Iterates through inputFile's lines
		 * according to the following protocol:
		 * 
		 * 1st line only: number of keys and children per node
		 * remainder lines: op_code [book_code|book_quantity] [book_code|book_title]
		 *
		 * op_code shall be a number from 1 to 6, meaning, respectively:
		 *	- insert operation, followed by a book code and the book title;
		 *	- remove operation, followed by a book code;
		 *	- search operation, followed by a book code;
		 *	- increment operation, followed by some quantity and a book code;
		 *	- decrement operation, followed by some quantity and a book code;
		 *	- catalog operation
		 *
		 * book_code and book_quantity must be
		 * a number within the architecture's int range;
		 *
		 * book_title must not be greater than
		 * INPUT_MAX-25 macro, doing so leads to UB;
		 *
		 * If that standard is not followed, it
		 * stops right then and returns 2;
		 *
		 * If there's any error processing some
		 * operation, its op_code is written in
		 * the indicated outputFile with a negative
		 * sign, and the loop continues.
		 *
		 */
		substring = catalog->dbFileName;
		library_free(catalog);
		if(!fgets(buffer, sizeof(buffer), inputFile)) goto ERROR;
		if((TREEP = strtol(buffer, NULL, 10)) <= 1){
			TREEP = 4;
			goto ERROR;
		}
		catalog = library_start(substring);
		while(fgets(buffer, sizeof(buffer), inputFile)){
			
			/* Breaks down a line from inputFile by
			   a separator (whitespace), following
			   above protocol. */
			if(!(substring = strtok(buffer, " ")))
				goto ERROR;

			memset(&book, 0, sizeof(struct Book));
			switch(substring[0]-'0'){
				case INS:
					if(!(substring = strtok(NULL, " "))) goto ERROR;
					if((book.code = strtol(substring, NULL, 10)) <= 0) goto ERROR;
					if(!(substring = strtok(NULL, "\n"))) goto ERROR;
					strncpy(book.title, substring, sizeof(book.title));
					if(library_registerBook(catalog, &book))
						fprintf(outputFile, "-1\n");
					break;
					
				case RMV:
					/*if(library_removeBook(catalog, strtol(strtok(NULL, " "), NULL, 10)))
						fprintf(outputFile, "-2\n");*/
					break;
					
				case SRC:
					if(!(substring = strtok(NULL, " "))) goto ERROR;
					if((book.code = strtol(substring, NULL, 10)) <= 0) goto ERROR;
					if((book = library_searchBook(catalog, book.code)).isAvailable >= SUCC)
						fprintf(outputFile, "%d %d %s\n", book.code, book.quantity, book.title);
					else
						fprintf(outputFile, "-3\n");
					break;
					
				case INC:
					if(!(substring = strtok(NULL, " "))) goto ERROR;
					if((book.quantity = strtol(substring, NULL, 10)) <= 0) goto ERROR;
					if(!(substring = strtok(NULL, " "))) goto ERROR;
					if((book.code = strtol(substring, NULL, 10)) <= 0) goto ERROR;
					if(library_updateBook(catalog, &book))
						fprintf(outputFile, "-4\n");
					break;
					
				case DEC:
					if(!(substring = strtok(NULL, " "))) goto ERROR;
					if((book.quantity = strtol(substring, NULL, 10)) <= 0) goto ERROR;
					if(!(substring = strtok(NULL, " "))) goto ERROR;
					if((book.code = strtol(substring, NULL, 10)) <= 0) goto ERROR;
					book.quantity *= -1;
					if(library_updateBook(catalog, &book))
						fprintf(outputFile, "-5\n");
					break;
					
				case CTL:
					if(report(catalog, outputFile))
						fprintf(outputFile, "-6\n");
					break;
				
				default:
					ERROR:
						goto BREAKALL;

			}
			
		}
		
		/* Cleanup */
		BREAKALL:;
		fclose(inputFile);
		fclose(outputFile);
		btree_free(catalog->db);
		catalog->db = btree_create();
		
	}

	/* Success, but some files may be skipped
	   if out of protocol */
	return 0;
	
}


char report(struct Library* lib, FILE* outputFile){
	
	int qtd = 0, tot = 0, idx = 1;
	struct Book *book = calloc(1, sizeof(struct Book)), *bookaux;
	
	/* Traverses the BTree, retrieving one book at a time */
	while(bookaux = btree_traverse(lib->db, &idx)){
		
		qtd++;
		
		book = realloc(book, (qtd+1)*sizeof(struct Book));
		book[qtd-1].code = bookaux->code;
		book[qtd-1].quantity = bookaux->quantity;
		strcpy(book[qtd-1].title, bookaux->title);
		
		idx = qtd+1;
		tot += bookaux->quantity;
		
	}
	qsort(book, qtd, sizeof(book[0]), comp);
	
	fprintf(outputFile, "%d\n", qtd); //How many different books there are
	fprintf(outputFile, "%d\n", tot); //How many books there are
	/* Prints all books' information */
	for(int i = 0; i < qtd; i++)
		fprintf(outputFile, "%d %d %s\n", book[i].code, book[i].quantity, book[i].title);

	free(book);

	/* 0 if success, 1 if no book registered */
	return qtd ? 0 : 1;
	
}


void print_info(struct Book* book){
	
	printf("================================================\n");
	printf("||Informações do livro de código \"%d\":\n||\n", book->code);
	printf("||\tTítulo: %s\n", book->title);
	printf("||\tQuantidade: %d\n", book->quantity);
	printf("||\tStatus: %s%s\n", book->isAvailable ? "" : "in", "disponível");
	printf("||\n");
	printf("================================================\n");
	
}


int comp(const void* a, const void* b){

	const struct Book* x = a;
	const struct Book* y = b;

	return (x->code < y->code) ? -1 : !(x->code == y->code);
	
}


void help(void){
	
	printf("Gerenciador de uma biblioteca virtual, utilizando\n");
	printf("Árvore B como sua estrutura de dados. O programa tem\n");
	printf("seu banco de dados como um arquivo textual local, além\n");
	printf("de outro arquivo usado para representar simbolicamente\n");
	printf("o estado da Árvore B interno do programa\n\n");
	
	printf("Argumentos:\n");
	printf("\t-h: este manual\n");
	printf("\t-i: interface interativa; opção padrão se não houverem argurmentos ou forem inválidos\n");
	printf("\t-f: arquivo_de_entrada1 [arquivo_de_entrada2] [...]: interface de arquivos; usado em conjunto com -o\n");
	printf("\t-o: arquivo_de_saida1 [arquivo_de_saida2] [...]: interface de arquivos; usado em conjunto com -f\n\n");
	
	printf("Interface interativa:\n");
	printf("\tAs seguintes operações estão disponíveis\n");
	printf("\tpor meio de menus interativos:\n");
	printf("\t\t- cadastrar livro\n");
	printf("\t\t- remover livro\n");
	printf("\t\t- procurar livro\n");
	printf("\t\t- incrementar quantidade de um livro\n");
	printf("\t\t- decrementar quantidade de um livro\n");
	printf("\t\t- imprimir catálogo e informações\n");
	printf("\t\t- ajuda (este manual)\n\n");
	
	printf("Interface de arquivos:\n");
	printf("\tAs operações podem ser indicadas num arquivo de entrada\n");
	printf("\tseguindo o seguinte padrão:\n");
	printf("\top_code [book_code|book_quantity] [book_code|book_title]\n\n");
	printf("\tOnde \"op_code\" deve ser um número de 1 a 6 inclusos,\n");
	printf("\tindicando, respectivamente:\n");
	printf("\t\t- cadastro de livro, seguido de um código para o mesmo e seu título;\n");
	printf("\t\t- remoção de livro, seguido de um código para o mesmo;\n");
	printf("\t\t- procura de livro, seguido de um código para o mesmo;\n");
	printf("\t\t- entrada de livro, seguido de uma quantidade para incremento e o código de um livro\n");
	printf("\t\t- saída de livro, seguido de uma quantidade para decremento e o código de um livro\n");
	printf("\t\t- catálogo\n\n");
	printf("\tO código do livro e a quantidade a se incrementar/decrementar\n");
	printf("\tdevem estar dentro do alcance de um tipo inteiro, relativo à arquitetura\n\n");
	printf("\tO título do livro não deve ser maior que o macro INPUT_MAX-25 (999 por padrão)\n\n");
	printf("\tCaso o conteúdo do(s) arquivo(s) de entrada não sigam este padrão\n");
	printf("\to programa fecha, potencialmente levando a um erro de segmentação\n\n");
	printf("\tCaso ocorra algum erro no processamento de alguma operação, o programa\n");
	printf("\tescreve no(s) arquivo(s) de saída seu código em forma negativa, mas\n");
	printf("\tcontinua o processamento\n\n");
	printf("\tCaso a quantidade de arquivos de saída seja igual à de entrada, cada\n");
	printf("\tsaída será feita respectiva aos mesmos. Caso seja menor, será feita\n");
	printf("\tuma divisão (igual ou não) para os mesmos. Caso seja maior, o programa\n");
	printf("\tindicará um erro\n\n");
	
	printf("Operação de cadastrar livro:\n");
	printf("\tCadastra um livro na Árvore B interna do programa e no seu banco de dados local\n\n");
	printf("\tUtilização:\n");
	printf("\t\tcom a interface interativa: siga as instruções do menu de cadastro\n");
	printf("\t\tcom a interface de arquivos: 1 codigo_do_livro \"titulo_do_livro\"\n");
	printf("\t\t\tExemplo de entrada: 1 123 \"O Pequeno Principe\"\n");
	printf("\t\t\tExemplo de saída: nada ou -1, caso o código já exista\n\n");
	
	printf("Operação de remover livro:\n");
	printf("\tRemove um livro da Árvore B interna do programa e no seu banco de dados local\n");
	printf("\tapenas se a quantidade desse livro for igual a 0\n\n");
	printf("\tUtilização:\n");
	printf("\t\tcom a interface interativa: siga as instruções do menu de remoção\n");
	printf("\t\tcom a interface de arquivos: 2 codigo_do_livro\n");
	printf("\t\t\tExemplo de entrada: 2 123\n");
	printf("\t\t\tExemplo de saída: nada ou -2, caso o código não exista ou a quantidade do livro seja maior que 0\n\n");
	
	printf("Operação de procurar livro:\n");
	printf("\tProcura um livro na Árvore B interna do programa, retornando suas informações.\n\n");
	printf("\tUtilização:\n");
	printf("\t\tcom a interface interativa: siga as instruções do menu de procura\n");
	printf("\t\tcom a interface de arquivos: 3 codigo_do_livro\n");
	printf("\t\t\tExemplo de entrada: 3 123\n");
	printf("\t\t\tExemplo de saída: 123 0 \"O Pequeno Principe\" ou -3, caso o código não exista\n\n");
	
	printf("Operação de incrementar livro:\n");
	printf("\tAumenta a quantidade de um livro na biblioteca\n\n");
	printf("\tUtilização:\n");
	printf("\t\tcom a interface interativa: siga as instruções do menu de incremento\n");
	printf("\t\tcom a interface de arquivos: 4 quantidade codigo_do_livro\n");
	printf("\t\t\tExemplo de entrada: 4 10 123\n");
	printf("\t\t\tExemplo de saída: nada ou -4, caso o código não exista\n\n");
	
	printf("Operação de decrementar livro:\n");
	printf("\tDiminui a quantidade de um livro na biblioteca\n\n");
	printf("\tUtilização:\n");
	printf("\t\tcom a interface interativa: siga as instruções do menu de decremento\n");
	printf("\t\tcom a interface de arquivos: 5 quantidade codigo_do_livro\n");
	printf("\t\t\tExemplo de entrada: 5 10 123\n");
	printf("\t\t\tExemplo de saída: nada ou -5, caso o código não exista ou a quantidade solicitada é maior que a disponível\n\n");
	
	printf("Operação de imprimir catálogo:\n");
	printf("\tImprime as informações do sistema (total de livros e quantidade) e dos livros\n\n");
	printf("\tUtilização:\n");
	printf("\t\tcom a interface interativa: siga as instruções do menu de catálogo\n");
	printf("\t\tcom a interface de arquivos: 6\n");
	printf("\t\t\tExemplo de entrada: 6\n");
	printf("\t\t\tExemplo de saída: 1\\n0\\n123 0 \"O Pequeno Principe\"\n\n");
	
}