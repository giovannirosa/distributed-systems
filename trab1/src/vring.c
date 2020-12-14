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

#include "../../lib/smpl.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// Eventos
#define TEST 1
#define FAULT 2
#define RECOVERY 3

// Tempo maximo de simulacao
#define MAX_TIME 150

// Descritor do processo
typedef struct {
  int id;     // identificador de facility do SMPL
  int *state; // vetor de estados de cada processo
} ProcessType;

// Descritor do evento
typedef struct {
  int id;     // identificador de facility do SMPL
  int *state; // vetor de estados de cada processo
} Event;

ProcessType *process;

int main(int argc, char *argv[]) {
  static int N,                  // numero de processos
      token,                     // processo que esta executando
      event, r, i, j, t, token2; // variaveis auxiliares

  static char fa_name[5];
  const char *t_result;

  if (argc != 2) {
    puts("Uso correto: vring <numero de processos>");
    exit(1);
  }

  N = atoi(argv[1]);
  if (N < 2) {
    printf("O numero minimo de processos e 2!\n");
    exit(1);
  } else {
    printf("Este programa foi executado para N=%d processos\n", N);
    printf("O tempo maximo de simulacao e de %d\n", MAX_TIME);
  }

  smpl(0, "Um exemplo de simulacao");
  reset();
  stream(1);

  // inicializar processos
  process = (ProcessType *)malloc(sizeof(ProcessType) * N);
  for (i = 0; i < N; ++i) {
    memset(fa_name, '\0', 5);
    sprintf(fa_name, "%d", i);
    process[i].id = facility(fa_name, 1);
    process[i].state = malloc(sizeof(int) * N);
    for (j = 0; j < N; ++j) {
      process[i].state[j] = i == j ? 0 : -1;
    }
  }

  // escalonamento inicial de eventos
  // intervalo de testes de 10 unidades de tempo
  // simulacao comeca no tempo zero e escalonar o primeiro teste de todos os
  // processos no tempo 30
  for (i = 0; i < N; ++i) {
    schedule(TEST, 30.0, i);
  }
  schedule(FAULT, 31.0, 1);
  schedule(RECOVERY, 61.0, 1);

  // loop principal do simulador
  while (time() < MAX_TIME) {
    cause(&event, &token);
    switch (event) {
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
        } else if (t == 1 && process[token].state[token2] % 2 != 1) {
          ++process[token].state[token2];
          printf("state[%d] atualizado para %d\n", token2,
                 process[token].state[token2]);
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
              printf("state[%d] atualizado para %d\n", i, process[token2].state[i]);
              process[token].state[i] = process[token2].state[i];
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
      r = request(process[token].id, token, 0);
      if (r != 0) {
        printf("\nNao foi possivel falhar o processo %d\n", token);
        exit(1);
      } else {
        printf("\n--> Processo %d falhou no tempo %4.1f\n", token, time());
      }
      break;
    case RECOVERY:
      release(process[token].id, token);
      printf("\n--> Processo %d recuperou no tempo %4.1f\n", token, time());
      schedule(TEST, 30.0, token);
      break;
    }
  }

  puts("\n==========================================");
  puts("Programa finalizado com sucesso");
  puts("Autor: Giovanni Rosa");
  puts("==========================================");
}