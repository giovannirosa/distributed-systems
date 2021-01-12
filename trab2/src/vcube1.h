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
#define MAX_TIME 250

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

#endif