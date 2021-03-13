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

bool test(int token, int N, int logN) {
  if (status(process[token].id) != 0)
    return false; // se processo falho, nao testa
  printf("\n==========================================\n");
  printf("Iniciando testes do processo %d\n", token);
  print_state(N, token, 0);

  // calcula os processos do cluster a ser testado
  for (int s = 1; s <= logN; ++s) {
    node_set *nodes = cis(token, s);
    print_cluster(nodes->nodes, token, nodes->size, s);

    bool tested = false;
    // verifica processos do cluster
    for (int j = 0; j < nodes->size; ++j) {
      // tratamento para numero ímpar de processos
      if (nodes->nodes[j] >= N) {
        continue;
      }

      // verifica se é testador do processo j no cluster
      int token2 = verify_tester(nodes->nodes[j], s, token);
      if (token2 != -1) {
        tested = true;

        // verifica estado do processo
        int t = status(process[token2].id);
        printf("Processo %d testou processo %d no tempo %4.1f: %s\n", token,
               token2, time(), t % 2 == 0 ? "correto" : "falho");

        // se estado mudou, atualiza vetor de estados
        if ((t == 0 && process[token].state[token2] % 2 != 0) ||
            (t == 1 && process[token].state[token2] % 2 != 1)) {
          if (process[token].state[token2] == -1) {
            process[token].state[token2] = t;
          } else {
            ++process[token].state[token2];
          }
          printf("State[%d] atualizado para %d\n", token2,
                 process[token].state[token2]);
          // incrementa # testes do evento
          count_event_test(N, token, token2);
          // analisa descoberta do evento para definir diagnostico
          count_event_discovery(N, token, token2, process[token].state[token2]);
        }

        // se par esta sem falha, verifica novidades
        if (t % 2 == 0) {
          check_state(N, token, token2);
        }
      }
    }
    if (!tested) {
      puts("Nenhum processo testado");
    }
    set_free(nodes); // libera memoria
  }
  schedule(TEST, 30.0, token);  // agenda proximo teste
  print_state(N, token, 1);     // imprime vetor de estados
  process[token].tested = true; // teste concluido na rodada
  printf("==========================================\n");
  return true;
}

bool failure(int token, int N) {
  // se o evento anterior nao foi diagnosticado ainda, adia o evento para a
  // proxima rodada
  if (event != NULL && !event->diag) {
    delay_event(FAULT, token);
    return false;
  }

  // cria evento de falha
  create_event(N, 1, token);
  int r = request(process[token].id, token, 0);
  if (r != 0) {
    printf("\nNao foi possivel falhar o processo %d\n", token);
    exit(1);
  } else {
    printf("\n--> Event[%d]: Processo %d falhou no tempo %4.1f\n", event->id,
           token, time());
  }
  return true;
}

bool recovery(int token, int N) {
  // se o evento anterior nao foi diagnosticado ainda, adia o evento para a
  // proxima rodada
  if (event != NULL && !event->diag) {
    delay_event(RECOVERY, token);
    return false;
  }

  // cria evento de recuperacao
  create_event(N, 0, token);
  release(process[token].id, token);
  printf("\n--> Event[%d]: Processo %d recuperou no tempo %4.1f\n", event->id,
         token, time());
  schedule(TEST, 30.0, token);
  return true;
}

int cluster(int i, int j, int logN) {
  for (int s = 1; s <= logN; ++s) {
    node_set *nodes_j = cis(j, s);
    for (int k = 0; k < nodes_j->size; ++k) {
      if (nodes_j->nodes[k] == i) {
        return s;
      }
    }
  }
  return 0;
}

int FFneighbor(int i, int s, int token) {
  int first = -1;
  node_set *nodes_j = cis(i, s);
  for (int k = 0; k < nodes_j->size; ++k) {
    if (process[token].state[nodes_j->nodes[k]] % 2 == 0) {
      first = nodes_j->nodes[k];
      break;
    }
  }
  set_free(nodes_j);
  return first;
}

int verify_tester(int j, int s, int token) {
  int to_test = -1;
  // verifica se o processo (token) é o primeiro sem falha no cluster do
  // processo j
  int first = FFneighbor(j, s, token);
  if (first == token) {
    to_test = j;
  }

  return to_test;
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

void print_cluster(int nodes[], int token, int size, int s) {
  printf("Verificando cluster %d: [", s);
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
             "%d]\n",
             event->id, event->n_tests, event->latency);
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
  }
}

bool is_correct(int token, int j) {
  process[token].state[j] % 2 == 0;
}