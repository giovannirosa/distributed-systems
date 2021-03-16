/*
  Autor: Giovanni Rosa
  Ultima modificao: 31/01/2021

Declaracao do programa VCube (versao 2)
*/

#ifndef BEBCAST_H
#define BEBCAST_H

#include "../../lib/vcube2.h"
#include <regex.h>

// CONSTANTES ========================================

// Tempo maximo de simulacao
#define MAX_TIME 130

// Latencia entre as mensagens
#define LATENCY 30

// Evento de recebimento de mensagem
#define RECEIVE_MSG 4
// Evento de recebimento de ACK
#define RECEIVE_ACK 5

// ESTRUTURA DE DADOS ================================

// Descritor da lista de falhas
typedef struct {
  int id;      // identificador do processo
  bool failed; // inicialmente falho
} Fault;

// Descritor da mensagem
typedef struct {
  int type;   // tipo da mensagem: 0 = msg, 1 = ack
  int sender; // processo que enviou a mensagem
  int s;      // indice do cluster da mensagem
} ProcessMessage;

// Descritor da difusao do processo
typedef struct {
  int id;           // identificador de facility do SMPL
  Array messages;   // lista de mensagens destinadas
  bool delivered;   // indica se mensagem foi entregue
  bool *pendingACK; // lista de acks pendentes
} ProcessDifusion;

// VARIAVEIS =========================================

// Vetor de processos da simulacao
Fault *fault;

// Vetor de processos da simulacao
ProcessDifusion *difusion;

// Indica origem do broadcast
int source;

// BROADCAST =========================================

/**
 * Inicia best-effort broadcast
 * @param source processo origem do broadcast
 * @param logN log do numero total de processos
 */
void bebcast(int source, int logN);

/**
 * Envia mensagem
 * @param sender processo transmissor
 * @param s cluster a que o receptor pertence no transmissor
 * @param receiver processo receptor
 */
void send_msg(int sender, int s, int receiver);

/**
 * Envia ACK
 * @param sender processo transmissor
 * @param receiver processo receptor
 */
void send_ACK(int sender, int receiver);

/**
 * Recebe mensagem
 * @param token processo receptor
 * @param N numero total de processos
 */
void receive_msg(int token, int N);

/**
 * Recebe ACK
 * @param token processo receptor
 * @param N numero total de processos
 */
void receive_ACK(int token, int N);

/**
 * Entrega a mensagem para um processo especifico
 * @param token processo
 */
void deliver(int token);

/**
 * Reenvia mensagem em caso de deteccao de falha
 * @param token processo
 * @param N numero total de processos
 * @param logN log do numero total de processos
 */
void resend_failed(int token, int N, int logN);

// AUXILIARES ========================================

/**
 * Reseta os pendentes de um processo
 * @param token processo
 * @param N numero total de processos
 */
void reset_pending(int token, int N);

/**
 * Retorna se existe a mensagem foi entregue para um processo
 * @param token processo principal
 */
bool is_delivered(int token);

/**
 * Retorna se existe ACK pendente de um processo especifico
 * @param token processo principal
 * @param j processo a ser verificado
 */
bool is_pendingACK(int token, int j);

/**
 * Retorna se existe algum ACK pendente de um processo
 * @param token processo
 * @param N numero total de processos
 */
bool any_pending(int token, int N);

/**
 * Processa a entrada do usuario
 * @param N numero total de processos
 * @param N_faults numero total de falhas
 * @param argc quantidade de argumentos
 * @param argv vetor de argumentos
 */
void user_input(int *N, int *N_faults, int argc, char *argv[]);

/**
 * Constroi as falhas a partir da entrada
 * @param faults lista de falhas
 * @param N numero total de processos
 * @param N_faults numero total de falhas
 */
void build_faults(char *faults, int N, int *N_faults);

/**
 * Verifica o numero de ocorrencias de um caracter em uma string
 * @param str string a ser verificada
 * @param del caracter a ser procurado
 */
int occurrences(char *str, char del);

/**
 * Verifica se o processo é testador de outro em determinado cluster
 * @param N numero total de processos
 * @param N_faults numero total de falhas
 */
void schedule_events(int N, int N_faults);

/**
 * Inicializa processos para a difusao
 * @param N numero total de processos
 */
void init_difusion(int N);

/**
 * Retorna se determinado processo inicia falho
 * @param p indice do processo
 * @param N_faults numero total de falhas
 */
bool search_fault_failed(int p, int N_faults);

/**
 * Gera um numero randomico entre limites
 * @param upper indice do processo
 * @param lower numero total de falhas
 */
int gen_rand(int upper, int lower);

/**
 * Imprime o vetor de pendentes de um processo especifico
 * @param N numero total de processos
 * @param token index do processo
 */
void print_pending(int N, int token);

#endif