#ifndef DICT_H
#define DICT_H
//#define MAX_KEY_SIZE 20

#include <stdbool.h>

typedef struct node {
  struct node *parent, *child;
  int val;
  char *key;
  //char key[MAX_KEY_SIZE];
} node_t;

typedef struct list {
  node_t *head;
} list_t;

typedef struct my_dict {
  list_t **lists;
} my_dict_t;

// Initialize a dictionary
void dict_init(my_dict_t* dict);

// Destroy a dictionary
void dict_destroy(my_dict_t* dict);

// Set a value in a dictionary
void dict_set(my_dict_t* dict, const char* key, int value);

// Check if a dictionary contains a key
bool dict_contains(my_dict_t* dict, const char* key);

// Get a value in a dictionary
int dict_get(my_dict_t* dict, const char* key);

// Remove a value from a dictionary
void dict_remove(my_dict_t* dict, const char* key);

#endif
