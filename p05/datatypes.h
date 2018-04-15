// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DAINF UTFPR
// Versão 1.0 -- Março de 2015
//
// Estruturas de dados internas do sistema operacional

#ifndef __DATATYPES__
#define __DATATYPES__

#include <ucontext.h>

// Estrutura que define uma tarefa
typedef struct task_t
{
	// para usar com a biblioteca de filas (cast
	struct task_t *prev, *next;
	// ID da tarefa
	int tid;
	// contexto 
	ucontext_t *context;

	// Prioridade
	int statPrio;
	int dinPrio;

	// tarefa do sistema ou do usuario 
	int sys_tf;
	
} task_t ;

// estrutura que define um semáforo
typedef struct
{
  // preencher quando necessário
} semaphore_t ;

// estrutura que define um mutex
typedef struct
{
  // preencher quando necessário
} mutex_t ;

// estrutura que define uma barreira
typedef struct
{
  // preencher quando necessário
} barrier_t ;

// estrutura que define uma fila de mensagens
typedef struct
{
  // preencher quando necessário
} mqueue_t ;

#endif
