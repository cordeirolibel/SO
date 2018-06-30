#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "pingpong.h"
#define MAX_PROD 10
#define MAX_CONS 15

// operating system check
#if defined(_WIN32) || (!defined(__unix__) && !defined(__unix) && (!defined(__APPLE__) || !defined(__MACH__)))
#warning Este codigo foi planejado para ambientes UNIX (LInux, *BSD, MacOS). A compilacao e execucao em outros ambientes e responsabilidade do usuario.
#endif


typedef struct Fila
{
   struct Fila *prev ;  // ptr para usar cast com queue_t
   struct Fila *next ;  // ptr para usar cast com queue_t
   int id ;
   // outros campos podem ser acrescidos aqui
} Fila ;

semaphore_t s_vaga ;
semaphore_t s_buffer ;
semaphore_t s_item ;
Fila *fila;

void produtorBody(void *id)
{
    Fila* item;
    int i;

    srand(time(NULL));

    for(i=0;i<MAX_PROD;i++)
    {
        task_sleep (1);

        item = malloc (sizeof (Fila));
        item->id = rand()%100;
        item->next = NULL;
        item->prev = NULL;
       
        sem_down (&s_vaga);
        sem_down (&s_buffer);

        //insere item no buffer
        queue_append ((queue_t**) &fila, (queue_t*) item) ;

        sem_up (&s_buffer);
        sem_up (&s_item);

        printf("p%d produziu %d\n",task_id()-1,item->id);
    }

    task_exit (0) ;
}

void consumidorBody(void *id)
{
    Fila* item;
    int i, item_id;

    for(i=0;i<MAX_CONS;i++)
    {
        sem_down (&s_item);
        sem_down (&s_buffer);

        //retira item do buffer
        item = fila;
        queue_remove ((queue_t**) &fila, (queue_t*) item) ;
        item_id = item->id;
        free (item);

        sem_up (&s_buffer);
        sem_up (&s_vaga);

        //print item
        printf("\t\t\t\t\tc%d consumiu %d\n",task_id()-4,item_id);

        task_sleep (1);
    }

    task_exit (0) ;
}

int main (int argc, char *argv[])
{
    int i ;
    task_t tk_produtores[3];
    task_t tk_consumidores[2];
    fila = NULL;

    printf ("Main INICIO\n") ;

    pingpong_init () ;

    sem_create (&s_vaga, 5) ;
    sem_create (&s_buffer, 1) ;
    sem_create (&s_item,0);

    task_create (&tk_produtores[0], produtorBody, "Task") ;
    task_create (&tk_produtores[1], produtorBody, "Task") ;
    task_create (&tk_produtores[2], produtorBody, "Task") ;
    task_create (&tk_consumidores[0], consumidorBody, "Task") ;
    task_create (&tk_consumidores[1], consumidorBody, "Task") ;
    

    task_join (&tk_produtores[0]) ;
    task_join (&tk_produtores[1]) ;
    task_join (&tk_produtores[2]) ;
    task_join (&tk_consumidores[0]) ;
    task_join (&tk_consumidores[1]) ;

    printf ("Main FIM\n") ;
    task_exit (0) ;

    exit (0) ;
}
