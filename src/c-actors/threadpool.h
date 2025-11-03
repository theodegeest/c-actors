#ifndef THREADPOOL_H_
#define THREADPOOL_H_

#include "actor_universe.h"

typedef struct {
  pthread_t *threads;
  int number_of_threads;
} Threadpool;

// This function makes a threadpool.
// The ownership is given to the caller.
Threadpool *threadpool_make(ActorUniverse *actor_universe,
                            int number_of_threads);

// This function stops a threadpool.
void threadpool_stop(Threadpool *threadpool);

// This function should be called to free a threadpool.
void threadpool_free(Threadpool *threadpool);

#endif // !THREADPOOL_H_
