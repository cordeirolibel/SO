# PingPongOS

Este projeto visa construir, de forma incremental, um pequeno sistema operacional didático. O sistema é construído 
inicialmente na forma de uma biblioteca de threads cooperativas dentro de um processo do sistema operacional real
(Linux, MacOS ou outro Unix).

O desenvolvimento é incremental, adicionando gradativamente funcionalidades como preempção, contabilização,
semáforos, filas de mensagens e acesso a um disco virtual. Essa abordagem simplifica o desenvolvimento e depuração
do núcleo, além de dispensar o uso de linguagem de máquina.

A estrutura geral do código a ser desenvolvido é apresentada na figura abaixo. Os arquivos em cinza são fixos
(fornecidos pelo professor), enquanto os arquivos em verde devem ser desenvolvidos pelos alunos.

![](http://wiki.inf.ufpr.br/maziero/lib/exe/fetch.php?cache=&media=so:ppos.png)

-------------------------------

__Os sub-projetos desenvolvidos são:__
- Biblioteca de filas (warm-up)
- Trocas de contexto
- Gestão de tarefas
- Dispatcher
- Escalonador por prioridades
- Preempção por tempo
- Contabilização
- Tarefa main
- Operador Join
- Sleeping
- Semáforos
- Uso de semáforos
- Operador Barreira
- Filas de mensagens

Esse projeto foi proposto pelo professor [__Carlos A. Maziero__](http://wiki.inf.ufpr.br/maziero/doku.php?id=so:pingpongos).
