/*
  Autor: Giovanni Rosa
  Ultima modificao: 14/12/2020

Declaracao de uma lista com todas as suas funcoes auxiliares
*/

#ifndef LISTA_H
#define LISTA_H

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct node_s *Node;
typedef struct array_s *Array;

// nó da lista com conteudo, proximo e anterior
struct node_s {
  void *cont;
  Node next, prev;
};

// lista com inicio, fim e tamanho
struct array_s {
  Node ini, end;
  long size;
};

// inicia lista alocando memória e iniciando parâmetros
Array init_array(void);

// libera lista da memória, um nó de cada vez, então a lista
void free_array(Array);

// cria Node para inserir na lista
Node create_node(void *);

// retorna noh do index desejado
Node return_node(Array, int);

// insere nó com conteúdo passado no final da lista
Node insert_array(Array, void *);

// remove nó do final da lista
void remove_array(Array);

// remove nó específico da lista
void remove_array_spec(Array, Node);

// copia o conteudo de uma lista
Array copy_array(Array);

#endif