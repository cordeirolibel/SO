/**========================================================
| UTFPR  2018   Operating System
| https://github.com/gabrielsouzaesilva/
| https://github.com/cordeirolibel/
=========================================================**/

#include <stdio.h>
#include "queue.h"


//------------------------------------------------------------------------------
// Insere um elemento no final da fila.
// Condicoes a verificar, gerando msgs de erro:
// - a fila deve existir
// - o elemento deve existir
// - o elemento nao deve estar em outra fila

void queue_append (queue_t **queue, queue_t *elem) {  

	//================================
	//=========Mensagens de ERRO

	//a fila deve existir
	if (queue == NULL){
		printf("ERRO: Fila nao existe.\n");
		return;
	}
	//o elemento deve existir
	else if (elem == NULL){
		printf("ERRO: Elemento nao existe.\n");
		return;
	}
	//o elemento nao deve estar em outra fila
	else if (elem->next != NULL || elem->prev != NULL){
		printf("ERRO: Elemento esta em outra fila.\n");
		return;
	}

	//================================
	//========= Adiciona elem

	if (*queue == NULL){ // Fila Vazia
		*queue = elem;
		(*queue)->prev = *queue;
		(*queue)->next = *queue;
	}
	else{ //Fila nao vazia
		elem->next = (*queue);
		elem->prev = (*queue)->prev;

		(*queue)->prev->next = elem;
		(*queue)->prev = elem;
	}

	return;
}

//------------------------------------------------------------------------------
// Remove o elemento indicado da fila, sem o destruir.
// Condicoes a verificar, gerando msgs de erro:
// - a fila deve existir
// - a fila nao deve estar vazia
// - o elemento deve existir
// - o elemento deve pertencer a fila indicada
// Retorno: apontador para o elemento removido, ou NULL se erro

queue_t *queue_remove (queue_t **queue, queue_t *elem) {
	//A fila deve existir
	if (queue == NULL)
	{
		printf("ERRO: Fila nao existe.\n");
		return NULL;
	}
	//A fila não deve estar vazia
	else if (*queue == NULL)
	{
		printf("ERRO: Fila vazia.\n");
		return NULL;
	}
	// O elemento deve existir
	else if (elem == NULL)
	{
		printf("ERRO: Elemento nao existe.\n");
		return NULL;
	}
	// O elemento deve pertencer a fila indicada
	else
	{
		queue_t* aux = (*queue);
		
		do
		{
			if (aux->next == elem->next)
			{
				if (queue_size(*queue) > 1)
				{
					aux = elem;
					if (aux == *queue)
					{
						(*queue) = aux->next;
						aux->prev->next = aux->next;
						aux->next->prev = aux->prev;
						elem->next = NULL;
						elem->prev = NULL;
						return aux;
					}
					else
					{
						aux->prev->next = aux->next;
						aux->next->prev = aux->prev;
						elem->next = NULL;
						elem->prev = NULL;
						return aux;
					}
					
				}
				else
				{
					aux = elem;
					(*queue) = NULL;
					elem->next = NULL;
					elem->prev = NULL;
					return aux;
				}
			}
			else
			{
				aux = aux->next; // Proximo elemento
			}
		} while(aux != (*queue));

		printf("ERRO: Elemento nao pertence a fila.\n");
		return NULL;
	}
}

//------------------------------------------------------------------------------
// Conta o numero de elementos na fila
// Retorno: numero de elementos na fila

int queue_size (queue_t *queue) { 
	//vazia
	if (queue == NULL)
		return 0;

	//nao vazia
	int i = 1;
	queue_t* queue_moved = queue->next;
	while(queue_moved!=queue){
		queue_moved = queue_moved->next;
		i++;
	}
	
	return i;
}

//------------------------------------------------------------------------------
// Percorre a fila e imprime na tela seu conteúdo. A impressão de cada
// elemento é feita por uma função externa, definida pelo programa que
// usa a biblioteca. Essa função deve ter o seguinte protótipo:
//
// void print_elem (void *ptr) ; // ptr aponta para o elemento a imprimir

void queue_print (char *name, queue_t *queue, void print_elem (void*) ) {

	printf("%s: [",name);
	print_elem(queue);

	//nao vazia
	if (queue != NULL){
		queue_t* queue_moved = queue->next;
		while(queue_moved!=queue){

			printf(" ");
			print_elem(queue_moved);
			queue_moved = queue_moved->next;
		}
	}


	printf("]\n");
	return;
}
