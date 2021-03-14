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
      N_faults,        // numero de falhas
      token,           // processo que esta executando
      e, r, t, token2; // variaveis auxiliares

  const char *t_result;

  node_set *nodes;

  // inicializacao da lista de eventos
  event_array = init_array();

  // processa entradas do usuario
  user_input(&N, &N_faults, argc, argv);

  smpl(0, "Simulacao do Trabalho Pratico 3");
  reset();
  stream(1);

  // inicializa processos
  init_process(N);
  init_difusion(N);

  // Use current time as seed for random generator
  // double seed = 123;
  // printf("random seed = %lf\n", seed);
  // srand(seed);

  // escalonamento inicial de eventos
  // intervalo de testes de 30 unidades de tempo
  // simulacao comeca no tempo zero e escalonar o primeiro teste de todos os
  // processos no tempo 30
  schedule_events(N, N_faults);

  // calcula valor de logN
  int logN = ceil(log2(N));

  // printf("FFneighbor(0,1) = %d, FFneighbor(0,2) = %d, FFneighbor(0,3) =
  // %d\n", FFneighbor(0,1,0), FFneighbor(0,2,0), FFneighbor(0,3,0));
  // printf("FFneighbor(1,1) = %d, FFneighbor(1,2) = %d, FFneighbor(1,3) =
  // %d\n", FFneighbor(1,1,1), FFneighbor(1,2,1), FFneighbor(1,3,1));
  // printf("FFneighbor(3,1) = %d, FFneighbor(3,2) = %d, FFneighbor(3,3) =
  // %d\n", FFneighbor(3,1,3), FFneighbor(3,2,3), FFneighbor(3,3,3));
  // printf("FFneighbor(6,1) = %d, FFneighbor(6,2) = %d, FFneighbor(6,3) =
  // %d\n", FFneighbor(6,1,6), FFneighbor(6,2,6), FFneighbor(6,3,6));
  // printf("FFneighbor(7,1) = %d, FFneighbor(7,2) = %d, FFneighbor(7,3) =
  // %d\n", FFneighbor(7,1,7), FFneighbor(7,2,7), FFneighbor(7,3,7));

  bebcast(source,logN);

  // loop principal do simulador
  while (time() < MAX_TIME) {
    cause(&e, &token);
    // verifica novo round
    count_round(N, logN);
    // verifica se um evento foi agendado para processo inexistente
    if (token > N - 1) {
      printf("Um evento foi agendado para o processo %d, mas o maximo de "
             "processos e de %d!\n",
             token, N);
      exit(1);
    }
    switch (e) {
    case TEST:
      test(token, N, logN);
      break;
    case FAULT:
      failure(token, N);
      break;
    case RECOVERY:
      recovery(token, N);
      break;
    case RECEIVE_MSG:
      receive_msg(token);
      break;
    case RECEIVE_ACK:
      receive_ACK(token, N);
      break;
    }
  }

  puts("\n******************************************");
  puts("Simulacao encerrada");

  puts("\n==========================================");

  // imprime eventos da simulacao
  print_event_array();

  // libera memoria
  free_array(event_array);
  free(process);
  free(fault);

  puts("\n==========================================");
  puts("Programa finalizado com sucesso");
  puts("Autor: Giovanni Rosa :)");
  puts("==========================================");
}

void bebcast(int source, int logN) {
  // apenas um bebcast por vez
  if (!is_delivered(source)) {
    deliver(source);
    for (int s = 1; s <= logN; ++s) {
      int first = FFneighbor(source, s, source);
      send_msg(source, s, first);
      difusion[source].pendingACK[first] = true;
    }
  }
}

void send_msg(int sender, int s, int receiver) {
  difusion[receiver].s = s;
  difusion[receiver].sender = sender;
  printf("Mensagem enviada do processo %d para o processo %d\n", sender, receiver);
  schedule(RECEIVE_MSG, 10.0, receiver);
}

void send_ACK(int sender, int receiver) {
  difusion[receiver].sender = sender;
  printf("Mensagem enviada do processo %d para o processo %d\n", sender, receiver);
  schedule(RECEIVE_ACK, 10.0, receiver);
}

void receive_msg(int token) {
  if (!is_delivered(token)) {
    deliver(token);
  }
  if (is_correct(token, source) && is_correct(token, difusion[token].sender)) {
    printf("Mensagem recebida pelo processo %d do processo %d com cluster %d\n", token, difusion[token].sender, difusion[token].s);
    send_ACK(token, difusion[token].sender);
    int s = difusion[token].s;
    while (--s != 0) {
      int first = FFneighbor(token, s, token);
      send_msg(token, s, first);
      difusion[token].pendingACK[first] = true;
    }
  }
}

void receive_ACK(int token, int N) {
  printf("ACK recebido pelo processo %d do processo %d\n", token, difusion[token].sender);
  difusion[token].pendingACK[difusion[token].sender] = false;
  if (difusion[token].sender != -1 && !any_pending(token, N)) {
    send_ACK(token, difusion[token].sender);
  }
}

bool is_delivered(int token) { return difusion[token].delivered; }

bool is_pendingACK(int token, int j) { return difusion[token].pendingACK[j]; }

bool any_pending(int token, int N) {
  for (int i = 0; i < N; ++i) {
    if (difusion[token].pendingACK[i]) {
      return true;
    }
  }
  return false;
}

