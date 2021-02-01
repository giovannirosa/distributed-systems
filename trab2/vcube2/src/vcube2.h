/*
  Autor: Giovanni Rosa
  Ultima modificao: 31/01/2021

Declaracao do programa VCube (versao 2)
*/

#ifndef VRING_H
#define VRING_H

#include "../../../lib/array.h"
#include "../../../lib/cisj.h"
#include "../../../lib/smpl.h"
#include <stdbool.h>

// Evento de teste
#define TEST 1
// Evento de falha
#define FAULT 2
// Evento de recuperacao
#define RECOVERY 3

// Tempo maximo de simulacao
#define MAX_TIME 220

// Descritor do processo
typedef struct {
  int id;      // identificador de facility do SMPL
  int *state;  // vetor de estados de cada processo
  bool tested; // se o processo testou na rodada atual
  int cluster; // cluster a ser testado
} ProcessType;

// Vetor de processos da simulacao
ProcessType *process;

// Controle de ID dos eventos
int id_cont = 0;

// Descritor do evento
typedef struct {
  int id;        // identificador do evento
  int e_round;   // round em que o evento ocorreu
  double e_time; // tempo em que o evento ocorreu
  int proc;      // numero do processo de origem
  int type;      // tipo do evento: 0 = recuperacao, 1 = falha
  int n_tests;   // numero de testes do evento
  bool diag;     // se o evento ja foi diagnosticado
  int latency;   // latencia para diagnosticar o evento
} Event;

// Evento atual da simulacao
Event *event;

// Lista de eventos da simulacao
Array event_array;

// Rodada atual da simulacao
int sim_round = 0;

/**
 * Verifica se o processo é testador de outro em determinado cluster
 * @param j processo potencial testado
 * @param s index do cluster
 * @param token processo potencial testador
 * @param nodes vetor de processos do cluster
 */
int verify_tester(int j, int s, int token);

/**
 * Conta o proximo cluster a ser testado por determinado processo
 * @param token index do processo
 * @param logN log na base 2 do numero total de processos
 */
void count_cluster(int, int);

/**
 * Verifica se todos os processos sem falha testaram na rodada atual
 * @param N numero total de processos
 * @param logN log na base 2 do numero total de processos
 */
void count_round(int, int);

/**
 * Adia evento para a proxima rodada
 * @param type tipo do evento: 0 = recuperacao, 1 = falha
 * @param token index do processo em que o evento estava agendado
 */
void delay_event(int, int);

/**
 * Imprime os processos do cluster
 * @param nodes vetor de processos do cluster
 * @param cluster index do cluster
 * @param size tamanho do vetor de processos do cluster
 */
void print_cluster(int nodes[], int token, int size);

/**
 * Imprime o vetor de estados de um processo especifico
 * @param token index do processo
 * @param N numero total de processos
 * @param when imprimir: antes = 0, depois = 1, nao imprimir = -1
 */
void print_state(int, int, int);

/**
 * Verifica novidades do vetor de estados de um processo em relacao ao outro
 * @param N numero total de processos
 * @param token index do processo testador
 * @param token2 index do processo testado
 */
void check_state(int, int, int);

/**
 * Contabiliza descoberta do evento por um processo
 * @param N numero total de processos
 * @param token index do processo testador
 * @param token2 index do processo testado
 * @param state valor do estado da descoberta
 */
void count_event_discovery(int, int, int, int);

/**
 * Contabiliza teste do evento por um processo
 * @param N numero total de processos
 * @param token index do processo testador
 * @param token2 index do processo testado
 */
void count_event_test(int, int, int);

/**
 * Imprime vetor com todos os eventos da simulacao
 */
void print_event_array();

/**
 * Cria um evento
 * @param N numero total de processos
 * @param type tipo do evento: 0 = recuperacao, 1 = falha
 * @param token index do processo em que o evento ocorreu
 */
void create_event(int, int, int);

/**
 * Processa as entradas do usuario
 * @param N numero total de processos
 * @param argc quantidade de argumentos
 * @param argc vetor de argumentos
 */
void user_input(int *, int, char *[]);

/**
 * Inicializa processos na memoria
 * @param N numero total de processos
 */
void init_process(int);

/**
 * Agenda os eventos que ocorrerao durante a simulacao
 * @param N numero total de processos
 */
void schedule_events(int);

#endif