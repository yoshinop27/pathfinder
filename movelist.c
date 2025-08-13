#include "movelist.h"

#include <stdlib.h>
#include <string.h>

// Initialize a movelist. The m parameter points to space that holds a movelist
void movelist_init(movelist_t* m) {
  m->head = NULL;
}

// Clean up allocated memory in a movelist.
void movelist_destroy(movelist_t* m) {
  while (m->head != NULL){
    node_t *temp = m->head;
    m->head = m->head->next;
    free(temp);
  }
}

// Peek at the value at the front of movelist m. Returns EMPTY if the list is empty
dir_t movelist_peek_front(const movelist_t* m) {
  if(m->head != NULL){
    return m->head->d;
  } else {
    return EMPTY;
  }
}

// Take the value at the front of movelist m. Returns EMPTY if the list is empty
dir_t movelist_take_front(movelist_t* m) {
  if (m->head != NULL && m->head->next != NULL){
  dir_t front = m->head->d;
  node_t *head_temp = m->head->next;
  free(m->head);
  m->head = head_temp;
  return front;
  } else if (m->head != NULL && m->head->next == NULL){
  dir_t front = m->head->d;
  free(m->head);
  m->head = NULL;
  return front;
  } else {
    return EMPTY;
  }
}

// Peek at the value at the back of movelist m. Returns EMPTY if the list is empty
dir_t movelist_peek_back(const movelist_t* m) {
  node_t *i;
  if (m->head != NULL){
  for (i = m->head; i->next != NULL; i = i->next){}
  return i->d;
  } else {
    return EMPTY;
  }
}


// Take the value at the back of movelist m. Returns EMPTY if the list is empty
dir_t movelist_take_back(movelist_t* m) {
  if (m->head == NULL){
    return EMPTY;
  } else {
  node_t *i;
  for (i = m->head; i->next != NULL; i = i->next){}
  dir_t back = i->d;
  free(i);
  return back;
  }
}

// Add a move to the front of movelist m
void movelist_add_front(movelist_t* m, dir_t d) {
  node_t *new = malloc(sizeof(node_t));
  new->d = d;
  new->next = m->head;
  m->head = new;
}

// Add a move to the back of movelist m
void movelist_add_back(movelist_t* m, dir_t d) {
  node_t *i;
  node_t *new = malloc(sizeof(node_t));
  new->d = d;
  new->next = NULL;
  if (m->head == NULL){
    m->head = new;
  } else {
    i = m->head;
    while(i->next != NULL){
      i = i->next;
    }
    i->next = new;
  }
}

// Empty the movelist m
void movelist_clear(movelist_t* m) {
  if (m->head != NULL) {
  while (m->head->next != NULL){
    node_t *temp = m->head;
    m->head = m->head->next;
    free(temp);
  }
  free(m->head);
  }
}
