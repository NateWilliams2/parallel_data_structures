#include <gtest/gtest.h>

#include "queue.hh"

/****** Queue Invariants ******/

// Invariant 1
// For every value V that has been put onto the queue p times and returned by take q times, there must be p-q copies of this value on the queue. This only holds if p >= q.

// Invariant 2
// No value should ever be returned by take if it was not first passed to put by some thread.

// Invariant 3
// If a thread puts value A and then puts value B, and no other thread puts these specific values, B must not be taken from the queue before taking A.

typedef struct put_args {
  my_queue_t *s;
  int val;
} put_args_t;

// Worker thread for invariant 2 test
void* put_worker(void* arg){
  put_args_t *args = (put_args_t*) arg;
  queue_put(args->s, args->val);
  pthread_exit(0);
}

/****** Begin Tests ******/
// A test of invariant 3: Value order must be maintained
TEST(QueueTest, Invariant3){
  // Create a queue
  my_queue_t s;
  queue_init(&s);
  int A = 13;
  int B = 19; // Values to put
  int x = 10;

  put_args_t args[10];
  pthread_t workers[10];
  for(int i=0; i < 10; i++){ // Put 10 filler elements
    args[i].s = &s;
    args[i].val = x;
    if(pthread_create(&workers[i], NULL, put_worker, &args[i]) != 0) perror("Could not create thread");
  }
  queue_put(&s, A); // Put A and then B while filler threads are running
  queue_put(&s, B);

  for(int i=0; i < 10; i++){ // Wait for threads to exit
    if(pthread_join(workers[i], NULL) != 0) perror("Could not exit thread");
  }

  int val = 0;
  bool got_A = false;
  bool got_B = false;
  // A and then B should be taken.
  for(int i=0; i < 12; i++){ // take all elements
    if((val = queue_take(&s)) == A){
      got_A = true;
    } else if(val == B){
      ASSERT_TRUE(got_A); // If we take B, make sure A has been taken first
      got_B = true;
      }
    }
  ASSERT_TRUE(got_B & got_A); // make sure both A and B have been taken
}

// A test of invariant 2: only values put by some thread should ever be taken.
TEST(QueueTest, Invariant2){
  // Create a queue
  my_queue_t s;
  queue_init(&s);
  int putval = 15; // Value to put

  put_args_t args[10];
  pthread_t workers[10];
  for(int i=0; i < 10; i++){ // put 10 non-putval elements
    args[i].s = &s;
    args[i].val = putval;
    if(pthread_create(&workers[i], NULL, put_worker, &args[i]) != 0) perror("Could not create thread");
  }

  for(int i=0; i < 10; i++){ // Wait for threads to exit
    if(pthread_join(workers[i], NULL) != 0) perror("Could not exit thread");
  }

  // None of the elements taken should be anything but putval (this is logically equivalent to invariant 2)
  for(int i=0; i < 10; i++){ // take 10 elements
    ASSERT_EQ(queue_take(&s), putval);
  }
}

// A test of invariant 1: put/take number integrity
TEST(QueueTest, Invariant1) {
  srand (time(NULL)); // Seed random number
  // Create a queue
  my_queue_t s;
  queue_init(&s);
  for(int loop=0; loop < 5; loop++){ // Test several random put/take values
    int put = rand() % 10 + 11;
    int take = rand() % 10;
    int num_vals = 0;
    for(int i=0; i < put; i++){ // put some elements
      queue_put(&s, 1);
    }
    for(int i=0; i < take; i++){ // take less elements
      queue_take(&s);
    }
    while(1){
      int val = queue_take(&s); // Record number of elements left after takes
      if(val == 1) num_vals++;
      if(val == -1) break;
    }
    ASSERT_EQ(num_vals, put-take); // there should be p-q copies of V
  }
}


// Basic queue functionality
TEST(QueueTest, BasicQueueOps) {
  my_queue_t q;
  queue_init(&q);

  // Make sure the queue is empty
  ASSERT_TRUE(queue_empty(&q));

  // Make sure taking from the queue returns -1
  ASSERT_EQ(-1, queue_take(&q));

  // Add some items to the queue
  queue_put(&q, 1);
  queue_put(&q, 2);
  queue_put(&q, 3);

  // Make sure the queue is not empty
  ASSERT_FALSE(queue_empty(&q));

  // Take the values from the queue and check them
  ASSERT_EQ(1, queue_take(&q));
  ASSERT_EQ(2, queue_take(&q));
  ASSERT_EQ(3, queue_take(&q));

  // Make sure the queue is empty
  ASSERT_TRUE(queue_empty(&q));

  // Clean up
  queue_destroy(&q);
}
