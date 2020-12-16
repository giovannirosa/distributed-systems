/*
  Autor: Giovanni Rosa
  Ultima modificao: 15/12/2020

Declaracao do programa VRing
*/

#ifndef VRING_H
#define VRING_H

#include "../../lib/array.h"
#include "../../lib/smpl.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

// Evento de teste
#define TEST 1
// Evento de falha
#define FAULT 2
// Evento de recuperacao
#define RECOVERY 3

// Tempo maximo de simulacao
#define MAX_TIME 350

// Descritor do processo
typedef struct {
  int id;      // identificador de facility do SMPL
  int *state;  // vetor de estados de cada processo
  bool tested; // se o processo testou na rodada atual
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
 * Verifica se todos os processos sem falha testaram na rodada atual
 * @param N numero total de processos
 */
void count_round(int);

/**
 * Adia evento para a proxima rodada
 * @param type tipo do evento: 0 = recuperacao, 1 = falha
 * @param token index do processo em que o evento estava agendado
 */
void delay_event(int, int);

/**
 * Imprime o vetor de estados de um processo especifico
 * @param token index do processo
 * @param N numero total de processos
 */
void print_state(int, int);

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