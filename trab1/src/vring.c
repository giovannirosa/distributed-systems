/*
  Autor: Giovanni Rosa
  Ultima modificao: 13/12/2020

Programa dedicado a implementacao do VRing como requisito para o Trabalho
Pratico 1 da disciplina de Sistemas Distribuidos.

Trabalho Pratico 1: Implemente o algoritmo VRing no ambiente de simulacao SMPL,
e mostre resultados para diversos valores de N e diversos eventos - um evento em
um processo de cada vez, um evento so ocorre depois do evento anterior ser
diagnosticado. Para cada evento mostre claramente o numero de testes executados
e a latencia para completar o diagnostico do evento. Cada nodo mantem o vetor
STATE[0..N-1] de contadores de eventos, inicializado em -1 (estado "unknown").
Assume-se que os processos sao inicializados sem-falha, a entrada correspondente
ao proprio processo no vetor STATE[] do testador e setada para zero. Ao
descobrir um novo evento em um nodo testado, o testador incrementa a entrada
correspondente no vetor STATE[].

Para a transferencia de informacoes de diagnostico lembre-se da estrategia do
VRing: quando um processo sem-falha testa outro processo sem-falha obtem
informacoes sobre os estados de todos os processos que nao testou no intervalo
de testes corrente. E importante comparar as entradas correspondentes dos
vetores STATE (testador e testado) para saber se o testado tem alguma novidade.
Se o valor da entrada for maior no vetor STATE do processo testado, entao copia
a informacao.
*/

#include "vring.h"

int main(int argc, char *argv[]) {
  static int N,              // numero de processos
      token,                 // processo que esta executando
      e, r, i, j, t, token2; // variaveis auxiliares

  const char *t_result;

  event_array = init_array();

  // processar entradas do usuario
  user_input(&N, argc, argv);

  smpl(0, "Um exemplo de simulacao");
  reset();
  stream(1);

  // inicializar processos
  init_process(N);

  // escalonamento inicial de eventos
  // intervalo de testes de 10 unidades de tempo
  // simulacao comeca no tempo zero e escalonar o primeiro teste de todos os
  // processos no tempo 30
  schedule_events(N);

  // loop principal do simulador
  while (time() < MAX_TIME) {
    cause(&e, &token);
    if (token == 0) {
      ++sim_round;
      puts("\n******************************************");
      printf("Iniciando round de testes %d\n", sim_round);
    }
    switch (e) {
    case TEST:
      if (status(process[token].id) != 0)
        break; // se processo falho, nao testa
      token2 = token;
      printf("\n==========================================\n");
      printf("Iniciando testes do processo %d\n", token);
      printf("State do processo %d: ", token);
      for (i = 0; i < N; ++i) {
        printf("%d[%d] ", i, process[token].state[i]);
      }
      puts("");
      do {
        token2 = (token2 + 1) % N;
        if (token2 == token) {
          printf("Todos os demais processos estao falhos!\n");
          exit(1);
        }
        t = status(process[token2].id);
        t_result = t % 2 == 0 ? "correto" : "falho";
        printf("Processo %d testou processo %d no tempo %4.1f: %s\n", token,
               token2, time(), t_result);
        if (t == 0 && process[token].state[token2] % 2 != 0) {
          ++process[token].state[token2];
          printf("state[%d] atualizado para %d\n", token2,
                 process[token].state[token2]);
          count_event_test(N, token2, token);
          count_event_discovery(N, token2, token);
        } else if (t == 1 && process[token].state[token2] % 2 != 1) {
          ++process[token].state[token2];
          printf("state[%d] atualizado para %d\n", token2,
                 process[token].state[token2]);
          count_event_test(N, token2, token);
          count_event_discovery(N, token2, token);
        }
        if (t % 2 == 0) { // se par esta sem falha
          printf(
              "Atualizando state do processo %d com o state do processo %d\n",
              token, token2);
          printf("State do processo %d: ", token2);
          for (i = 0; i < N; ++i) {
            printf("%d[%d] ", i, process[token2].state[i]);
          }
          puts("");
          bool transfered = false;
          for (i = (token2 + 1) % N;; i = (i + 1) % N) {
            if (i == token2 || i == token)
              break;
            if (process[token2].state[i] > process[token].state[i]) {
              transfered = true;
              printf("Novidade encontrada, transferindo state[%d]...\n", i);
              printf("state[%d] atualizado para %d\n", i,
                     process[token2].state[i]);
              process[token].state[i] = process[token2].state[i];
              count_event_discovery(N, i, token);
            }
          }
          if (!transfered) {
            puts("Nenhuma transferencia realizada");
          }
        }
      } while (t != 0);
      schedule(TEST, 30.0, token);
      printf("State do processo %d: ", token);
      for (i = 0; i < N; ++i) {
        printf("%d[%d] ", i, process[token].state[i]);
      }
      printf("\n==========================================\n");
      break;
    case FAULT:
      create_event(1, token);
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
      create_event(0, token);
      release(process[token].id, token);
      printf("\n--> Event[%d]: Processo %d recuperou no tempo %4.1f\n",
             event->id, token, time());
      schedule(TEST, 30.0, token);
      break;
    }
  }

  print_event_array();
  free_array(event_array);

  puts("\n==========================================");
  puts("Programa finalizado com sucesso");
  puts("Autor: Giovanni Rosa");
  puts("==========================================");
}

