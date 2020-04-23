#include "stack.hh"

#include <stdlib.h>
#include <stdio.h>

// Initialize a stack
void stack_init(my_stack_t* stack) {
  if(pthread_mutex_init(&stack->lock, NULL) != 0) perror("Could not initialize mutex lock");
  stack->head = NULL;
}

// Destroy a stack
void stack_destroy(my_stack_t* stack) {
  pthread_mutex_lock(&stack->lock);
  node_t *temp = stack->head;
  for(node_t *current = stack->head; current != NULL;){ // free all nodes sequentially
    temp = current;
    current = temp->next;
    free(temp);
  }
}

// Push an element onto a stack
void stack_push(my_stack_t* stack, int element) {
  pthread_mutex_lock(&stack->lock);
  node_t *temp = stack->head; // Previous node saved to temp
  stack->head = (node_t*)malloc(sizeof(node_t*));
  if(stack->head == NULL) perror("Could not allocate space");
  stack->head->data = element;
  stack->head->next = temp; // Set previous node to next
  pthread_mutex_unlock(&stack->lock);
}

// Check if a stack is empty
bool stack_empty(my_stack_t* stack) {
  if(stack->head == NULL){
    return true;
  }
  return false;
}

// Pop an element off of a stack
int stack_pop(my_stack_t* stack) {
  pthread_mutex_lock(&stack->lock);
  if(stack->head == NULL){
    pthread_mutex_unlock(&stack->lock);
    return -1;
  } else{
    int val = stack->head->data;
    node_t *temp = stack->head->next; // Save next and head val before freeing
    free(stack->head);
    stack->head = temp; // Set head to next val
    pthread_mutex_unlock(&stack->lock);
    return val;
  }
}
