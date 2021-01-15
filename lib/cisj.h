/*
  Autor: Giovanni Rosa
  Ultima modificao: 14/01/2021

Declaracao da função Cisj
*/

#ifndef CISJ_H
#define CISJ_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define POW_2(num) (1 << (num))
#define VALID_J(j, s) ((POW_2(s - 1)) >= j)

/* |-- node_set.h */
typedef struct node_set {
  int *nodes;
  ssize_t size;
  ssize_t offset;
} node_set;

void set_free(node_set *nodes);

/**
 * Funcao cis do vcube
 * @param i index do processo testador
 * @param s index do cluster do processo testador
 */
node_set *cis(int i, int s);

#endif