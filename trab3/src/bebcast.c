/*
  Autor: Giovanni Rosa
  Ultima modificao: 17/03/2021

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

static int randomize_helper(FILE *in) {
  unsigned int seed;

  if (!in)
    return -1;

  if (fread(&seed, sizeof seed, 1, in) == 1) {
    fclose(in);
    printf("A semente da randomizacao: %u\n", seed);
    srand(seed);
    return 0;
  }

  fclose(in);
  return -1;
}

static int randomize(void) {
  if (!randomize_helper(fopen("/dev/urandom", "r")))
    return 0;
  if (!randomize_helper(fopen("/dev/arandom", "r")))
    return 0;
  if (!randomize_helper(fopen("/dev/random", "r")))
    return 0;

  /* No randomness sources found. */
  return -1;
}

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
    if (e == TEST && process[token].state[token] % 2 != 0) {
      continue;
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
      failure(token, N, false);
      reset_pending(token, N);
      break;
    case RECOVERY:
      recovery(token, N);
      break;
    case RECEIVE_MSG:
      if (is_correct(token, token)) {
        receive_msg(token, N);
      }
      break;
    case RECEIVE_ACK:
      if (is_correct(token, token)) {
        receive_ACK(token, N);
      }
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
  // obtem o primeiro processo correto do processo token no cluster s
  int first = FFneighbor(token, s, token);
  if (first != -1) {
    // envia mensagem
    send_msg(token, s, first);
    // marca como ACK pendente
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
    // entrega a mensagem
    deliver(source);
    // envia broadcast para o primeiro processo correto de cada cluster
    for (int s = 1; s <= logN; ++s) {
      calc_send(source, s);
    }
  }
}

void send_msg(int sender, int s, int receiver) {
  ProcessMessage *msg = (ProcessMessage *)malloc(sizeof(ProcessMessage));
  msg->type = 0;        // tipo = mensagem
  msg->s = s;           // cluster
  msg->sender = sender; // emissor
  // insere na fila de mensagens do receptor
  insert_array(difusion[receiver].messages, msg);
  printf("Mensagem enviada do processo %d para o processo %d com cluster %d\n",
         sender, receiver, s);
  // agenda evento para o receptor
  schedule(RECEIVE_MSG, LATENCY, receiver);
}

void send_ACK(int sender, int receiver) {
  printf("O processo %d nao possui ACKs pendentes\n", sender);
  ProcessMessage *msg = (ProcessMessage *)malloc(sizeof(ProcessMessage));
  msg->type = 1;        // tipo = ACK
  msg->s = -1;          // cluster irrelevante
  msg->sender = sender; // emissor
  // insere na fila de mensagens do receptor
  insert_array(difusion[receiver].messages, msg);
  printf("ACK enviado do processo %d para o processo %d\n", sender, receiver);
  // agenda evento para o receptor
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
  // entrega mensagem, caso ainda nao entregue
  if (!is_delivered(token)) {
    deliver(token);
  }
  // prossegue com broadcast ou ACK, caso origem e emissor corretos
  if (is_correct(token, source) && is_correct(token, msg->sender)) {
    int s = msg->s;
    while (--s != 0) {
      // envia mensagem para primeiro processo correto do cluster
      calc_send(token, s);
    }
    // caso nao existam ACKs pendentes, retorna ACK para emissor
    if (msg->sender != -1 && !any_pending(token, N)) {
      send_ACK(token, msg->sender);
    }
  }
}