void deliver(int token) {
  difusion[token].delivered = true;
  printf("Mensagem entregue pelo processo %d\n", token);
}

void user_input(int *N, int *N_faults, int argc, char *argv[]) {
  if (argc != 3 && argc != 4) {
    puts("Uso correto: bebcast <fonte do broadcast> <numero de processos> "
         "<lista de falhas opcional (#,#:#)>");
    exit(1);
  }

  source = atoi(argv[1]);
  *N = atoi(argv[2]);
  if (argc == 4) {
    printf("source = %i, N = %i, faults = %s\n", source, *N, argv[3]);
    build_faults(argv[3], *N, N_faults);
  } else {
    printf("source = %i, N = %i\n", source, *N);
  }

  if (source >= *N) {
    printf("O processo fonte %d (F) deve estar dentro do numero de processos "
           "%d (N)! [F < N]\n",
           source, *N);
    exit(1);
  } else if (*N < 2) {
    printf("O numero minimo de processos e 2!\n");
    exit(1);
  } else if (argc == 4 && fault == NULL) {
    printf("A lista de falhas deve seguir o formato: <processo>:<tempo da "
           "falha opcional>\n\t\t\t\tex.: 0:30,2:60,5,7:120\n");
    exit(1);
  } else {
    printf("Este programa foi executado para N=%d processos\n", *N);
    printf("O tempo maximo de simulacao e de %d\n", MAX_TIME);
  }
}

void build_faults(char *faults, int N, int *N_faults) {
  int ent = 0;
  *N_faults = occurrences(faults, ',') + 1;
  char **semiresult = malloc(sizeof(char *) * *N_faults);
  char *token = strtok(faults, ",");
  while (token != NULL) {
    semiresult[ent] = malloc(strlen(token) + 1);
    strcpy(semiresult[ent++], token);
    token = strtok(NULL, ",");
  }
  free(token);

  fault = malloc(sizeof(Fault) * *N_faults);
  for (int i = 0; i < *N_faults; i++) {
    int number_tokens = occurrences(semiresult[i], ':') + 1;
    if (number_tokens == 1) {
      fault[i].id = atoi(semiresult[i]);
      fault[i].failed = false;
    } else {
      char *token2 = strtok(semiresult[i], ":");
      ent = 0;
      while (token2 != NULL) {
        if (ent++ == 0) {
          fault[i].id = atoi(token2);
        } else {
          fault[i].failed = atoi(token2) == 1;
        }
        token2 = strtok(NULL, ":");
      }
    }
    free(semiresult[i]);
    if (fault[i].id >= N) {
      printf("O processo %d (F) da lista de falhas deve estar dentro do numero "
             "de processos %d (N)! [F < N]\n",
             fault[i].id, N);
      exit(1);
    }
  }
  free(semiresult);
}

int occurrences(char *str, char del) {
  int len = strlen(str);
  int c = 0;
  for (int i = 0; i < len; ++i) {
    if (str[i] == del) {
      c++;
    }
  }
  return c;
}

void schedule_events(int N, int N_faults) {
  // escalonamento inicial
  for (int i = 0; i < N; ++i) {
    schedule(TEST, 30.0, i);
  }
  for (int i = 0; i < N; i++) {
    for (int j = 0; j < N; ++j) {
      process[i].state[j] = 0;
    }
  }

  int init = 60;
  for (int i = 0; i < N_faults; i++) {
    printf("id = %d, failed = %s\n", fault[i].id,
           fault[i].failed ? "true" : "false");
    if (fault[i].failed) {
      for (int j = 0; j < N; ++j) {
        process[j].state[fault[i].id] = 1;
      }
    } else {
      schedule(FAULT, init, fault[i].id);
      init += 30;
    }
  }

  for (int i = 0; i < N; ++i) {
    print_state(N, i, -1);
  }

  // for (int i = 0; i < N_faults; i++) {
  // printf("id = %d, time = %d\n", fault[i].id, fault[i].time);
  // if (fault[i].time == -1) {
  //   schedule(FAULT, gen_rand(MAX_TIME, 10) * 30, i);
  // } else if (fault[i].time == 0) {

  // } else {
  //   schedule(FAULT, fault[i].time, i);
  // }
  // }
  // schedule(FAULT, 35.0, 7);
  // schedule(RECOVERY, 130.0, 7);
  // schedule(RECOVERY, 170.0, 1);
  // schedule(RECOVERY, 240.0, 2);
}

void init_difusion(int N) {
  static char fa_name[5];

  difusion = (ProcessDifusion *)malloc(sizeof(ProcessDifusion) * N);
  for (int i = 0; i < N; ++i) {
    difusion[i].id = i;
    difusion[i].sender = -1;
    difusion[i].s = -1;
    difusion[i].delivered = false;
    difusion[i].pendingACK = malloc(sizeof(bool) * N);
    for (int j = 0; j < N; ++j) {
      difusion[i].pendingACK[j] = i == j ? 0 : -1;
    }
  }
}

bool search_fault_failed(int p, int N_faults) {
  for (int i = 0; i < N_faults; i++) {
    if (fault[i].id == p) {
      return fault[i].failed;
    }
  }
  return false;
}

int gen_rand(int upper, int lower) {
  return (rand() % (upper - lower + 1)) + lower;
}