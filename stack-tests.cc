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


// A test of invariant 1
TEST(StackTest, Invariant1) {
  srand (time(NULL)); // Seed random number
  // Create a stack
  my_stack_t s;
  stack_init(&s);
  for(int loop=0; loop < 5; loop++){
    int push = rand() % 10 + 11;
    int pop = rand() % 10;
    int num_vals = 0;
    for(int i=0; i < push; i++){
      stack_push(&s, 1);
    }
    for(int i=0; i < pop; i++){
      stack_pop(&s);
    }
    while(1){
      int val = stack_pop(&s);
      if(val == 1) num_vals++;
      if(val == -1) break;
    }
    ASSERT_EQ(num_vals, push-pop);
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
