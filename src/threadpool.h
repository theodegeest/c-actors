#ifndef THREADPOOL_H_
#define THREADPOOL_H_

#include "actor_universe.h"

typedef struct {
  pthread_t *threads;
  int number_of_threads;
} Threadpool;

typedef struct {
  int thread_index;
  ActorUniverse *actor_universe;
} ThreadpoolArgs;

void *threadpool_thread_function(void *void_args);

Threadpool *threadpool_make(ActorUniverse *actor_universe,
                            int number_of_threads);

void threadpool_stop(Threadpool *threadpool);

void threadpool_free(Threadpool *threadpool);

#endif // !THREADPOOL_H_
