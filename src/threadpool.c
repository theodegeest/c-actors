#include "threadpool.h"
#include "actor.h"
#include "log.h"
#include <stdlib.h>

volatile int g_threadpool_continue = 1;

void *threadpool_thread_function(void *void_args) {
  ThreadpoolArgs *args = (ThreadpoolArgs *)void_args;
  while (g_threadpool_continue) {
    pthread_mutex_lock(&args->actor_universe->actor_queue_mutex);

    int available_actor_index =
        actor_universe_get_available_actor(args->actor_universe);

    if (available_actor_index >= 0) {
      // There is an available actor, so reserve it
      actor_universe_reserve_available_actor(args->actor_universe,
                                             available_actor_index);
      Actor *actor = args->actor_universe->actor_queue[available_actor_index];
      pthread_mutex_unlock(&args->actor_universe->actor_queue_mutex);

      // We have reserved the rights to an actor, now process the message that
      // it had received
      process_actor(actor);
      log("thread: %d processed actor: %d\n", args->thread_index,
          available_actor_index);
      actor_universe_liberate_available_actor(args->actor_universe,
                                              available_actor_index);
    } else {
      // No available, so unlock and give a chance to another thread
      pthread_mutex_unlock(&args->actor_universe->actor_queue_mutex);
    }

    // usleep(1000 * 100);
  }
  free(args);
  pthread_exit(NULL);
}

Threadpool *make_threadpool(ActorUniverse *actor_universe,
                            int number_of_threads) {
  Threadpool *threadpool = malloc(sizeof(Threadpool));
  threadpool->threads = calloc(number_of_threads, sizeof(pthread_t));
  threadpool->number_of_threads = number_of_threads;

  for (int thread_index = 0; thread_index < number_of_threads; thread_index++) {

    ThreadpoolArgs *args = malloc(sizeof(ThreadpoolArgs));
    args->thread_index = thread_index;
    args->actor_universe = actor_universe;

    pthread_create(&threadpool->threads[thread_index], NULL,
                   threadpool_thread_function, (void *)args);
  }

  return threadpool;
}

void stop_threadpool(Threadpool *threadpool) {
  g_threadpool_continue = 0;
  for (int thread_index = 0; thread_index < threadpool->number_of_threads;
       thread_index++) {
    void *ret;
    pthread_join(threadpool->threads[thread_index], &ret);
  }
}

void free_threadpool(Threadpool *threadpool) {
  free(threadpool->threads);
  free(threadpool);
}
