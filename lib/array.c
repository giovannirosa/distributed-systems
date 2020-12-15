/*
  Autor: Giovanni Rosa
  Ultima modificao: 14/12/2020

Implementacao de uma lista com todas as suas funcoes auxiliares
*/

#include "array.h"

// inicia lista alocando memória e iniciando parâmetros
Array init_array(void) {
  Array l = malloc(sizeof(struct array_s));

  l->size = 0;
  l->ini = NULL;
  l->end = NULL;

  return l;
}

// libera lista da memória, um nó de cada vez, então a lista
void free_array(Array l) {
  Node aux;
  while ((aux = l->end)) {
    l->end = aux->prev;
    free(aux->cont);
    free(aux);
  }
  free(l);
}

// cria noh para inserir na lista
Node create_node(void *cont) {
  Node aux = malloc(sizeof(struct node_s));
  aux->cont = cont;
  aux->prev = NULL;
  aux->next = NULL;

  return aux;
}

// retorna noh do index desejado
Node return_node(Array lista, int index) {
  int count = 0;
  for (Node aux = lista->ini; aux; aux = aux->next) {
    if (index == count++) {
      return aux;
    }
  }

  return NULL;
}

// insere nó com conteúdo passado no final da lista
Node insert_array(Array l, void *cont) {
  Node n = create_node(cont);

  if (l->size == 0) {
    l->ini = n;
    l->end = n;
  } else {
    l->end->next = n;
    n->prev = l->end;
    l->end = n;
  }
  l->size++;

  return n;
}

// remove nó do final da lista
void remove_array(Array l) {
  Node aux = l->end;
  if (!aux)
    return;

  Node prev = aux->prev;
  if (prev) {
    l->end = prev;
    prev->next = NULL;
  } else {
    l->ini = NULL;
    l->end = NULL;
  }

  l->size--;
  free(aux);
}

// remove nó específico da lista
void remove_array_spec(Array l, Node aux) {
  if (!aux)
    return;

  if (aux == l->ini && aux == l->end) {
    l->ini = NULL;
    l->end = NULL;
  } else if (aux == l->ini) {
    aux->next->prev = NULL;
    l->ini = aux->next;
  } else if (aux == l->end) {
    aux->prev->next = NULL;
    l->end = aux->prev;
  } else {
    aux->next->prev = aux->prev;
    aux->prev->next = aux->next;
  }

  l->size--;
  free(aux);
}

// copia o conteudo de uma lista
Array copy_array(Array l) {
  Array copia = init_array();
  for (Node aux = l->ini; aux != NULL; aux = aux->next)
    insert_array(copia, aux->cont);
  return copia;
}