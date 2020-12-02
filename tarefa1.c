/* Autor: Giovanni Rosa, Última modifição: 01/12/2020
  Nosso primeiro programa de simulação de Sistemas Distribuídos
  Vamos simular N nodos, cada um conta o "tempo" independentemente

  Tarefa 1: Fazer cada um dos processos testar o seguinte no anel.
            Implemente o teste com a função status() do SMPL e imprimir
            (printf) o resultado de cada teste executado. Por exemplo:
            “O processo i testou o processo j correto no tempo tal.”
*/

#include "smpl.h"
#include <stdio.h>
#include <stdlib.h>

// Eventos
#define TEST 1
#define FAULT 2
#define RECOVERY 3

// Tempo máximo de simulação
#define MAX_TIME 150

// Descritor do processo
typedef struct {
  int id; // identificador de facility do SMPL
} ProcessType;

ProcessType *process;

int main(int argc, char *argv[]) {
  static int N,               // número de processos
      token,                  // processo que está executando
      event, r, i, t, token2; // variaveis auxiliares

  static char fa_name[5];
  const char *t_result;

  if (argc != 2) {
    puts("Uso correto: tempo <número de processos>");
    exit(1);
  }

  N = atoi(argv[1]);
  if (N < 2) {
    printf("O número mínimo de processos é 2!\n", N);
    exit(1);
  } else {
    printf("Este programa foi executado para N=%d processos\n", N);
    printf("O tempo máximo de simulação é de %d\n", MAX_TIME);
  }

  smpl(0, "Um exemplo de simulação");
  reset();
  stream(1);

  // inicializar processos
  process = (ProcessType *)malloc(sizeof(ProcessType) * N);

  for (i = 0; i < N; ++i) {
    memset(fa_name, '\0', 5);
    sprintf(fa_name, "%d", i);
    process[i].id = facility(fa_name, 1);
  }

  // escalonamento inicial de eventos
  // intervalo de testes de 10 unidades de tempo
  // simulação começa no tempo zero e escalonar o primeiro teste de todos os
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
      token2 = (token2 + 1) % N;
      if (token2 == token) {
        printf("Existe apenas um processo!\n");
        exit(1);
      }
      t = status(process[token2].id);
      t_result = t == 0 ? "correto" : "falho";
      printf("Processo %d testou processo %d no tempo %4.1f: %s\n", token,
             token2, time(), t_result);
      schedule(TEST, 30.0, token);
      printf("==========================================\n");
      break;
    case FAULT:
      r = request(process[token].id, token, 0);
      if (r != 0) {
        printf("\nNao foi possivel falhar o processo %d\n", token);
        exit(1);
      } else {
        printf("\nProcesso %d falhou no tempo %4.1f\n", token, time());
      }
      break;
    case RECOVERY:
      release(process[token].id, token);
      printf("\nProcesso %d recuperou no tempo %4.1f\n", token, time());
      schedule(TEST, 30.0, token);
      break;
    }
  }
}