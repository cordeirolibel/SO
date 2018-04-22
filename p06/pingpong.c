/**========================================================
| UTFPR  2018   Operating System
| https://github.com/gabrielsouzaesilva/
| https://github.com/cordeirolibel/
=========================================================**/

#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
//#include "datatypes.h"		// estruturas de dados necessárias
#include "pingpong.h"
//#include "../p00/queue.h"
#include <signal.h>
#include <sys/time.h>

#define STACKSIZE 32768		/* tamanho de pilha das threads */
#define QUANTUM 20  /* 1 Quantum = 20 ticks */

//==============================================================
// Variaveis globais ===========================================

// Estrutura que define uma tarefa main
task_t *tk_main;
task_t *tk_dispatcher;
task_t *tk_atual;
task_t *queue_tks;

ucontext_t ct_main;

int id_tasks;

int ticks;
unsigned int clkTicks;


// estrutura que define um tratador de sinal (deve ser global ou static)
struct sigaction action ;

// estrutura de inicialização to timer
struct itimerval timer;

//==============================================================
// Funções gerais ==============================================

//Simples print para testes
void test(int num){
	printf("Teste %d\n",num);
	fflush(stdout);
}

void interrupt_handler(int signum){

	//tarefas do sistema nao sao interrompidas
	if (tk_atual->sys_tf == 1)
		return;

	// Quantum
	if (ticks == 0){

		//print DEBUG
		#ifdef DEBUG
		printf ("Quantum \n");
		fflush(stdout);
		#endif

		//acabou o tempo da tarefa
		task_yield ();
	}
	else
		ticks-=1;

	clkTicks += 1;
	tk_atual->tProcessador += 1;

	return;
}


// Inicializa o sistema operacional; deve ser chamada no inicio do main()
void pingpong_init () {
	// Inicializa numero de ticks de relogio
	clkTicks = 0;

	// desativa o buffer da saida padrao (stdout), usado pela função printf 
	setvbuf (stdout, 0, _IONBF, 0);

	// ===> Criando tarefa main
	// C++  >> task_t* tk_main = new task_t;
	tk_main = malloc(sizeof(*tk_main)); 
	// C++  >> ucontext_t* ct_main = new ct_main;
	ucontext_t* ct_main = malloc(sizeof(*ct_main));
	getcontext(ct_main); // armazena contexto atual em ct_main
	tk_main->context = ct_main;
	tk_main->sys_tf = 0;

	//id de tarefas inicia em 0
	id_tasks = 0;
	queue_tks = NULL;

	//tarefa dispatcher
	tk_dispatcher = malloc(sizeof(*tk_dispatcher));
	task_create(tk_dispatcher,(void*)dispatcher_body,NULL);
	tk_dispatcher->sys_tf = 1;

	tk_dispatcher->tk_inicio = 0;
	tk_dispatcher->tProcessador = 0;
	tk_dispatcher->ativ = 0;

	//Referencia para task atual
	tk_atual = tk_main;

	///==============================Timer init
	/// registra a a��o para o sinal de timer SIGALRM
	action.sa_handler = interrupt_handler ;
	sigemptyset (&action.sa_mask) ;
	action.sa_flags = 0 ;
	if (sigaction (SIGALRM, &action, 0) < 0){
		perror ("Erro em sigaction: ") ;
		exit (1) ;
	}

	// ajusta valores do temporizador
	timer.it_value.tv_usec = 1000 ;      // primeiro disparo, em micro-segundos
	timer.it_value.tv_sec  = 0 ;      // primeiro disparo, em segundos
	timer.it_interval.tv_usec = 1000 ;   // disparos subsequentes, em micro-segundos
	timer.it_interval.tv_sec  = 0 ;   // disparos subsequentes, em segundos

	// arma o temporizador ITIMER_REAL (vide man setitimer)
	if (setitimer (ITIMER_REAL, &timer, 0) < 0)	{
		perror ("Erro em setitimer: ") ;
		exit (1) ;
	}
	///==============================

	#ifdef DEBUG
	printf ("pingpong init conlcuido.\n");
	fflush(stdout);
	#endif

	return;
}



//==============================================================
// Gerência de tarefas =========================================

