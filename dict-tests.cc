#include <gtest/gtest.h>

#include "dict.hh"

#define NUM_THREADS 25

/****** Dictionary Invariants ******/

// Invariant 1
// If a key A has been set and has not since been removed, dict_contains(A) should return true.
// If the key has been removed and not reset, dict_contains(A) should return false.

// Invariant 2
// If a key A has been set with a value B, and has not been modified since, dict_get(A) returns B

// Invariant 3
// If any thread resets the value of a key from A to B, dict_get should return B when called by any thread.

// Invariant 4
// If a key has been removed, dict_get should return -1

/****** Synchronization ******/

// Under what circumstances can accesses to your dictionary structure can proceed in parallel? Answer below.
// The dictionary is implemented as an array of linked lists, where each node of each linked list represents
// a key-value pair. With an array that is much larger than the number of values stored, we can expect each
// bucket of the array to contain one or less elements, on average.
//
// Each array index has its own lock. This means that all accesses to separate array buckets can occur
// concurrently. If multiple elements reside on the same linked-list array index, these elements cannot
// be accessed concurrently by any of the dictionary functions.

// When will two accesses to your dictionary structure be ordered by synchronization? Answer below.
// Accesses to elements which are stored in the same array index must be ordered in serial. One Dictionary
// operation on an element in index x must fully complete before executing another operation in index x.
// As explained above, this means that in a sufficiently large array very few elements will have to be accessed in serial. Most elements will be accessible in parallel to each other.
//
//

/****** Begin Tests ******/
typedef struct set_args {
  my_dict_t *d;
  const char *key;
  int val;
} set_args_t;

typedef struct remove_args {
  my_dict_t *d;
  const char *key;
} remove_args_t;

// Worker thread for tests: set value
void* set_worker(void* arg){
  set_args_t *args = (set_args_t*) arg;
  dict_set(args->d, args->key, args->val);
  pthread_exit(0);
}

// Worker thread for tests: remove value
void* remove_worker(void* arg){
  remove_args_t *args = (remove_args_t*) arg;
  dict_remove(args->d, args->key);
  pthread_exit(0);
}

// Test for invariant 4: remove
TEST(DictionaryTest, Invariant4) {
  my_dict_t d;
  dict_init(&d);
  char words[NUM_THREADS][4]; // Test with multi-char keys
  // Make sure values are not set(more keys than buckets ensures overlap)
  for(int i=0; i < NUM_THREADS; i++) {
    words[i][0] = 'a';
    words[i][1] = 'a' + i;
    words[i][2] = 'z' - i;
    words[i][3] = '\0';
    ASSERT_EQ(dict_get(&d, (const char*) words[i]), -1); // No values in dict
  }

  // set values (more keys than buckets ensures overlap)
  pthread_t set_workers[NUM_THREADS];
  set_args_t set_args[NUM_THREADS];
  for(int i=0; i < NUM_THREADS; i++) {
    set_args[i].d = &d;
    set_args[i].key = words[i];
    set_args[i].val = i;
    if(pthread_create(&set_workers[i], NULL, set_worker, &set_args[i]) != 0) perror("Could not create thread");
  }

  for(int i=0; i < NUM_THREADS; i++) {
    if(pthread_join(set_workers[i], NULL) != 0) perror("Could not exit thread");
  }

  // Make sure all keys are present
  for(int i=0; i < NUM_THREADS; i++) {
    ASSERT_EQ(dict_get(&d, (const char*) words[i]), i); // No values in dict
  }

  pthread_t remove_workers[NUM_THREADS];
  remove_args_t remove_args[NUM_THREADS];
  for(int i=0; i < NUM_THREADS; i++) {
    remove_args[i].d = &d;
    remove_args[i].key = words[i];
    if(pthread_create(&remove_workers[i], NULL, remove_worker, &remove_args[i]) != 0) perror("Could not create thread");
  }

  for(int i=0; i < NUM_THREADS; i++) {
    if(pthread_join(remove_workers[i], NULL) != 0) perror("Could not exit thread");
  }

  // Make sure all keys are removed
  for(int i=0; i < NUM_THREADS; i++) {
    ASSERT_EQ(dict_get(&d, (const char*) words[i]), -1); // No values in dict
  }
  // Clean up
  dict_destroy(&d);
}

// Test for invariant 3: value reset
TEST(DictionaryTest, Invariant3) {
  my_dict_t d;
  dict_init(&d);
  char words[NUM_THREADS][3]; // Test with multi-char keys
  // Make sure values are empty
  for(int i=0; i < NUM_THREADS; i++) {
    words[i][0] = 'a';
    words[i][1] = 'a' + i;
    words[i][2] = '\0';
    ASSERT_EQ(dict_get(&d, (const char*) words[i]), -1); // No values in dict
  }

  // set values with threads(more keys than buckets ensures overlap)
  pthread_t workers[NUM_THREADS*2];
  set_args_t args[NUM_THREADS*2];
  for(int i=0; i < NUM_THREADS; i++) {
    args[i].d = &d;
    args[i].key = words[i];
    args[i].val = i;
    if(pthread_create(&workers[i], NULL, set_worker, &args[i]) != 0) perror("Could not create thread");
  }

  for(int i=0; i < NUM_THREADS; i++) {
    if(pthread_join(workers[i], NULL) != 0) perror("Could not exit thread");
  }

  // reset values to new ones
  for(int i=NUM_THREADS; i < NUM_THREADS*2; i++) {
    args[i].d = &d;
    args[i].key = words[i-NUM_THREADS];
    args[i].val = i;
    args[i].val = i;
    if(pthread_create(&workers[i], NULL, set_worker, &args[i]) != 0) perror("Could not create thread");
  }

  for(int i=NUM_THREADS; i < NUM_THREADS*2; i++) {
    if(pthread_join(workers[i], NULL) != 0) perror("Could not exit thread");
  }

  // Make sure all keys are present
  for(int i=NUM_THREADS; i < NUM_THREADS*2; i++) {
    ASSERT_EQ(dict_get(&d, (const char*) words[i-NUM_THREADS]), i); // No values in dict
  }
  // Clean up
  dict_destroy(&d);
}

