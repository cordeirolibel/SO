/**========================================================
| UTFPR  2018   Operating System
| https://github.com/gabrielsouzaesilva/
| https://github.com/cordeirolibel/
=========================================================**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
task_t *queue_sleep;

ucontext_t ct_main;

int id_tasks;

int ticks;//ticks para a tarefa (quantum)
unsigned int clkTicks; //ticks desde o inicio

int last_exit;//ultimo valor de exitS

// estrutura que define um tratador de sinal (deve ser global ou static)
struct sigaction action ;

// estrutura de inicialização to timer
struct itimerval timer;

//==============================================================
// Funções gerais ==============================================

//interrupção de tempo 
void interrupt_handler(int signum){
	//tarefas do sistema nao sao interrompidas
	if (tk_atual->sys_tf == 1){
		clkTicks += 1;
		tk_atual->tProcessador += 1;
		return;
	}

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
	// desativa o buffer da saida padrao (stdout), usado pela função printf 
	setvbuf (stdout, 0, _IONBF, 0);

	// Inicializa numero de ticks de relogio
	clkTicks = 0;

	// ===> Criando tarefa main
	// C++  >> task_t* tk_main = new task_t;
	tk_main = malloc(sizeof(*tk_main));
	// C++  >> ucontext_t* ct_main = new ct_main;
	//ucontext_t* ct_main = malloc(sizeof(*ct_main));
	//getcontext(ct_main); // armazena contexto atual em ct_main
	//tk_main->context = ct_main;

	//id de tarefas inicia em 0
	id_tasks = 0;
	queue_tks = NULL;

	//Inicia fila de tarefas adormecidas
	queue_sleep = NULL;

	task_create(tk_main, NULL, NULL);
	//task_setprio(tk_main, 1);//<<<<========================================

	//tarefa dispatcher
	tk_dispatcher = malloc(sizeof(*tk_dispatcher));
	task_create(tk_dispatcher,(void*)dispatcher_body,NULL);
	tk_dispatcher->sys_tf = 1;

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


	task->tk_inicio = systime();
	task->tProcessador = 0;
	task->ativ = 0;

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
	// Ajusta contexto de task
	task->context = context;

	//fila de suspensas
	task->queue_tks_susp = NULL;

	// Ajusta ponteiros de fila
	task->next = NULL;
	task->prev = NULL;

	// Tarefa criada com prioridade 0
	task->statPrio = 0;
	task->dinPrio = 0;

	task->wakeUp = 0;

	// Ajusta id
	task->tid = id_tasks;
	id_tasks++;

	if (start_func != NULL)
	{
		// ajusta alguns valores internos do contexto salvo em context
		makecontext (context, (void*)(*start_func), 1, arg);
	}

	// Se estiver criando o dispatcher, retorna 0 e terminar task_create.
	if (task == tk_dispatcher)
	{
		return 0;
	}
	else if (task != tk_main)// Se não coloca tarefas na fila de prontas
		queue_append((queue_t**) &queue_tks, (queue_t*) task);


	//tarefa do usuario
	task->sys_tf = 0;

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
	int result;

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

	printf("Task %d exit: execution time %u ms, processor time %u ms, %d activations \n", task_id(), tempoExec, tk_atual->tProcessador, tk_atual->ativ);

	//==== Acordando tarefas suspensas
	//numero de tarefas na fila
	task_t *queue = tk_atual->queue_tks_susp;
	int userTasks = queue_size ((queue_t*) queue);
	while ( userTasks > 0 )	{

		task_resume (queue, &queue);
		userTasks = queue_size ((queue_t*) queue);
	}
	last_exit = exitCode;

	// desalocar
	free(tk_atual->context);

	//====verifica se existem tarefas na fila de prontas
	//numero de tarefas na fila
	userTasks = queue_size ((queue_t*) queue_tks);

	if ((userTasks >= 0) && tk_atual != tk_dispatcher)
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
	int userTasks_sleep = queue_size ((queue_t*) queue_sleep);

	while ( userTasks > 0 || userTasks_sleep>0 )	
	{
		// Ponteiro auxiliar para manipulação de elementos na fila de adormecidas.
		task_t *alarmClk = queue_sleep;

		int n_sleep_tk = queue_size ((queue_t*) queue_sleep);

		// Percorre a fila de adormecidas e procura quais devem ser acordadas
		while(n_sleep_tk--){
			//Salva o proximo
			next = alarmClk->next;

			if (alarmClk->wakeUp <= clkTicks)
			{	

				alarmClk->wakeUp = 0;
				//Remove a tarefa da fila de tarefas adormecidas
				queue_remove ((queue_t**) &queue_sleep, (queue_t*) alarmClk);

				//Adiciona a tarefa na fila de prontas
				queue_append((queue_t**) &queue_tks, (queue_t*) alarmClk);

				userTasks = queue_size ((queue_t*) queue_tks);
			}
			alarmClk = next;
			
		} 

		//se tiver uma tarefa na fila de prontas
		if (userTasks>0){
			next = scheduler(); // scheduler é uma função
			
			if (next){
				//... // ações antes de lançar a tarefa "next", se houverem
				ticks = QUANTUM;
				task_switch (next) ; // transfere controle para a tarefa "next"
				//... // ações após retornar da tarefa "next", se houverem
			}
		}

		userTasks = queue_size ((queue_t*) queue_tks);
		userTasks_sleep = queue_size ((queue_t*) queue_sleep);
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

	//printf("%d\n", highPrio->tid);

	return (task_t*) queue_remove ((queue_t**) &queue_tks, (queue_t*) highPrio);
}

// suspende uma tarefa, retirando-a de sua fila atual, adicionando-a à fila
// queue e mudando seu estado para "suspensa"; usa a tarefa atual se task==NULL
void task_suspend (task_t *task, task_t **queue) {

	//o elemento esta em outra fila (sleep)
	if (task->next != NULL || task->prev != NULL){
		queue_remove ((queue_t**) &queue_sleep, (queue_t*) task);
	}

	//adicionando a fila de suspensao
	queue_append((queue_t**) queue, (queue_t*) task);

	task_switch(tk_dispatcher);
}

// acorda uma tarefa, retirando-a de sua fila atual, adicionando-a à fila de
// tarefas prontas ("ready queue") e mudando seu estado para "pronta"
void task_resume (task_t *task, task_t **queue) {

	//removendo da fila suspensao
	queue_remove ((queue_t**) queue, (queue_t*) task);

	//sleep
	if (task->wakeUp != 0){
		//adicionando a fila de sleep
		queue_append((queue_t**) &queue_sleep, (queue_t*) task);
	}
	else{
		//adicionando a fila de prontas
		queue_append((queue_t**) &queue_tks, (queue_t*) task);
	}

}

//==============================================================
// Operações de escalonamento ==================================

// libera o processador para a próxima tarefa, retornando à fila de tarefas
// prontas ("ready queue")
void task_yield () {
	if ((tk_dispatcher != tk_atual))
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
int task_join (task_t *task) {
	//Caso a tarefa b nao exista ou ja tenha encerrado, 
	//esta chamada deve retornar imediatamente, 
	//sem suspender a tarefa corrente.
	if (task == NULL || task->prev == NULL){
		return -1;
	}

	//suspende tarefa
	task_suspend(tk_atual,&task->queue_tks_susp);

	return last_exit;
}

//==============================================================
// Operações de gestão do tempo ================================

// suspende a tarefa corrente por t segundos
void task_sleep (int t)
{
	//Tempo para ser acordada
	tk_atual->wakeUp = clkTicks + t*1000;

	//Remove a tarefa da fila de prontas
	//queue_remove ((queue_t**) &queue_tks, (queue_t*) tk_atual);

	//Adiciona a tarefa na fila de tarefas adormecidas
	queue_append((queue_t**) &queue_sleep, (queue_t*) tk_atual);
	
	task_switch(tk_dispatcher);
}

// retorna o relógio atual (em milisegundos)
unsigned int systime ()
{
	return clkTicks;
}

//==============================================================
// Operações de IPC ============================================

// semáforos

// cria um semáforo com valor inicial "value"
int sem_create (semaphore_t *s, int value)
{
	s->counter = value;

	//fila de suspensas
	s->queue_tks_susp = NULL;
}

// requisita o semáforo
int sem_down (semaphore_t *s)
{
	tk_atual->sys_tf = 1;
	if (s == NULL)
	{
		return -1;
	}
	else
	{
		s->counter = s->counter - 1;
		if (s->counter < 0)
		{
			//Tarefa vai para a fila de suspensas do semaforo
			queue_append((queue_t**) &s->queue_tks_susp, (queue_t*) tk_atual);
			tk_atual->sys_tf = 0;
			task_switch(tk_dispatcher);
			return 0;
		}
		else
		{
			tk_atual->sys_tf = 0;
			return 0;
		}
	}
}

// libera o semáforo
int sem_up (semaphore_t *s)
{
	int save_sys;
	save_sys = tk_atual->sys_tf;
	tk_atual->sys_tf = 1;
	if (s == NULL)
	{
		tk_atual->sys_tf = save_sys;
		return -1;
	}
	else
	{
		s->counter = s->counter + 1;
		if (s->counter <= 0)
		{
			if(s->queue_tks_susp != NULL)
			{

				task_resume(s->queue_tks_susp, &(s->queue_tks_susp));
				tk_atual->sys_tf = save_sys;
				return 0;
			}
				
		}

		tk_atual->sys_tf = save_sys;
		return 0;
	}
}

// destroi o semáforo, liberando as tarefas bloqueadas
int sem_destroy (semaphore_t *s)
{
	int queueSize;
	int i;


	if ( s == NULL)
	{
		return -1;
	}
	else
	{
		if (s->queue_tks_susp != NULL)
		{
			queueSize = queue_size ((queue_t*) s->queue_tks_susp);
			// Ponteiro auxiliar para manipulação de elementos.
			task_t* aux = s->queue_tks_susp;

			for(i=0; i<queueSize; i++)
			{
				task_resume(aux, &s->queue_tks_susp);
				aux = aux->next;
			}
		}
	}

	s = NULL;
	return 0;
}

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
int mqueue_create (mqueue_t *queue, int max, int size) {

	//erro
	if (queue == NULL)
		return -1;

	//set
	queue->max = max;
	queue->size = size;
	queue->queue = NULL;
	queue->queue_tks_susp_recv = NULL;
	queue->queue_tks_susp_send = NULL;
	return 0;
}

// envia uma mensagem para a fila
int mqueue_send (mqueue_t *queue, void *msg) {
	int n_itens;
	mqueue_t* item;

	//erro
	if (queue == NULL)
		return -1;

	//bloqueante: caso a fila esteja cheia
	n_itens = mqueue_msgs(queue);
	while (queue->max <= n_itens){
		task_suspend (tk_atual, &(queue->queue_tks_susp_send));
		n_itens = mqueue_msgs(queue);
	}

	//copia mensagem
	item = malloc(sizeof(mqueue_t));
	item->dado = malloc(sizeof(queue->size));
	memcpy(&(item->dado),msg,queue->size);

	queue_append((queue_t**) &(queue->queue), (queue_t*) item);

	//libera tarefa
	n_itens = queue_size ((queue_t*) queue->queue_tks_susp_recv);
	if (n_itens>0){
		task_resume (queue->queue_tks_susp_recv, &(queue->queue_tks_susp_recv));
	}
	return 0;
}

// recebe uma mensagem da fila
int mqueue_recv (mqueue_t *queue, void *msg) {
	int n_itens;

	//erro
	if (queue == NULL)
		return -1;

	//bloqueante: caso não tenha mensagem
	n_itens = mqueue_msgs(queue);
	while(n_itens<=0){
		task_suspend (tk_atual, &(queue->queue_tks_susp_recv));
		n_itens = mqueue_msgs(queue);
	}

	//remove item
	memcpy(msg,&(queue->queue->dado),queue->size);

	queue_remove ((queue_t**) &(queue->queue), (queue_t*)queue->queue);
	//free(item->dado);
	//free(item);

	//libera tarefa
	n_itens = queue_size ((queue_t*) queue->queue_tks_susp_send);
	if (n_itens>0){
		task_resume (queue->queue_tks_susp_send, &(queue->queue_tks_susp_send));
	}
	return 0;
}

// destroi a fila, liberando as tarefas bloqueadas
int mqueue_destroy (mqueue_t *queue) {
	// Ponteiro auxiliar para manipulação de elementos.
	task_t* aux;
	int n_itens, i;

	//libera tarefas send
	aux = queue->queue_tks_susp_send;
	n_itens = queue_size ((queue_t*) queue->queue_tks_susp_send);
	for(i=0; i<n_itens; i++)
	{
		aux = aux->next;
		task_resume(aux->prev, &queue->queue_tks_susp_send);
		
	}

	//libera tarefas recebe
	aux = queue->queue_tks_susp_recv;
	n_itens = queue_size ((queue_t*) queue->queue_tks_susp_recv);
	for(i=0; i<n_itens; i++)
	{
		aux = aux->next;
		task_resume(aux->prev, &queue->queue_tks_susp_recv);
		
	}
	return 0;
}

// informa o número de mensagens atualmente na fila
int mqueue_msgs (mqueue_t *queue) {
	return queue_size ((queue_t*) queue->queue);
}
