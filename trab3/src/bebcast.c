/*
  Autor: Giovanni Rosa
  Ultima modificao: 09/03/2021

Programa dedicado a implementacao do Best-Effort Broadcast sobre o VCube como
requisito para o Trabalho Pratico 3 da disciplina de Sistemas Distribuidos.

Trabalho Pratico 3: Implemente o algoritmo de Best-Effort Broadcast sobre o
VCube. O programa deve receber como entradas: o processo fonte do broadcast, o
numero de nodos do sistema e uma lista de nodos para falhar durante a execucao.
Deve ser possivel tanto definir processos que estarao falhos durante toda a
execucao, como processos que falham em momentos especificos.

Alem disso, deve ser possivel fazer uma execucao com escalonamento de falhas
definidas aleatoriamente. Em outras palavras: quando vc executa o algoritmo
usando esta abordagem, processos escolhidos aleatoriamente vao falhar entre o
inicio e o fim do broadcast.

No relatorio, explique cuidadosamente como vc implementou cada funcao e o
algoritmo como um todo. Mostre os logs com cuidado, de forma que seja possivel
ver com clareza o progresso do broadcast, inclusive apos a ocorrencia de falhas.
Um caso que deve estar nos logs e o seguinte: um processo aguarda o ACK de outro
processo. Em seguida (antes de receber o ACK) o processo que deveria mandar o
ACK falha. Agora o processo que aguardava o ACK deve mandar uma nova mensagem
para outro processo do cluster do processo que falhou. Deve ser facil visualizar
toda esta situacao no seu log!
*/

#include "bebcast.h"

int main(int argc, char *argv[]) {
  static int N,        // numero de processos
      source,          // fonte do broadcast
      token,           // processo que esta executando
      e, r, t, token2; // variaveis auxiliares

  char *faults; // lista de falhas

  const char *t_result;

  node_set *nodes;

  // inicializacao da lista de eventos
  event_array = init_array();

  // processa entradas do usuario
  user_input(&source, &N, faults, argc, argv);

  smpl(0, "Simulacao do Trabalho Pratico 3");
  reset();
  stream(1);

  // inicializa processos
  init_process(N);

  // escalonamento inicial de eventos
  // intervalo de testes de 30 unidades de tempo
  // simulacao comeca no tempo zero e escalonar o primeiro teste de todos os
  // processos no tempo 30
  // schedule_events(N);

  // calcula valor de logN
  int logN = ceil(log2(N));

  // loop principal do simulador
  // while (time() < MAX_TIME) {
  //   cause(&e, &token);
  //   // verifica novo round
  //   count_round(N, logN);
  //   // verifica se um evento foi agendado para processo inexistente
  //   if (token > N - 1) {
  //     printf("Um evento foi agendado para o processo %d, mas o maximo de "
  //            "processos e de %d!\n",
  //            token, N);
  //     exit(1);
  //   }
  //   switch (e) {
  //   case TEST:
  //     if (status(process[token].id) != 0)
  //       break; // se processo falho, nao testa
  //     printf("\n==========================================\n");
  //     printf("Iniciando testes do processo %d\n", token);
  //     print_state(N, token, 0);

  //     // calcula os processos do cluster a ser testado
  //     nodes = cis(token, process[token].cluster);
  //     print_cluster(nodes->nodes, token, nodes->size);

  //     bool tested = false;
  //     // verifica processos do cluster
  //     for (int j = 0; j < nodes->size; ++j) {
  //       // tratamento para numero ímpar de processos
  //       if (nodes->nodes[j] >= N) {
  //         continue;
  //       }

  //       // verifica se é testador do processo j no cluster
  //       token2 = verify_tester(nodes->nodes[j], process[token].cluster,
  //       token); if (token2 != -1) {
  //         tested = true;

  //         // verifica estado do processo
  //         t = status(process[token2].id);
  //         t_result = t % 2 == 0 ? "correto" : "falho";
  //         printf("Processo %d testou processo %d no tempo %4.1f: %s\n",
  //         token,
  //                token2, time(), t_result);

  //         // se estado mudou, atualiza vetor de estados
  //         if ((t == 0 && process[token].state[token2] % 2 != 0) ||
  //             (t == 1 && process[token].state[token2] % 2 != 1)) {
  //           if (process[token].state[token2] == -1) {
  //             process[token].state[token2] = t;
  //           } else {
  //             ++process[token].state[token2];
  //           }
  //           printf("State[%d] atualizado para %d\n", token2,
  //                  process[token].state[token2]);
  //           // incrementa # testes do evento
  //           count_event_test(N, token, token2);
  //           // analisa descoberta do evento para definir diagnostico
  //           count_event_discovery(N, token, token2,
  //                                 process[token].state[token2]);
  //         }

  //         // se par esta sem falha, verifica novidades
  //         if (t % 2 == 0) {
  //           check_state(N, token, token2);
  //         }
  //       }
  //     }
  //     if (!tested) {
  //       puts("Nenhum processo testado");
  //     }
  //     set_free(nodes);              // libera memoria
  //     schedule(TEST, 30.0, token);  // agenda proximo teste
  //     print_state(N, token, 1);     // imprime vetor de estados
  //     process[token].tested = true; // teste concluido na rodada
  //     count_cluster(token, logN);   // calcula proximo cluster
  //     printf("==========================================\n");
  //     break;
  //   case FAULT:
  //     // se o evento anterior nao foi diagnosticado ainda, adia o evento para
  //     a
  //     // proxima rodada
  //     if (event != NULL && !event->diag) {
  //       delay_event(FAULT, token);
  //       break;
  //     }

  //     // cria evento de falha
  //     create_event(N, 1, token);
  //     r = request(process[token].id, token, 0);
  //     if (r != 0) {
  //       printf("\nNao foi possivel falhar o processo %d\n", token);
  //       exit(1);
  //     } else {
  //       printf("\n--> Event[%d]: Processo %d falhou no tempo %4.1f\n",
  //              event->id, token, time());
  //     }
  //     break;
  //   case RECOVERY:
  //     // se o evento anterior nao foi diagnosticado ainda, adia o evento para
  //     a
  //     // proxima rodada
  //     if (event != NULL && !event->diag) {
  //       delay_event(RECOVERY, token);
  //       break;
  //     }

  //     // cria evento de recuperacao
  //     create_event(N, 0, token);
  //     release(process[token].id, token);
  //     printf("\n--> Event[%d]: Processo %d recuperou no tempo %4.1f\n",
  //            event->id, token, time());
  //     schedule(TEST, 30.0, token);
  //     break;
  //   }
  // }

  puts("\n******************************************");
  puts("Simulacao encerrada");

  puts("\n==========================================");

  // imprime eventos da simulacao
  print_event_array();

  // libera memoria
  free_array(event_array);
  free(process);

  puts("\n==========================================");
  puts("Programa finalizado com sucesso");
  puts("Autor: Giovanni Rosa :)");
  puts("==========================================");
}

void user_input(int *source, int *N, char *faults, int argc, char *argv[]) {
  if (argc != 4) {
    puts("Uso correto: bebcast <fonte do broadcast> <numero de processos> "
         "<lista de falhas>");
    exit(1);
  }

  *source = atoi(argv[1]);
  *N = atoi(argv[2]);
  faults = argv[3];
  printf("source = %i, N = %i, faults = %s\n", *source, *N, faults);
  if (*N < 2) {
    printf("O numero minimo de processos e 2!\n");
    exit(1);
  } else {
    printf("Este programa foi executado para N=%d processos\n", *N);
    printf("O tempo maximo de simulacao e de %d\n", MAX_TIME);
  }
}