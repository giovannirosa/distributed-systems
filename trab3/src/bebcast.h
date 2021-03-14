/*
  Autor: Giovanni Rosa
  Ultima modificao: 31/01/2021

Declaracao do programa VCube (versao 2)
*/

#ifndef BEBCAST_H
#define BEBCAST_H

#include "../../lib/vcube2.h"
#include <regex.h>

// Tempo maximo de simulacao
#define MAX_TIME 300

// Evento de recebimento de mensagem
#define RECEIVE_MSG 4
// Evento de recebimento de ACK
#define RECEIVE_ACK 5

// Descritor da lista de falhas
typedef struct {
  int id;      // identificador do processo
  bool failed; // inicialmente falho
} Fault;

// Vetor de processos da simulacao
Fault *fault;

// Indica origem do broadcast
int source;

// Descritor da difusao do processo
typedef struct {
  int id;           // identificador de facility do SMPL
  int sender;       // processo que enviou a mensagem
  int s;            // indice do cluster da mensagem
  bool delivered;   // indica se mensagem foi entregue
  bool *pendingACK; // lista de acks pendentes
} ProcessDifusion;

// Vetor de processos da simulacao
ProcessDifusion *difusion;

void bebcast(int source, int logN);

void send_msg(int sender, int s, int receiver);

void send_ACK(int sender, int receiver);

void receive_msg(int token);

void receive_ACK(int token, int N);

bool is_delivered(int token);

bool is_pendingACK(int token, int j);

bool any_pending(int token, int N);

void deliver(int token);

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

#endif