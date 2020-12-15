/*
  Autor: Giovanni Rosa
  Ultima modificao: 14/12/2020

Declaracao do programa VRing
*/

#ifndef VRING_H
#define VRING_H

#include "../../lib/array.h"
#include "../../lib/smpl.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

// Eventos
#define TEST 1
#define FAULT 2
#define RECOVERY 3

// Tempo maximo de simulacao
#define MAX_TIME 150

// Descritor do processo
typedef struct {
  int id;     // identificador de facility do SMPL
  int *state; // vetor de estados de cada processo
} ProcessType;

ProcessType *process;

// Controle de ID dos eventos
int id_cont = 0;

// Descritor do evento
typedef struct {
  int id;      // identificador do evento
  int e_round; // round em que o evento ocorreu
  int proc;    // numero do processo de origem
  int type;    // tipo do evento: 0 = recuperacao, 1 = falha
  int n_tests; // numero de testes do evento
  int n_proc;  // numero de processos que descobriram o evento
  int latency; // latencia para diagnosticar o evento
} Event;

Event *event;

Array event_array;

int sim_round = 0;

void count_event_discovery(int, int, int);
void count_event_test(int, int, int);
void print_event_array();
void create_event(int, int);
void user_input(int *, int, char *[]);
void init_process(int);
void schedule_events(int);

#endif