/*
  Autor: Giovanni Rosa
  Ultima modificao: 31/01/2021

Programa dedicado a implementacao do VCube (versao 2) como requisito para o
Trabalho Pratico 2 da disciplina de Sistemas Distribuidos.

Trabalho Pratico 2: Implemente as duas versoes do VCube usando SMPL. Lembre-se:
na versao 1, em cada intervalo de testes, cada processo correto executa testes
em 1 cluster sequencialmente ate encontrar um processo correto, ou testar todos
os processos falhos. Tendo testado um processo correto, obtem informacoes sobre
os processos restantes do cluster (aqueles que nao testou) a partir do processo
correto testado. Na versao 2, sao definidos de antemao com o uso da funcao
C(i,s) executada por e para todos os processos quem sao os testadores de quais
processos. Esta estrategia garante NlogN testes a cada logN rodadas de testes.
Na versao 2, ao testar um processo correto, o testador obtem informacoes sobre
qualquer "novidade" que o processo testado tenha. Sugestao para implementar na
simulacao (em um sistema real nao seria boa ideia): simplesmente compare os
vetores State do testador com o testado procurando entradas com maior valor.

Vamos manter a mesma definicao de rodada para as duas versoes: todos os
processos corretos executaram testes em 1 de seus clusters.

Mostre resultados para diversos valores de N e diversos eventos - um evento em
um processo de cada vez, um evento so ocorre depois do evento anterior ser
diagnosticado. Para cada evento mostre claramente o numero de testes executados
e a latencia para completar o diagnostico do evento. Cada nodo mantem o vetor
STATE[0..N-1] de contadores de eventos, inicializado em -1 (estado "unknown").
Assume-se que os processos sao inicializados sem-falha, a entrada correspondente
ao proprio processo no vetor STATE[] do testador e setada para zero. Ao
descobrir um novo evento em um processo testado, o testador incrementa a entrada
correspondente no vetor STATE[].
*/

#include "vcube2.h"

int verify_tester(int j, int s, int token) {
  int to_test = -1;
  // calcula cluster do processo j
  node_set *nodes_j = cis(j, s);
  // verifica se o processo (token) é o primeiro sem falha no cluster do
  // processo j
  for (int k = 0; k < nodes_j->size; ++k) {
    if (process[token].state[nodes_j->nodes[k]] % 2 == 0) {
      if (nodes_j->nodes[k] == token) {
        // se sim, sera o testador no cluster s da rodada atual
        to_test = j;
      }
      break;
    }
  }
  set_free(nodes_j);
  return to_test;
}

void count_cluster(int token, int logN) {
  if (process[token].cluster == logN) {
    process[token].cluster = 1;
  } else {
    ++process[token].cluster;
  }
}

void count_round(int N, int logN) {
  bool all_tested = true;
  // verifica se todos os processos ja testaram na rodada
  for (int i = 0; i < N; ++i) {
    if (process[i].state[i] % 2 == 0 && !process[i].tested) {
      all_tested = false;
      break;
    }
  }

  // se sim, inicia novo round
  if (all_tested) {
    ++sim_round;
    puts("\n******************************************");
    printf("Iniciando round de testes %d\n", sim_round);
    for (int i = 0; i < N; ++i) {
      process[i].tested = false;
    }
  }
}

void delay_event(int type, int token) {
  printf("\n--> O evento agendado para %2.1f foi adiado para %2.1f pois o "
         "evento anterior nao foi diagnosticado\n",
         time(), time() + 30.0);
  schedule(type, 30.0, token);
}

void print_cluster(int nodes[], int token, int size) {
  printf("Verificando cluster %d: [", process[token].cluster);
  for (int i = 0; i < size; ++i) {
    printf("%i", nodes[i]);
    if (i != size - 1) {
      printf(", ");
    }
  }
  puts("]");
}

void print_state(int N, int token, int when) {
  printf("State do processo %d", token);
  if (when == 0) {
    printf(" (antes)");
  } else if (when == 1) {
    printf(" (depois)");
  }
  printf(": [");
  for (int i = 0; i < N; ++i) {
    printf("%d", process[token].state[i]);
    if (i != N - 1) {
      printf(", ");
    }
  }
  puts("]");
}

void check_state(int N, int token, int token2) {
  print_state(N, token2, -1);
  bool transfered = false;
  // verifica quaisquer novidades, exceto do proprio processo ou do testado
  for (int i = 0; i < N; ++i) {
    if (i == token || i == token2)
      continue;
    if (process[token2].state[i] > process[token].state[i]) {
      transfered = true;
      printf("State[%d] atualizado para %d\n", i, process[token2].state[i]);
      process[token].state[i] = process[token2].state[i];
      count_event_discovery(N, token, i, process[token2].state[i]);
    }
  }
  if (!transfered) {
    puts("Nenhuma transferencia realizada");
  }
}

void count_event_discovery(int N, int token, int token2, int state) {
  if (event != NULL && event->latency == -1 && event->proc == token2 &&
      process[token2].state[token2] == state) {
    printf("Event[%d] descoberto pelo processo %d\n", event->id, token);
    // checa se todos os processos sem falha descobriram o evento
    bool diag = true;
    for (int i = 0; i < N; ++i) {
      if (process[i].state[i] % 2 == 0 &&
          process[i].state[token2] != process[token2].state[token2]) {
        diag = false;
        break;
      }
    }
    if (diag) {
      printf("--> Diagnostico do evento %d completo [# testes = %d, latência = "
             "%d]\n", event->id, event->n_tests, event->latency);
      event->diag = true;
      event->latency = sim_round - event->e_round;
    }
  }
}

void count_event_test(int N, int token, int token2) {
  if (event != NULL && event->latency == -1 && event->proc == token2) {
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
  event->latency = -1;
  insert_array(event_array, event);

  // se falha, reseta o vetor de estados e cluster, pois é falha crash
  if (type == 1) {
    for (int i = (token + 1) % N;; i = (i + 1) % N) {
      if (i == token)
        break;
      process[token].state[i] = -1;
    }
    process[token].cluster = 1;
  }

  // atualiza o proprio state, apenas para controle do diagnostico, esse valor
  // nao e usado para transferencias
  ++process[token].state[token];
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
    process[i].tested = true;
    process[i].cluster = 1;
  }
}