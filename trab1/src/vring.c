/*
  Autor: Giovanni Rosa
  Ultima modificao: 15/12/2020

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
        if (t == 0 && process[token].state[token2] % 2 != 0 ||
            t == 1 && process[token].state[token2] % 2 != 1) {
          ++process[token].state[token2];
          printf("State[%d] atualizado para %d\n", token2,
                 process[token].state[token2]);
          count_event_test(N, token, token2);
          count_event_discovery(N, token, token2);
        }
        if (t % 2 == 0) { // se par esta sem falha, verifica novidades
          check_state(N, token, token2);
        }
      } while (t != 0);
      schedule(TEST, 30.0, token);
      print_state(N, token);
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

  puts("\n==========================================");
  puts("Programa finalizado com sucesso");
  puts("Autor: Giovanni Rosa :)");
  puts("==========================================");
}

void delay_event(int type, int token) {
  printf("\n--> O evento agendado para %2.1f foi adiado para %2.1f pois o "
         "evento anterior nao foi diagnosticado\n",
         time(), time() + 30.0);
  schedule(type, 30.0, token);
}

void print_state(int N, int token) {
  printf("State do processo %d: ", token);
  for (int i = 0; i < N; ++i) {
    printf("%d[%d] ", i, process[token].state[i]);
  }
  puts("");
}

void check_state(int N, int token, int token2) {
  printf("Atualizando state do processo %d com o state do processo %d\n", token,
         token2);
  printf("State do processo %d: ", token2);
  for (int i = 0; i < N; ++i) {
    printf("%d[%d] ", i, process[token2].state[i]);
  }
  puts("");
  bool transfered = false;
  for (int i = (token2 + 1) % N;; i = (i + 1) % N) {
    if (i == token)
      break;
    if (process[token2].state[i] > process[token].state[i]) {
      transfered = true;
      printf("Novidade encontrada, transferindo state[%d]...\n", i);
      printf("State[%d] atualizado para %d\n", i, process[token2].state[i]);
      process[token].state[i] = process[token2].state[i];
      count_event_discovery(N, token, i);
    }
  }
  if (!transfered) {
    puts("Nenhuma transferencia realizada");
  }
}

void count_event_discovery(int N, int token, int token2) {
  // if (event != NULL) {
  //   printf("Event[%d] = %d %d %d\n", event->id, event->latency, event->proc,
  //          token2);
  // }
  if (event != NULL && event->latency == 0 && event->proc == token2) {
    printf("Event[%d] descoberto pelo processo %d\n", event->id, token);
    // checa se todos os processos sem falha descobriram o evento
    bool diag = true;
    for (int i = 0; i < N; ++i) {
      if (process[i].state[token2] != -1 &&
          process[i].state[token2] != process[token2].state[token2]) {
        diag = false;
        break;
      }
    }
    if (diag) {
      printf("--> Diagnostico do evento %d completo\n", event->id);
      event->diag = true;
      event->latency = sim_round - event->e_round;
    }
  }
}

void count_event_test(int N, int token, int token2) {
  if (event != NULL && event->latency == 0 && event->proc == token2) {
    printf("Event[%d] testado pelo processo %d\n", event->id, token);
    ++event->n_tests;
  }
}

void print_event_array() {
  puts("");
  puts("Eventos durante a simulacao:");
  const char *type_str;
  for (Node aux = event_array->ini; aux; aux = aux->next) {
    event = (Event *)aux->cont;
    type_str = event->type == 0 ? "RECUP" : "FALHA";
    printf("Event[%d]: %s | round %d | tempo %05.1f | processo %d | numero de "
           "testes de %d | latencia de %d\n",
           event->id, type_str, event->e_round, event->e_time, event->proc,
           event->n_tests, event->latency);
  }
}

void create_event(int N, int type, int token) {
  event = (Event *)malloc(sizeof(Event));
  event->id = ++id_cont;
  event->e_round = sim_round;
  event->e_time = time();
  event->proc = token;
  event->type = type;
  event->n_tests = 0;
  event->diag = false;
  event->latency = 0;
  insert_array(event_array, event);

  // se falha, reseta o vetor de estados pois é falha crash
  if (type == 1) {
    for (int i = (token + 1) % N;; i = (i + 1) % N) {
      if (i == token)
        break;
      process[token].state[i] = -1;
    }
  }

  // atualiza o proprio state, apenas para controle do diagnostico, esse valor
  // nao e usado para transferencia
  ++process[token].state[token];
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
  // schedule(FAULT, 119.0, 3);
  // schedule(RECOVERY, 189.0, 3);
}