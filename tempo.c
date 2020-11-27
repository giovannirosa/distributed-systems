/* Autor: Giovanni Rosa, Última modifição: 26/11/2020
  Nosso primeiro programa de simulação de Sistemas Distribuídos
  Vamos simular N nodos, cada um conta o "tempo" independentemente */

#include "smpl.h"
#include <stdio.h>
#include <stdlib.h>

// Eventos
#define TEST 1
#define FAULT 2
#define RECOVERY 3

// Descritor do processo

typedef struct {
  int id; // identificador de facility do SMPL
          // outras variáveis...
} ProcessType;

ProcessType *process;

int main(int argc, char *argv[]) {
  static int N, // número de processos
      token,    // processo que está executando
      event, r, i;

  static char fa_name[5];

  if (argc != 2) {
    puts("Uso correto: tempo <número de processos>");
    exit(1);
  }

  N = atoi(argv[1]);

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
  while (time() < 150.0) {
    cause(&event, &token);
    switch (event) {
    case TEST:
      if (status(process[token].id) != 0)
        break; // se processo falho, nao testa
      printf("Processo %d vai testar no tempo %4.1f\n", token, time());
      schedule(TEST, 30.0, token);
      break;
    case FAULT:
      r = request(process[token].id, token, 0);
      if (r != 0) {
        printf("Nao foi possivel falhar o processo %d", token);
        exit(1);
      } else {
        printf("Processo %d falhou no tempo %4.1f\n", token, time());
      }
      break;
    case RECOVERY:
      release(process[token].id, token);
      printf("Processo %d recuperou no tempo %4.1f\n", token, time());
      schedule(TEST, 30.0, token);
      break;
    }
  }
}

/*
  Tarefa 1: fazer cada um dos processos testar o seguinte no anel
            testar com a funçao status do SMPL e imprimir o resultado de cada
  teste por exemplo: o processo 1 testou o processo 2 correto

  Tarefa 2: cada
  processo correto executa testes até achar outro processo correto. Imprimir os
  testes e resultados.

  Tarefa 3: cada processo mantém localmente o vetor
  State[N] Inicializa o State[N] com -1 para as N entradas indicando "unknown"
            Atualiza as entradas correspondentes do State[] ao testar

  Tarefa 4: quando um processo correto testa outro processo correto obtem as
  informaçoes de diagnostico do processo testado sobre todos os processos do
  sistenma exceto aqueles que testou nesta rodada
*/