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

  bebcast(source, logN);

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
    puts("\n==========================================");
    const char *exec = "TEST";
    switch (e) {
    case FAULT:
      exec = "FAULT";
      break;
    case RECOVERY:
      exec = "RECOVERY";
      break;
    case RECEIVE_MSG:
      exec = "RECEIVE_MSG";
      break;
    case RECEIVE_ACK:
      exec = "RECEIVE_ACK";
      break;
    }
    printf("Executando processo %d no tempo %.0lf [%s]\n", token, time(), exec);
    switch (e) {
    case TEST:
      test(token, N, logN);
      resend_failed(token, N, logN);
      break;
    case FAULT:
      printf("Processo %d falhou\n", token);
      failure(token, N);
      reset_pending(token, N);
      break;
    case RECOVERY:
      recovery(token, N);
      break;
    case RECEIVE_MSG:
      receive_msg(token, N);
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

void calc_send(int token, int s) {
  int first = FFneighbor(token, s, token);
  if (first != -1) {
    send_msg(token, s, first);
    difusion[token].pendingACK[first] = true;
  } else {
    printf("Nao existem processos corretos no cluster %d do processo %d\n", s,
           token);
  }
}

void bebcast(int source, int logN) {
  // apenas um bebcast por vez
  if (!is_delivered(source)) {
    puts("\n******************************************");
    puts("\n==========================================");
    printf("Executando processo %d no tempo 0 [BEBCAST]\n", source);
    printf("Iniciando broadcast com origem %d\n", source);
    deliver(source);
    for (int s = 1; s <= logN; ++s) {
      calc_send(source, s);
    }
  }
}

void send_msg(int sender, int s, int receiver) {
  ProcessMessage *msg = (ProcessMessage *)malloc(sizeof(ProcessMessage));
  msg->type = 0;
  msg->s = s;
  msg->sender = sender;
  insert_array(difusion[receiver].messages, msg);
  printf("Mensagem enviada do processo %d para o processo %d com cluster %d\n",
         sender, receiver, s);
  schedule(RECEIVE_MSG, LATENCY, receiver);
}

void send_ACK(int sender, int receiver) {
  printf("O processo %d nao possui ACKs pendentes\n", sender);
  ProcessMessage *msg = (ProcessMessage *)malloc(sizeof(ProcessMessage));
  msg->type = 1;
  msg->s = -1;
  msg->sender = sender;
  insert_array(difusion[receiver].messages, msg);
  printf("ACK enviado do processo %d para o processo %d\n", sender, receiver);
  schedule(RECEIVE_ACK, LATENCY, receiver);
}

ProcessMessage *retrieve_msg(int token) {
  Node node = difusion[token].messages->ini;
  ProcessMessage *msg = node->cont;
  remove_array_spec(difusion[token].messages, node);
  return msg;
}

void receive_msg(int token, int N) {
  // Consome primeira mensagem na lista, que foi incluida primeiro
  ProcessMessage *msg = retrieve_msg(token);
  printf("Mensagem recebida pelo processo %d do processo %d com cluster %d\n",
         token, msg->sender, msg->s);
  if (!is_delivered(token)) {
    deliver(token);
  }
  if (is_correct(token, source) && is_correct(token, msg->sender)) {
    int s = msg->s;
    while (--s != 0) {
      // printf("processo %d, cluster %d\n", token, s);
      calc_send(token, s);
    }
    if (msg->sender != -1 && !any_pending(token, N)) {
      send_ACK(token, msg->sender);
    }
  }
}

void receive_ACK(int token, int N) {
  // Consome primeira mensagem na lista, que foi incluida primeiro
  ProcessMessage *msg = retrieve_msg(token);
  printf("ACK recebido pelo processo %d do processo %d\n", token, msg->sender);
  difusion[token].pendingACK[msg->sender] = false;
  print_pending(N, token);
  if (msg->sender != -1) {
    for (int i = 0; i < N; ++i) {
      if (difusion[i].pendingACK[token]) {
        send_ACK(token, i);
      }
    }
  }
  if (token == source && !any_pending(token, N)) {
    printf("Ultimo ACK recebido! Transmissao completa!\n");
  }
}

void reset_pending(int token, int N) {
  for (int i = 0; i < N; ++i) {
    difusion[token].pendingACK[i] = false;
  }
}

void resend_failed(int token, int N, int logN) {
  for (int i = 0; i < N; ++i) {
    if (!is_correct(token, i) && difusion[token].pendingACK[i]) {
      printf("%d falhou e o processo %d vai reenviar a mensagem\n", i, token);
      difusion[token].pendingACK[i] = false;
      int s = cluster(i, token, logN);
      calc_send(token, s);
    }
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
    build_faults(argv[3], *N, N_faults);
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
    printf("A latencia entre as mensagens e o intervalo de testes e de %d\n",
           LATENCY);
    printf("A origem do broadcast e o processo %d\n", source);
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
    schedule(TEST, LATENCY, i);
  }
  for (int i = 0; i < N; i++) {
    for (int j = 0; j < N; ++j) {
      process[i].state[j] = 0;
    }
  }

  int init = LATENCY * 2;
  for (int i = 0; i < N_faults; i++) {
    printf("id = %d, failed = %s\n", fault[i].id,
           fault[i].failed ? "true" : "false");
    if (fault[i].failed) {
      for (int j = 0; j < N; ++j) {
        process[j].state[fault[i].id] = 1;
      }
    } else {
      schedule(FAULT, init, fault[i].id);
      init += LATENCY;
    }
  }

  puts("Os estados inicias de cada processo sao:");
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
    difusion[i].messages = init_array();
    difusion[i].delivered = false;
    difusion[i].pendingACK = malloc(sizeof(bool) * N);
    for (int j = 0; j < N; ++j) {
      difusion[i].pendingACK[j] = false;
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

void print_pending(int N, int token) {
  printf("Pending do processo %d: [", token);
  for (int i = 0; i < N; ++i) {
    printf("%d", difusion[token].pendingACK[i]);
    if (i != N - 1) {
      printf(", ");
    }
  }
  puts("]");
}