#pragma once
#include <stdlib.h>
#include "map.h"
#include "movelist.h"

/**
 * Sleep for a given number of milliseconds
 * \param   ms  The number of milliseconds to sleep for
 */
void sleep_ms(size_t ms);

// create queue holding coordinate types
typedef struct node_coord {
  coord_t c;
  struct node_coord *next;
} node_c;

typedef struct queue {
  node_c *head;
} queue_t;

// search queue functions
void add (queue_t *q, coord_t c);
void pop (queue_t *q);
void init (queue_t *q);

// moving to neighbor cells
coord_t increment(coord_t c, dir_t d);


