#include <gtest/gtest.h>

#include "stack.hh"
#include "stdlib.h"
#include "time.h"

/****** Stack Invariants ******/

// Invariant 1
// For every value V that has been pushed onto the stack p times and returned by pop q times, there must be p-q copies of this value on the stack. This only holds if p >= q.

// Invariant 2
// No value should ever be returned by pop if it was not first passed to push by some thread.

// Invariant 3
// If a thread pushes value A and then pushes value B, and no other thread pushes these specific values, A must not be popped from the stack before popping B.

/****** Begin Tests ******/


typedef struct push_args {
  my_stack_t *s;
  int val;
} push_args_t;

// Worker thread for invariant 2 test
void* push_worker(void* arg){
  push_args_t *args = (push_args_t*) arg;
  stack_push(args->s, args->val);
  pthread_exit(0);
}

// A test of invariant 3: Value order must be maintained
TEST(StackTest, Invariant3){
  // Create a stack
  my_stack_t s;
  stack_init(&s);
  int A = 13;
  int B = 19; // Values to push
  int x = 10;

  push_args_t args[10];
  pthread_t workers[10];
  for(int i=0; i < 10; i++){ // Push 10 filler elements
    args[i].s = &s;
    args[i].val = x;
    if(pthread_create(&workers[i], NULL, push_worker, &args[i]) != 0) perror("Could not create thread");
  }
  stack_push(&s, A); // Push A and then B while filler threads are running
  stack_push(&s, B);

  for(int i=0; i < 10; i++){ // Wait for threads to exit
    if(pthread_join(workers[i], NULL) != 0) perror("Could not exit thread");
  }

  int val = 0;
  bool got_B = false;
  bool got_A = false;
  // B and then A should be popped.
  for(int i=0; i < 12; i++){ // Pop all elements
    if((val = stack_pop(&s)) == B){
      got_B = true;
    } else if(val == A){
      ASSERT_TRUE(got_B); // If we pop A, make sure B has been popped first
      got_A = true;
      }
    }
  ASSERT_TRUE(got_B & got_A); // make sure both A and B have been popped
}

// A test of invariant 2: only values pushed by some thread should ever be popped.
TEST(StackTest, Invariant2){
  // Create a stack
  my_stack_t s;
  stack_init(&s);
  int pushval = 15; // Value to push

  push_args_t args[10];
  pthread_t workers[10];
  for(int i=0; i < 10; i++){ // Push 10 elements
    args[i].s = &s;
    args[i].val = pushval;
    if(pthread_create(&workers[i], NULL, push_worker, &args[i]) != 0) perror("Could not create thread");
  }

  for(int i=0; i < 10; i++){ // Wait for threads to exit
    if(pthread_join(workers[i], NULL) != 0) perror("Could not exit thread");
  }

  // None of the elements popped should be anything but pushval (this is logically equivalent to invariant 2)
  for(int i=0; i < 10; i++){ // Pop 10 elements
    ASSERT_EQ(stack_pop(&s), pushval);
  }
}


// A test of invariant 1
TEST(StackTest, Invariant1) {
  srand (time(NULL)); // Seed random number
  // Create a stack
  my_stack_t s;
  stack_init(&s);
  for(int loop=0; loop < 5; loop++){ // Test several random push/pop values
    int push = rand() % 10 + 11;
    int pop = rand() % 10;
    int num_vals = 0;
    for(int i=0; i < push; i++){ // Push some elements
      stack_push(&s, 1);
    }
    for(int i=0; i < pop; i++){ // Pop less elements
      stack_pop(&s);
    }
    while(1){
      int val = stack_pop(&s); // Record number of elements left after pops
      if(val == 1) num_vals++;
      if(val == -1) break;
    }
    ASSERT_EQ(num_vals, push-pop); // there should be p-q copies of V
  }
}

// A simple test of basic stack functionality
TEST(StackTest, BasicStackOps) {
  // Create a stack
  my_stack_t s;
  stack_init(&s);

  // Push some values onto the stack
  stack_push(&s, 1);
  stack_push(&s, 2);
  stack_push(&s, 3);
  // Make sure the elements come off the stack in the right order
  ASSERT_EQ(3, stack_pop(&s));
  ASSERT_EQ(2, stack_pop(&s));
  ASSERT_EQ(1, stack_pop(&s));

  // Clean up
  stack_destroy(&s);
}

// Another test case
TEST(StackTest, EmptyStack) {
  // Create a stack
  my_stack_t s;
  stack_init(&s);

  // The stack should be empty
  ASSERT_TRUE(stack_empty(&s));

  // Popping an empty stack should return -1
  ASSERT_EQ(-1, stack_pop(&s));

  // Put something on the stack
  stack_push(&s, 0);

  // The stack should not be empty
  ASSERT_FALSE(stack_empty(&s));

  // Pop the element off the stack.
  // We're just testing empty stack behavior, so there's no need to check the resulting value
  stack_pop(&s);

  // The stack should be empty now
  ASSERT_TRUE(stack_empty(&s));

  // Clean up
  stack_destroy(&s);
}
