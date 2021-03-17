/*
  Autor: Giovanni Rosa
  Ultima modificao: 31/01/2021

Declaracao do programa VCube (versao 2)
*/

#ifndef VCUBE_H
#define VCUBE_H

#include "./array.h"
#include "./cisj.h"
#include "./smpl.h"
#include <stdbool.h>

// Imprime logs de teste
#define PRINT 0

// Evento de teste
#define TEST 1
// Evento de falha
#define FAULT 2
// Evento de recuperacao
#define RECOVERY 3

// Descritor do processo
typedef struct {
  int id;      // identificador de facility do SMPL
  int *state;  // vetor de estados de cada processo
  bool tested; // se o processo testou na rodada atual
} ProcessType;

// Vetor de processos da simulacao
ProcessType *process;

// Controle de ID dos eventos
static int id_cont = 0;

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
static int sim_round = 0;

/**
 * Falha o processo
 * @param token processo a falhar
 * @param N numero total de processos
 * @param logN log do numero total de processos
 */
bool test(int token, int N, int logN);

/**
 * Falha o processo
 * @param token processo a falhar
 * @param N numero total de processos
 * @param ignore_diag ignora a verificacao de diagnostico
 */
bool failure(int token, int N, bool ignore_diag);

/**
 * Recupera o processo
 * @param token processo a recuperar
 * @param N numero total de processos
 */
bool recovery(int token, int N);

/**
 * Retorna o indice do cluster de j a que i pertence
 * @param i processo a ser verificado se pertence ao cluster
 * @param j processo a ser verificado os clusters
 * @param logN log do numero total de processos
 */
int cluster(int i, int j, int logN);

/**
 * Retorna o primeiro processo correto no cluster
 * @param i processo a ter o cluster verificado
 * @param s indice do cluster
 * @param token processo a ter os estados verificados
 */
int FFneighbor(int i, int s, int token);

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
void count_cluster(int token, int logN);

/**
 * Verifica se todos os processos sem falha testaram na rodada atual
 * @param N numero total de processos
 * @param logN log na base 2 do numero total de processos
 */
void count_round(int N, int logN);

/**
 * Adia evento para a proxima rodada
 * @param type tipo do evento: 0 = recuperacao, 1 = falha
 * @param token index do processo em que o evento estava agendado
 */
void delay_event(int type, int token);

/**
 * Imprime os processos do cluster
 * @param nodes vetor de processos do cluster
 * @param token index do processo
 * @param size tamanho do vetor de processos do cluster
 * @param s indice do cluster
 */
void print_cluster(int nodes[], int token, int size, int s);

/**
 * Imprime o vetor de estados de um processo especifico
 * @param N numero total de processos
 * @param token index do processo
 * @param when imprimir: antes = 0, depois = 1, nao imprimir = -1
 */
void print_state(int N, int token, int when);

/**
 * Verifica novidades do vetor de estados de um processo em relacao ao outro
 * @param N numero total de processos
 * @param token index do processo testador
 * @param token2 index do processo testado
 */
void check_state(int N, int token, int token2);

/**
 * Contabiliza descoberta do evento por um processo
 * @param N numero total de processos
 * @param token index do processo testador
 * @param token2 index do processo testado
 * @param state valor do estado da descoberta
 */
void count_event_discovery(int N, int token, int token2, int state);

/**
 * Contabiliza teste do evento por um processo
 * @param N numero total de processos
 * @param token index do processo testador
 * @param token2 index do processo testado
 */
void count_event_test(int N, int token, int token2);

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
void create_event(int N, int type, int token);

/**
 * Inicializa processos na memoria
 * @param N numero total de processos
 */
void init_process(int N);

/**
 * Agenda os eventos que ocorrerao durante a simulacao
 * @param token processo principal
 * @param j processo a ser conferido estado
 */
bool is_correct(int token, int j);

#endif