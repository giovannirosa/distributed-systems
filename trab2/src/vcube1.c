/*
  Autor: Giovanni Rosa
  Ultima modificao: 11/01/2021

Programa dedicado a implementacao do VCube (versao 1) como requisito para o
Trabalho Pratico 2 da disciplina de Sistemas Distribuidos.

Trabalho Pratico 2: Implemente as duas versões do VCube usando SMPL. Lembre-se:
na versão 1, em cada intervalo de testes, cada processo correto executa testes
em 1 cluster sequencialmente até encontrar um processo correto, ou testar todos
os processos falhos. Tendo testado um processo correto, obtém informações sobre
os processos restantes do cluster (aqueles que não testou) a partir do processo
correto testado. Na versão 2, são definidos de antemão com o uso da função
C(i,s) executada por e para todos os processos quem são os testadores de quais
processos. Esta estratégia garante NlogN testes a cada logN rodadas de testes.
Na versão 2, ao testar um processo correto, o testador obtém informações sobre
qualquer “novidade” que o processo testado tenha. Sugestão para implementar na
simulação (em um sistema real não seria boa ideia): simplesmente compare os
vetores State do testador com o testado procurando entradas com maior valor.

Vamos manter a mesma definição de rodada para as duas versões: todos os
processos corretos executaram testes em 1 de seus clusters.

Mostre resultados para diversos valores de N e diversos eventos - um evento em
um processo de cada vez, um evento só ocorre depois do evento anterior ser
diagnosticado. Para cada evento mostre claramente o número de testes executados
e a latência para completar o diagnóstico do evento. Cada nodo mantém o vetor
STATE[0..N-1] de contadores de eventos, inicializado em -1 (estado “unknown”).
Assume-se que os processos são inicializados sem-falha, a entrada correspondente
ao próprio processo no vetor STATE[] do testador é setada para zero. Ao
descobrir um novo evento em um processo testado, o testador incrementa a entrada
correspondente no vetor STATE[].
*/

#include "vcube1.h"

int main(int argc, char *argv[]) {
  static int N,              // numero de processos
      token,                 // processo que esta executando
      e, r, i, j, t, token2; // variaveis auxiliares

  const char *t_result;

  // inicializacao da lista de eventos
  event_array = init_array();

  // processar entradas do usuario
  user_input(&N, argc, argv);

  smpl(0, "Simulacao do Trabalho Pratico 1");
  reset();
  stream(1);

  // inicializar processos
  init_process(N);

  // escalonamento inicial de eventos
  // intervalo de testes de 30 unidades de tempo
  // simulacao comeca no tempo zero e escalonar o primeiro teste de todos os
  // processos no tempo 30
  schedule_events(N);

  // loop principal do simulador
  while (time() < MAX_TIME) {
    cause(&e, &token);
    count_round(N);
    if (token > N - 1) {
      printf("Um evento foi agendado para o processo %d, mas o maximo de "
             "processos e de %d!\n",
             token, N);
      exit(1);
    }
    switch (e) {
    case TEST:
      if (status(process[token].id) != 0)
        break; // se processo falho, nao testa
      token2 = token;
      printf("\n==========================================\n");
      printf("Iniciando testes do processo %d\n", token);
      print_state(N, token);
      do {
        token2 = (token2 + 1) % N;
        if (token2 == token) {
          printf("Todos os demais processos estao falhos!\n");
          break;
        }
        t = status(process[token2].id);
        t_result = t % 2 == 0 ? "correto" : "falho";
        printf("Processo %d testou processo %d no tempo %4.1f: %s\n", token,
               token2, time(), t_result);
        if ((t == 0 && process[token].state[token2] % 2 != 0) ||
            (t == 1 && process[token].state[token2] % 2 != 1)) {
          if (process[token].state[token2] == -1) {
            process[token].state[token2] = t;
          } else {
            ++process[token].state[token2];
          }
          printf("State[%d] atualizado para %d\n", token2,
                 process[token].state[token2]);
          count_event_test(N, token, token2);
          count_event_discovery(N, token, token2, process[token].state[token2]);
        }
        if (t % 2 == 0) { // se par esta sem falha, verifica novidades
          check_state(N, token, token2);
        }
      } while (t != 0);
      schedule(TEST, 30.0, token);
      print_state(N, token);
      process[token].tested = true;
      printf("==========================================\n");
      break;
    case FAULT:
      // se o evento anterior nao foi diagnosticado ainda, adia o evento para a
      // proxima rodada
      if (event != NULL && !event->diag) {
        delay_event(FAULT, token);
        break;
      }
      create_event(N, 1, token);
      r = request(process[token].id, token, 0);
      if (r != 0) {
        printf("\nNao foi possivel falhar o processo %d\n", token);
        exit(1);
      } else {
        printf("\n--> Event[%d]: Processo %d falhou no tempo %4.1f\n",
               event->id, token, time());
      }
      break;
    case RECOVERY:
      // se o evento anterior nao foi diagnosticado ainda, adia o evento para a
      // proxima rodada
      if (event != NULL && !event->diag) {
        delay_event(RECOVERY, token);
        break;
      }
      create_event(N, 0, token);
      release(process[token].id, token);
      printf("\n--> Event[%d]: Processo %d recuperou no tempo %4.1f\n",
             event->id, token, time());
      schedule(TEST, 30.0, token);
      break;
    }
  }

  puts("\n******************************************");
  puts("Simulacao encerrada");

  puts("\n==========================================");

  print_event_array();
  free_array(event_array);
  free(process);

  puts("\n==========================================");
  puts("Programa finalizado com sucesso");
  puts("Autor: Giovanni Rosa :)");
  puts("==========================================");
}