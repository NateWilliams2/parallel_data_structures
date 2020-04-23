#ifndef QUEUE_H
#define QUEUE_H

#include <stdbool.h>
#include <pthread.h>

typedef struct node{
  int data;
  struct node* next; // Next points backwards, towards the tail
} node_t;

typedef struct my_queue {
  node_t *head, *tail;
  pthread_mutex_t head_lock, tail_lock; // One lock for head, one for tail
  int size;
} my_queue_t;

// Initialize a queue
void queue_init(my_queue_t* queue);

// Destroy a queue
void queue_destroy(my_queue_t* queue);

// Put an element at the end of a queue
void queue_put(my_queue_t* queue, int element);

// Chekc if a queue is empty
bool queue_empty(my_queue_t* queue);

// Take an element off the front of a queue
int queue_take(my_queue_t* queue);

// Function to lock tail & head to prevent deadlock
bool atomic_lock(my_queue_t* queue, int threshold, int def_lock);

// Function to unlock head & tail to prevent deadlock
void atomic_unlock(my_queue_t* queue, bool both, int def_lock);

#endif
