/*
  Autor: Giovanni Rosa
  Ultima modificao: 31/01/2021

Declaracao do programa VCube (versao 2)
*/

#ifndef BEBCAST_H
#define BEBCAST_H

#include "../../lib/vcube2.h"
#include <regex.h>

// Descritor da lista de falhas
typedef struct {
  int id;      // identificador do processo
  bool failed; // inicialmente falho
} Fault;

// Vetor de processos da simulacao
Fault *fault;

void user_input(int *, int *, int *, char *, int, char *[]);
void build_faults(char *, int, int *);
int occurrences(char *, char);
void schedule_events(int, int);
bool search_fault_failed(int, int);

#endif