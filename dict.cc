#include "dict.hh"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#define BUCKETS 101

// Dictionary implementation: Array of BUCKETS buckets, each holds a doubly-linked list of key-value pairs.
// List implementation:

// list_set sets key-value pair, adding one if none exists for that key.
void list_set(list_t* list, const char* key, int val){
  for(node_t *current = list->head; current != NULL; current = current->child){
    // Case where we find key/val pair
    if(strcmp(current->key, key) == 0){
      current->val = val;
      return;
    // Case where we reach end of list
    } else if(current->child == NULL){ // End of list, allocate new node for key/val pair
      current->child = (node_t*) malloc(sizeof(node_t*));
      assert(current->child != NULL);
      current->child->val = val;
      current->child->key = (char*) malloc(strlen(key) + 1);
      assert(current->child->key != NULL);
      strncpy(current->child->key, key, strlen(key) + 1);
      current->child->parent = current;
      current->child->child = NULL;
      return;
    }
  }
  // Case where list is empty
  printf("Allocate new node to head\n");
  list->head = (node_t*) malloc(sizeof(node_t));
  assert(list->head != NULL);
  list->head->val = val;
  printf("key size: %lu\n", strlen(key));

  list->head->key = (char*) malloc(strlen(key) + 1);
  assert(list->head->key != NULL);
  strncpy(list->head->key, key, strlen(key) + 1);
  printf("Set key %s, %s\n", key, list->head->key);

  // list->head->key = (char*) malloc(strlen(key) + 1);
  // printf("Key address: %p\n", &(list->head->key));
  // assert(list->head->key != NULL);
  // strncpy(list->head->key, key, (strlen(key) + 1));
  // printf("Key address: %p\n", &(list->head->key));
  // printf("Set key %s, %s\n", key, list->head->key);

  // char temp[] = "arlington";
  // list->head->key = temp;
  // printf("Key address: %p\n", &(list->head->key));
  // assert(list->head->key != NULL);
  // strncpy(list->head->key, key, (strlen(key) + 1));
  // printf("Key address: %p\n", &(list->head->key));
  // printf("Set key %s, %s\n", key, list->head->key);

  // strncpy(list->head->key, key, (strlen(key) + 1));
  // printf("Set key %s, %s\n", key, list->head->key);
  // printf("Set val %d\n", list->head->val);
  list->head->parent = NULL;
  list->head->child = NULL;
}

// list_contains returns true if a given key has an entry in the list.
bool list_contains(list_t* list, const char* key){
  printf("Contains key: %s\n", key);
  node_t *current = list->head;
  while(current != NULL){
    printf("key found: %s\n", current->key);
    if(strcmp(current->key, key) == 0){
      return true; // return true if key is found
    }
    current = current->child;
  }
  return false;
}

// list_get returns the value associated with a given key, or -1 if that key does not exist.
int list_get(list_t* list, const char* key){
  node_t *current = list->head;
  while(current != NULL){
    if(strcmp(current->key, key) == 0){
      return current->val; // return true if key is found
    }
    current = current->child;
  }
  return -1;
}

// list_remove removes the given key's key/value pair from the list. If none exists, it does nothing.
void list_remove(list_t* list, const char* key){
  node_t *current = list->head;
  while(current != NULL){
    if(strcmp(current->key, key) == 0){ // If key is found, remove node
      if(current == list->head) list->head = current->child;
      if(current->parent != NULL) current->parent->child = current->child;
      if(current->child != NULL) current->child->parent = current->parent;
      free(current->key);
      free(current);
    }
    current = current->child;
  }
}

// list_destroy destroys the contents of a list and then the list itself
void list_destroy(list_t* list){
  node_t *current = list->head;
  node_t *next;
  while(current != NULL){
    next = current->child;
    free(current->key);
    free(current);
    current = next;
  }
  free(list);
}


// simple hash function to convert key to an array index < BUCKETS
int hash(const char* key){
  int c = *key, sum = 0;
  while (c != '\0'){
    sum += c; // Sum ascii vals of all chars in string
    c = (*key++);
  }
  return sum % BUCKETS;
}


// Initialize a dictionary
void dict_init(my_dict_t* dict) {
  printf("Dictionary initialize\n");

  dict->lists = (list**) malloc(sizeof(list_t*) * BUCKETS);
  assert(dict->lists != NULL); // Initialize array
  for(int i=0; i<BUCKETS; i++){
    printf("Initialize bucket\n");
    dict->lists[i] = (list_t*) malloc(sizeof(list_t)); // Initialize each array bucket
    assert(dict->lists[i] != NULL);
    dict->lists[i]->head = NULL;
  }
}

// Destroy a dictionary
void dict_destroy(my_dict_t* dict) {
  for(int i=0; i<BUCKETS; i++){
    list_destroy(dict->lists[i]);
  }
  free(dict->lists);
}

// Set a value in a dictionary
void dict_set(my_dict_t* dict, const char* key, int value) {
  printf("Set Bucket: %d key: %s val: %d\n", hash(key), key, value);
  list_set(dict->lists[hash(key)], key, value);
}

// Check if a dictionary contains a key
bool dict_contains(my_dict_t* dict, const char* key) {
  printf("Contains Bucket: %d key: %s\n", hash(key), key);

  return list_contains(dict->lists[hash(key)], key);
}

// Get a value in a dictionary
int dict_get(my_dict_t* dict, const char* key) {
  return list_get(dict->lists[hash(key)], key);
}

// Remove a value from a dictionary
void dict_remove(my_dict_t* dict, const char* key) {
  printf("Remove Bucket: %d key: %s\n", hash(key), key);
  list_remove(dict->lists[hash(key)], key);
}
