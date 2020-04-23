#include "queue.hh"

#include <stdlib.h>
#include <stdio.h>

#define HEAD_LOCK 0
#define TAIL_LOCK 1
#define BOTH_LOCKS 2

// Function to lock tail & head to prevent deadlock
// Threshold represents size below which both lock shoudl be locked.
// Def_lock is the lock to be locked if both do not need to be locked.
// Returns true if both are locked.
bool atomic_lock(my_queue_t* queue, int threshold,int def_lock){
  // If queue is small, or def_lock is both, we need both locks.
  if(queue->size <= threshold || def_lock == BOTH_LOCKS){
    // Always lock tail first
    pthread_mutex_lock(&queue->tail_lock);
    pthread_mutex_lock(&queue->head_lock);
    return true;
  } else if(def_lock == HEAD_LOCK){
    pthread_mutex_lock(&queue->head_lock);
  } else if(def_lock == TAIL_LOCK){
    pthread_mutex_lock(&queue->tail_lock);
  }
  return false;
}

// Function to unlock head & tail to prevent deadlock
// If both is true, both locks are unlocked.
// Else, locks the lock signified by def_lock
void atomic_unlock(my_queue_t* queue, bool both, int def_lock){
  // If queue is small, or def_lock is both, unlock it as well.
  if(both){
    pthread_mutex_unlock(&queue->head_lock);
    pthread_mutex_unlock(&queue->tail_lock);
  } else if(def_lock == HEAD_LOCK){
    pthread_mutex_unlock(&queue->head_lock);
  }else if(def_lock == TAIL_LOCK){
    pthread_mutex_unlock(&queue->tail_lock);
  }
}

// Initialize a new queue
void queue_init(my_queue_t* queue) {
  if(pthread_mutex_init(&queue->tail_lock, NULL) != 0) perror("Could not initialize mutex lock");
  if(pthread_mutex_init(&queue->head_lock, NULL) != 0) perror("Could not initialize mutex lock");
  queue->tail = NULL;
  queue->head = NULL;
  queue->size = 0;
}

// Destroy a queue
void queue_destroy(my_queue_t* queue) {
  atomic_lock(queue, 0, BOTH_LOCKS);
  node_t *temp = queue->head;
  for(node_t *current = queue->head; current != NULL;){ // free all nodes sequentially
    temp = current;
    current = temp->next;
    free(temp);
  }
  queue->size = 0;
}

// Put an element at the end of a queue
void queue_put(my_queue_t* queue, int element) {
  bool both_unlock = atomic_lock(queue, 2, TAIL_LOCK); // Lock both locks if queue is small
  node_t *new_node = (node_t*)malloc(sizeof(node_t*));
  if(new_node == NULL) perror("Could not allocate space");
  new_node->data = element;
  new_node->next = NULL;
  // If there is older tail, set next to new node
  if(queue->size > 0) {
    queue->tail->next = new_node;
  } else {
    queue->head = new_node;
  }
  queue->tail = new_node;
  queue->size++;
  atomic_unlock(queue, both_unlock, TAIL_LOCK);
}

// Check if a queue is empty
bool queue_empty(my_queue_t* queue) {
  if(queue->size == 0) return true;
  return false;
}

// Take an element off the front of a queue
int queue_take(my_queue_t* queue) {
  bool both_unlock = atomic_lock(queue, 2, HEAD_LOCK); // Lock both locks if queue is small
  if(queue->size == 0){ // If empty queue, unlock & return -1
    atomic_unlock(queue, true, 0);
    return -1;
  } else{
    int val = queue->head->data;
    node_t *temp = queue->head;
    queue->head = queue->head->next;
    free(temp);
    queue->size--;
    atomic_unlock(queue, both_unlock, HEAD_LOCK);
    return val;
  }
}