void count_event_discovery(int N, int p, int q) {
  if (event != NULL && event->latency == 0 && event->proc == p) {
    printf("Event[%d] descoberto pelo processo %d\n", event->id, q);
    ++event->n_proc;
    if (event->n_proc == N - 1) {
      printf("Diagnostico do evento %d completo\n", event->id);
      event->latency = sim_round - event->e_round;
    }
  }
}

void count_event_test(int N, int p, int q) {
  if (event != NULL && event->latency == 0 && event->proc == p) {
    printf("Event[%d] testado pelo processo %d\n", event->id, q);
    ++event->n_tests;
  }
}

void print_event_array() {
  puts("");
  puts("Eventos durante a simulacao:");
  const char *type_str;
  for (Node aux = event_array->ini; aux; aux = aux->next) {
    event = (Event *)aux->cont;
    type_str = event->type == 0 ? "recuperacao" : "falha";
    printf("Event[%d] = %s, ocorreu no round %d no processo %d com numero de "
           "testes de %d e latencia de %d\n",
           event->id, type_str, event->e_round, event->proc, event->n_tests,
           event->latency);
  }
}

void create_event(int type, int process) {
  event = (Event *)malloc(sizeof(Event));
  event->id = ++id_cont;
  event->e_round = sim_round;
  event->proc = process;
  event->type = type;
  event->n_tests = 0;
  event->n_proc = 0;
  event->latency = 0;
  insert_array(event_array, event);
}

void user_input(int *N, int argc, char *argv[]) {
  if (argc != 2) {
    puts("Uso correto: vring <numero de processos>");
    exit(1);
  }

  *N = atoi(argv[1]);
  if (*N < 2) {
    printf("O numero minimo de processos e 2!\n");
    exit(1);
  } else {
    printf("Este programa foi executado para N=%d processos\n", *N);
    printf("O tempo maximo de simulacao e de %d\n", MAX_TIME);
  }
}

void init_process(int N) {
  static char fa_name[5];

  process = (ProcessType *)malloc(sizeof(ProcessType) * N);
  for (int i = 0; i < N; ++i) {
    memset(fa_name, '\0', 5);
    sprintf(fa_name, "%d", i);
    process[i].id = facility(fa_name, 1);
    process[i].state = malloc(sizeof(int) * N);
    for (int j = 0; j < N; ++j) {
      process[i].state[j] = i == j ? 0 : -1;
    }
  }
}

void schedule_events(int N) {
  for (int i = 0; i < N; ++i) {
    schedule(TEST, 30.0, i);
  }
  schedule(FAULT, 31.0, 1);
  schedule(RECOVERY, 61.0, 1);
}