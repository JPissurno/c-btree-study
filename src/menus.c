#include "library.h"


char menu_main(void){
	
	char OP, input[INPUT_MAX];
	
	while(1){
		
		printf("=========Livraria=========\n");
		printf("||[1] Inserir livro     ||\n");
		printf("||[2] Remover livro     ||\n");
		printf("||[3] Procurar livro    ||\n");
		printf("||[4] Acrescentar livro ||\n");
		printf("||[5] Decrementar livro ||\n");
		printf("||[6] Catálogo          ||\n");
		printf("||[7] Ajuda             ||\n");
		printf("||[0] Sair...           ||\n");
		printf("==========================\n");
		printf("||Input: ");
		fgets(input, sizeof(input), stdin);
		menu_clrscr();
		
		if((OP = input[0]) >= '0' && OP <= '7') break;
		else printf("Opção inválida.\n");
		
	}
	
	return OP-48;
	
}


struct Book menu_insert(void){
	
	struct Book book;
	char flag = 1, input[INPUT_MAX];
	
	for(int i = 0; flag; ){
		
		memset(input, 0, sizeof(input));
		printf("==================================================\n");
		printf("||##############################################||\n");
		printf("||############### Inserir Livro ################||\n");
		printf("||##############################################||\n");
		printf("==================================================\n");
		
		switch(i){
			case 0:
				printf("||Código do livro: ");
				book.code = safeInput(NULL);
				break;
				
			case 1:
				printf("||Título do livro: ");
				safeInput(book.title);
				break;
				
			case 2:
				printf("||Quantidade desse livro: ");
				book.quantity = safeInput(NULL);
				book.isAvailable = !!book.quantity;
				break;

			default:
				flag = 2;
				printf("||[1] Código: %d\n", book.code);
				printf("||[2] Título: %s\n", book.title);
				printf("||[3] Quantidade: %d\n", book.quantity);
				printf("||[4] Status: %s%s\n", book.quantity ? "" : "in", "disponível");
				printf("====================================\n");
				printf("||[Y]: confirmar                  ||\n");
				printf("||[N]: cancelar inserção          ||\n");
				printf("||[0]: recomeçar inserção         ||\n");
				printf("||[1~3]: alterar dado específico  ||\n");
				printf("====================================\n");
				printf("||Input: ");
				fgets(input, sizeof(input), stdin);
				
				if((i = input[0]-49) < 0 || i >= 3) flag = 3;
				if(input[0] == 'Y' || input[0] == 'N') flag = 0;
				if(input[0] == '0') flag = 1;
				
				break;
				
		}
		
		menu_clrscr();
		if(flag == 1) i++;
		if(flag == 2 && input[0] == 0) i = -1;
		if(flag == 3) printf("Opção inválida.\n");
		
	}
	
	if(input[0] != 'Y') book.isAvailable = -1;
	
	return book;
	
}


//int menu_remove(void){}


int menu_search(void){

	int n;
	char input[INPUT_MAX];

	while(1){

		printf("====================================================\n");
		printf("||################################################||\n");
		printf("||############### Pesquisar Livro ################||\n");
		printf("||################################################||\n");
		printf("====================================================\n");
		printf("||[0]: cancelar pesquisa ||\n");
		printf("===========================\n");
		printf("||Código do livro: ");
		fgets(input, sizeof(input), stdin);
		input[strlen(input)-1] = '\0';
		menu_clrscr();
		if(input[0] == '0') return 0;

		if((n = strtol(input, NULL, 10)) <= 0)
			printf("||Código inválido\n");
		else return n;

	}

}


struct Book menu_update(char type){
	
	struct Book book;
	char flag = 0, input[INPUT_MAX], op[12], *op2 = "Código do livro";

	strcpy(op, type ? "Incrementar" : "Decrementar");
	while(1){
		
		printf("======================================================\n");
		printf("||##################################################||\n");
		printf("||############### %s Livro ################||\n", op);
		printf("||##################################################||\n");
		printf("======================================================\n");
		if(flag) printf("||Livro de código %d      ||\n", book.code);
		printf("||[0]: cancelar operação ||\n");
		printf("===========================\n");
		printf("||%s: ", op2);
		fgets(input, sizeof(input), stdin);
		input[strlen(input)-1] = '\0';
		menu_clrscr();
		
		if(input[0] == '0'){
			
			book.isAvailable = -1;
			
			break;
			
		}
		
		if(!flag){
			
			if((book.code = strtol(input, NULL, 10)) <= 0)
				printf("||Código inválido\n");
			else {
				flag = 1;
				op2 = "Quantidade";
			}
			
		} else {
			
			if((book.quantity = strtol(input, NULL, 10)) <= 0)
				printf("||Quantidade inválida\n");
			else break;
			
		}
		
	}
	book.quantity *= type ? 1 : -1;
	
	return book;
	
}


int menu_list(struct Library* lib){
	
	struct Book *book;
	int x, qtd = 0, idx = 1, *code = calloc(1, sizeof(int));
	char input[INPUT_MAX], **title = calloc(1, sizeof(char*));

	while(book = btree_traverse(lib->db, &idx)){
		
		qtd++;
		code = realloc(code, (qtd+1)*sizeof(int));
		title = realloc(title, (qtd+1)*sizeof(char*));
		title[qtd-1] = calloc(strlen(book->title)+1, 1);
		
		code[qtd-1] = book->code;
		strcpy(title[qtd-1], book->title);
		
		idx = qtd+1;

	}

	if(!qtd){
		free(title);
		free(code);
		return -1;
	}

	while(1){

		printf("============================================\n");
		printf("||########################################||\n");
		printf("||############### Catálogo ###############||\n");
		printf("||########################################||\n");
		printf("============================================\n");
		for(int i = 1; i <= qtd; i++)
			printf("||[%d] %d - %s\n", i, code[i-1], title[i-1]);
		printf("==================================================\n");
		printf("||Total de livros: %d\n", qtd);
		printf("||[0]: cancelar operação\n");
		printf("||[1~%d]: %s\n", qtd, "informações sobre o livro");
		printf("==================================================\n");
		printf("||Input: ");
		fgets(input, sizeof(input), stdin);
		menu_clrscr();

		if(!(x = input[0]-48)) break;

		if((x = strtol(input, NULL, 10)) <= 0 || x > qtd)
			printf("||Opção inválida.\n");
		else break;

	}
	
	if(x) x = code[x-1];
	for(int i = 0; i < qtd; i++)
		free(title[i]);
	free(title);
	free(code);

	return x;
	
}


static int safeInput(char* str){
	
	int n;
	char input[INPUT_MAX];
	
	fgets(input, sizeof(input), stdin);
	input[strlen(input)-1] = '\0';
	
	if(str){
		
		strncpy(str, input, sizeof(input));

		if(!str[0]){
			str[0] = '?';
			str[1] = '\0';
		}
		
		return 0;
		
	}
	
	if((n = strtol(input, NULL, 10)) <= 0)
		return 0;
	
	return n;
	
}


void menu_clrscr(void){
	
    #if defined(__linux__) || defined(__unix__) || defined(__APPLE__)
        system("clear");
		return;
    #endif
    #if defined(_WIN32) || defined(_WIN64)
        system("cls");
    #endif
	
}