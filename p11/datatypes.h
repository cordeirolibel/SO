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

	// fila de tarefas suspensas esperando essa tarefa
	struct task_t *queue_tks_susp;

	// ID da tarefa
	int tid;
	// contexto 
	ucontext_t *context;

	// para a funcao task_sleep
	int wakeUp;

	// Prioridade
	int statPrio;
	int dinPrio;

	// tarefa do sistema ou do usuario 
	int sys_tf;

	// Vida da tarefa (Em unidades de ticks de relógio)
	int tk_inicio;
	int tProcessador; // Tempo de processador
	int ativ; // Numero de ativações

	// Flag para uso de semaforo
	int flagSem;


} task_t ;

// estrutura que define um semáforo
typedef struct
{
	int counter;

  	// fila de tarefas suspensas esperando essa tarefa
	struct task_t *queue_tks_susp;
  
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
	int n_threads;
	int total_threads;
	struct task_t *queue_barrier;

} barrier_t ;

// estrutura que define uma fila de mensagens
typedef struct
{
  // preencher quando necessário
} mqueue_t ;

#endif