// Test for invariant 2: value integrity
TEST(DictionaryTest, Invariant2) {
  my_dict_t d;
  dict_init(&d);
  char words[NUM_THREADS][3]; // Test with multi-char keys
  // Make sure values are not set(more keys than buckets ensures overlap)
  for(int i=0; i < NUM_THREADS; i++) {
    words[i][0] = 'a';
    words[i][1] = 'a' + i;
    words[i][2] = '\0';
    ASSERT_EQ(dict_get(&d, (const char*) words[i]), -1); // No values in dict
  }

  // set values (more keys than buckets ensures overlap)
  pthread_t workers[NUM_THREADS];
  set_args_t args[NUM_THREADS];
  for(int i=0; i < NUM_THREADS; i++) {
    args[i].d = &d;
    args[i].key = words[i];
    args[i].val = i;
    if(pthread_create(&workers[i], NULL, set_worker, &args[i]) != 0) perror("Could not create thread");
  }

  for(int i=0; i < NUM_THREADS; i++) {
    if(pthread_join(workers[i], NULL) != 0) perror("Could not exit thread");
  }

  // Make sure all keys are present
  for(int i=0; i < NUM_THREADS; i++) {
    ASSERT_EQ(dict_get(&d, (const char*) words[i]), i); // No values in dict
  }
  // Clean up
  dict_destroy(&d);
}

// Test for invariant 1: presence/removal
TEST(DictionaryTest, Invariant1) {
  my_dict_t d;
  dict_init(&d);
  char words[NUM_THREADS][2];
  // Make sure dict is empty (more keys than buckets ensures overlap)
  for(int i=0; i < NUM_THREADS; i++) {
    words[i][0] = 'a' + i;
    words[i][1] = '\0';
    ASSERT_FALSE(dict_contains(&d, (const char*) words[i]));
  }

  // (more keys than buckets ensures overlap)
  pthread_t workers[NUM_THREADS];
  set_args_t args[NUM_THREADS];
  for(int i=0; i < NUM_THREADS; i++) {
    args[i].d = &d;
    args[i].key = words[i];
    args[i].val = i;
    if(pthread_create(&workers[i], NULL, set_worker, &args[i]) != 0) perror("Could not create thread");
  }

  for(int i=0; i < NUM_THREADS; i++) {
    if(pthread_join(workers[i], NULL) != 0) perror("Could not exit thread");
  }

  // Make sure all keys are present
  for(int i=0; i < NUM_THREADS; i++) {
    ASSERT_TRUE(dict_contains(&d, words[i]));
  }
  // Clean up
  dict_destroy(&d);
}

// Basic functionality for the dictionary
TEST(DictionaryTest, BasicDictionaryOps) {
  my_dict_t d;
  dict_init(&d);

  // Make sure the dictionary does not contain keys A, B, and C
  ASSERT_FALSE(dict_contains(&d, "A"));
  ASSERT_FALSE(dict_contains(&d, "B"));
  ASSERT_FALSE(dict_contains(&d, "C"));

  // Add some values
  dict_set(&d, "A", 1);
  dict_set(&d, "B", 2);
  dict_set(&d, "C", 3);

  // Make sure these values are contained in the dictionary

  ASSERT_TRUE(dict_contains(&d, "A"));
  ASSERT_TRUE(dict_contains(&d, "B"));
  ASSERT_TRUE(dict_contains(&d, "C"));

  // Make sure these values are in the dictionary
  ASSERT_EQ(1, dict_get(&d, "A"));
  ASSERT_EQ(2, dict_get(&d, "B"));
  ASSERT_EQ(3, dict_get(&d, "C"));

  // Set some new values
  dict_set(&d, "A", 10);
  dict_set(&d, "B", 20);
  dict_set(&d, "C", 30);

  // Make sure these values are contained in the dictionary
  ASSERT_TRUE(dict_contains(&d, "A"));
  ASSERT_TRUE(dict_contains(&d, "B"));
  ASSERT_TRUE(dict_contains(&d, "C"));

  // Make sure the new values are in the dictionary
  ASSERT_EQ(10, dict_get(&d, "A"));
  ASSERT_EQ(20, dict_get(&d, "B"));
  ASSERT_EQ(30, dict_get(&d, "C"));

  // Remove the values
  dict_remove(&d, "A");
  dict_remove(&d, "B");
  dict_remove(&d, "C");

  // Make sure these values are not contained in the dictionary
  ASSERT_FALSE(dict_contains(&d, "A"));
  ASSERT_FALSE(dict_contains(&d, "B"));
  ASSERT_FALSE(dict_contains(&d, "C"));

  // Make sure we get -1 for each value
  ASSERT_EQ(-1, dict_get(&d, "A"));
  ASSERT_EQ(-1, dict_get(&d, "B"));
  ASSERT_EQ(-1, dict_get(&d, "C"));

  // Clean up
  dict_destroy(&d);
}