// Cria uma nova tarefa. Retorna um ID> 0 ou erro.
int task_create (task_t *task,			// descritor da nova tarefa
                 void (*start_func)(void *),	// funcao corpo da tarefa
                 void *arg) {			// argumentos para a tarefa

	// Crianto contexto
	ucontext_t* context = malloc(sizeof(*context));

	getcontext (context);

	//Criando Pilha
	char *stack = malloc (STACKSIZE) ;
	if (stack) {
		context->uc_stack.ss_sp = stack ;
		context->uc_stack.ss_size = STACKSIZE;
		context->uc_stack.ss_flags = 0;
		context->uc_link = 0;
	}
	else {
		perror ("Erro na criação da pilha: ");
		exit (1);
	}

	// Ajusta id
	id_tasks++;
	task->tid = id_tasks;

	// Ajusta contexto de task
	task->context = context;

	// Ajusta ponteiros de fila
	task->next = NULL;
	task->prev = NULL;

	// Tarefa criada com prioridade 0
	task->statPrio = 0;
	task->dinPrio = 0;



	// ajusta alguns valores internos do contexto salvo em context
	makecontext (context, (void*)(*start_func), 1, arg);

	// Se estiver criando o dispatcher, retorna 0 e terminar task_create.
	if (task == tk_dispatcher)
	{
		return 0;
	}
	else // Se não coloca tarefas na fila de prontas
		queue_append((queue_t**) &queue_tks, (queue_t*) task);

	//tarefa do usuario
	task->sys_tf = 0;

	task->tk_inicio = systime();
	task->tProcessador = 0;
	task->ativ = 0;

	#ifdef DEBUG
	printf ("task_create: criou tarefa %d\n", task->tid) ;
	fflush(stdout);
	#endif

	return id_tasks;
}



// alterna a execução para a tarefa indicada
int task_switch(task_t *task) {

	task->ativ += 1;

	task_t *tk_aux;
	int result = 0;

	tk_aux = tk_atual;
	tk_atual = task;
	result = swapcontext(tk_aux->context, tk_atual->context); // swapcontex retorna 0 se ok ou -1 se der erro.

	#ifdef DEBUG
	printf ("task_switch: trocando contexto %d -> %d\n", tk_aux->tid, tk_atual->tid);
	fflush(stdout);
	#endif

	return result;
}


// Termina a tarefa corrente, indicando um valor de status encerramento
void task_exit (int exitCode) {
	unsigned int tempoExec = systime();
	tempoExec = tempoExec - tk_atual->tk_inicio;

	printf("Task %d exit: execution time %4d ms, processor time %4d ms, %d activations \n", task_id(), tempoExec, tk_atual->tProcessador, tk_atual->ativ);

	// desalocar
	free(tk_atual->context);

	//====verifica se existem tarefas na fila de prontas
	//numero de tarefas na fila
	int userTasks = queue_size ((queue_t*) queue_tks);

	if ((userTasks >= 0) && (tk_atual->tid!=1))
	{
		// controle ao dispatcher
		task_switch(tk_dispatcher);
	}
	else
	{
		task_switch(tk_main);
	}


	#ifdef DEBUG
	printf ("task_exit: tarefa %d sendo encerrada\n", tk_atual->tid);
	fflush(stdout);
	#endif

	return;
}


// retorna o identificador da tarefa corrente (main eh 0)
int task_id ()
{
	return tk_atual->tid;
}

void dispatcher_body (void * arg){ // dispatcher é uma tarefa
	task_t *next;

	//numero de tarefas na fila
	int userTasks = queue_size ((queue_t*) queue_tks);

	while ( userTasks > 0 )	{
		next = scheduler() ; // scheduler é uma função
		if (next){
			//... // ações antes de lançar a tarefa "next", se houverem
			ticks = QUANTUM;
			task_switch (next) ; // transfere controle para a tarefa "next"
			//... // ações após retornar da tarefa "next", se houverem
		}
		userTasks = queue_size ((queue_t*) queue_tks);
	}

	task_exit(0) ; // encerra a tarefa dispatcher
}