void receive_ACK(int token, int N) {
  // Consome primeira mensagem na lista, que foi incluida primeiro
  ProcessMessage *msg = retrieve_msg(token);
  printf("ACK recebido pelo processo %d do processo %d\n", token, msg->sender);
  // atualiza lista de ACKs pendentes
  difusion[token].pendingACK[msg->sender] = false;
  // imprime lista de ACKs pendentes
  print_pending(N, token);
  // caso nao haja mais ACKs pendentes, retorna ACKs para processos que estao aguardando
  if (!any_pending(token, N)) {
    for (int i = 0; i < N; ++i) {
      if (difusion[i].pendingACK[token]) {
        send_ACK(token, i);
      }
    }
  }
  // se for ultimo ACK recebido pela origem, broadcast completo
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
  // verifica numero de entradas correto
  if (argc != 3 && argc != 4) {
    puts("Uso correto: bebcast <fonte do broadcast> <numero de processos> "
         "<lista de falhas opcional (#,#:#)>");
    exit(1);
  }

  source = atoi(argv[1]); // origem
  *N = atoi(argv[2]); // numero de processos
  // caso 4 argumentos, constroi lista de falhas
  if (argc == 4) {
    if (strcmp(argv[3], "R") == 0) {
      // randomico
      if (randomize())
        fprintf(stderr,
                "Warning: Could not find any sources for randomness.\n");
      build_random(*N, N_faults);
    } else {
      // definido pela entrada do usuario
      build_faults(argv[3], *N, N_faults);
    }
  }

  // validacoes das entradas
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
  // numero de falhas a partir de quantos processos existem na entrada do usuario
  *N_faults = occurrences(faults, ',') + 1;
  // processa cada entrada da lista separada por virgula
  char **semiresult = malloc(sizeof(char *) * (*N_faults));
  char *token = strtok(faults, ",");
  while (token != NULL) {
    semiresult[ent] = malloc(strlen(token) + 1);
    strcpy(semiresult[ent++], token);
    token = strtok(NULL, ",");
  }
  free(token);

  // constroi falhas com cada entrada da lista
  fault = malloc(sizeof(Fault) * (*N_faults));
  for (int i = 0; i < *N_faults; i++) {
    int number_tokens = occurrences(semiresult[i], ':') + 1;
    if (number_tokens == 1) {
      // se for apenas o processo, falha no decorrer da simulacao
      fault[i].id = atoi(semiresult[i]);
      fault[i].failed = false;
      if (fault[i].id == 0 && semiresult[i][0] != '0') {
        printf("A lista de falhas apresenta uma entrada que nao e numero!\n");
        exit(1);
      }
    } else {
      // caso contrario, se o complemento for 
      // 0, falha no decorrer da simulacao
      // 1, inicia falho
      char *token2 = strtok(semiresult[i], ":");
      ent = 0;
      while (token2 != NULL) {
        if (ent++ == 0) {
          fault[i].id = atoi(token2);
          if (fault[i].id == 0 && token2[0] != '0') {
            printf(
                "A lista de falhas apresenta uma entrada que nao e numero!\n");
            exit(1);
          }
        } else {
          int failedNumber = atoi(token2);
          if (failedNumber == 0 && token2[0] != '0') {
            printf(
                "A lista de falhas apresenta uma entrada que nao e numero!\n");
            exit(1);
          } else {
            fault[i].failed = failedNumber == 1;
          }
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

void build_random(int N, int *N_faults) {
  // numero de falhas aleatorio
  *N_faults = gen_rand(0, N - 1);

  // inicializa vetor de falhas
  fault = malloc(sizeof(Fault) * (*N_faults));
  for (int i = 0; i < *N_faults; ++i) {
    fault[i].id = -1;
  }

  // processos que falharao aleatorios e distintos
  for (int i = 0; i < *N_faults; ++i) {
    int candidate;
    do {
      candidate = gen_rand(0, N - 1);
    } while (already_exists(*N_faults, candidate));
    fault[i].id = candidate;
    fault[i].failed = false;
  }
}

bool already_exists(int N_faults, int p) {
  for (int i = 0; i < N_faults; ++i) {
    if (fault[i].id == p) {
      return true;
    }
  }
  return false;
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

  int init = LATENCY;
  bool print_sched = false;
  for (int i = 0; i < N_faults; i++) {
    if (fault[i].failed) {
      failure(fault[i].id, N, true);
      for (int j = 0; j < N; ++j) {
        if (process[j].state[fault[i].id] != -1) {
          process[j].state[fault[i].id] = 1;
        }
      }
    } else {
      if (!print_sched) {
        printf("A ordem das falhas no decorrer da simulacao: ");
        print_sched = true;
      }
      printf("%d", fault[i].id);
      if (i != N_faults - 1) {
        printf(",");
      }
      schedule(FAULT, init, fault[i].id);
      init += LATENCY;
    }
  }
  if (print_sched) {
    puts("");
  } else {
    puts("Nao existem falhas agendadas para o decorrer da simulacao");
  }

  puts("Os estados inicias de cada processo sao:");
  for (int i = 0; i < N; ++i) {
    print_state(N, i, -1);
  }
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

int gen_rand(int lower, int upper) {
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