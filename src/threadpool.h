#ifndef THREADPOOL_H_
#define THREADPOOL_H_

#include "actor_universe.h"

#define THREADPOOL_MAX_SIZE 2
typedef struct {
  pthread_t threads[THREADPOOL_MAX_SIZE];
} Threadpool;

typedef struct {
  int thread_index;
  ActorUniverse *actor_universe;
} ThreadpoolArgs;

void *threadpool_thread_function(void *void_args);

Threadpool *make_threadpool(ActorUniverse *actor_universe);

void stop_threadpool(Threadpool *threadpool);

void free_threadpool(Threadpool *threadpool);

#endif // !THREADPOOL_H_