//remove o primeiro elemento da fila e retorna o elemento removido 
task_t* scheduler(){
	// Menor prioridade
	int prevPrio = 20;

	// Ponteiro auxiliar para a tarefa com maior prioridade.
	task_t* highPrio = NULL;

	// Ponteiro auxiliar para manipulação de elementos.
	task_t* aux = queue_tks;

	// Percorre a fila de prontas e procura maior prioridade, ou seja, menor valor de dinPrio.
	do
	{
		if (aux->dinPrio <= prevPrio)
		{
			prevPrio = aux->dinPrio;
			highPrio = aux;
		}
		aux = aux->next;
	} while(aux != queue_tks);

	// Percorre a fila novamente, fazendo o envelhecimento das tarefas.
	aux = queue_tks;
	do
	{
		if (aux != highPrio)
		{
			if (aux->dinPrio == highPrio->dinPrio) // No caso de dois elementos na fila com mesmo valor de dinPrio.
			{
				if(aux->statPrio <= highPrio->statPrio) // Verifica qual dos dois tem maior prioridade estatica.
				{
					if(aux->tid <= highPrio->tid) // Verifica quem entrou na fila antes
						highPrio = aux; 
					else
						aux->dinPrio += -1; // Fator de envelhecimento = -1 (task aging)	
				}
				else
				{
					aux->dinPrio += -1; // Fator de envelhecimento = -1 (task aging)	
				}
			}
			else
				aux->dinPrio += -1; // Fator de envelhecimento = -1 (task aging)	
		}
		aux = aux->next;
	} while(aux != queue_tks);

	highPrio->dinPrio = highPrio->statPrio;

	return (task_t*) queue_remove ((queue_t**) &queue_tks, (queue_t*) highPrio);
}

// suspende uma tarefa, retirando-a de sua fila atual, adicionando-a à fila
// queue e mudando seu estado para "suspensa"; usa a tarefa atual se task==NULL
void task_suspend (task_t *task, task_t **queue) ;

// acorda uma tarefa, retirando-a de sua fila atual, adicionando-a à fila de
// tarefas prontas ("ready queue") e mudando seu estado para "pronta"
void task_resume (task_t *task) ;

//==============================================================
// Operações de escalonamento ==================================

// libera o processador para a próxima tarefa, retornando à fila de tarefas
// prontas ("ready queue")
void task_yield () {
	if ((tk_dispatcher != tk_atual) && (tk_atual != tk_main))
	{
		queue_append ((queue_t **) &queue_tks, (queue_t*) tk_atual) ;
		task_switch(tk_dispatcher);
	}
	else
		task_switch(tk_dispatcher);
}

// define a prioridade estática de uma tarefa (ou a tarefa atual)
void task_setprio (task_t *task, int prio)
{
	if (task == NULL)
	{
		tk_atual->statPrio = prio;
		tk_atual->dinPrio = prio;
	}
	else if (prio >= -20 || prio <= 20)
	{
		task->statPrio = prio;
		task->dinPrio = prio;
	}
	else
		printf("Error valor de prioridade proibido.\n");

}

// retorna a prioridade estática de uma tarefa (ou a tarefa atual)
int task_getprio (task_t *task)
{
	if (task == NULL)
		return tk_atual->statPrio;
	else
		return task->statPrio;
}

//==============================================================
// Operações de sincronização ==================================

// a tarefa corrente aguarda o encerramento de outra task
int task_join (task_t *task) ;

//==============================================================
// Operações de gestão do tempo ================================

// suspende a tarefa corrente por t segundos
void task_sleep (int t) ;

// retorna o relógio atual (em milisegundos)
unsigned int systime ()
{
	return clkTicks;
}

//==============================================================
// Operações de IPC ============================================

// semáforos

// cria um semáforo com valor inicial "value"
int sem_create (semaphore_t *s, int value) ;

// requisita o semáforo
int sem_down (semaphore_t *s) ;

// libera o semáforo
int sem_up (semaphore_t *s) ;

// destroi o semáforo, liberando as tarefas bloqueadas
int sem_destroy (semaphore_t *s) ;

// mutexes

// Inicializa um mutex (sempre inicialmente livre)
int mutex_create (mutex_t *m) ;

// Solicita um mutex
int mutex_lock (mutex_t *m) ;

// Libera um mutex
int mutex_unlock (mutex_t *m) ;

// Destrói um mutex
int mutex_destroy (mutex_t *m) ;

// barreiras

// Inicializa uma barreira
int barrier_create (barrier_t *b, int N) ;

// Chega a uma barreira
int barrier_join (barrier_t *b) ;

// Destrói uma barreira
int barrier_destroy (barrier_t *b) ;

// filas de mensagens

// cria uma fila para até max mensagens de size bytes cada
int mqueue_create (mqueue_t *queue, int max, int size) ;

// envia uma mensagem para a fila
int mqueue_send (mqueue_t *queue, void *msg) ;

// recebe uma mensagem da fila
int mqueue_recv (mqueue_t *queue, void *msg) ;

// destroi a fila, liberando as tarefas bloqueadas
int mqueue_destroy (mqueue_t *queue) ;

// informa o número de mensagens atualmente na fila
int mqueue_msgs (mqueue_t *queue) ;
