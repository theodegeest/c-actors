#include "file.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct {
  char *str;
} Message;

typedef struct {
  struct Actor *sender;
  Message *message;
} Letter;

#define MAILBOX_MAX_SIZE 5

typedef struct Actor {
  Message *mailbox[MAILBOX_MAX_SIZE];
  int mailbox_size;
  pthread_mutex_t mailbox_mutex;
  void (*behaviour_function)(struct Actor *, Letter *);
} Actor;

#define THREADPOOL_MAX_SIZE 2
typedef struct {
  pthread_t threads[THREADPOOL_MAX_SIZE];
} Threadpool;

#define ACTOR_QUEUE_INIT_CAPACITY 5
typedef struct {
  Actor *actor_queue;
  int actor_queue_max_capacity;
  int actor_queue_current_capacity;
  int actor_index;
  pthread_mutex_t actor_queue_mutex;
} ActorUniverse;

typedef struct {
  int thread_index;
  ActorUniverse *actor_universe;
} ThreadpoolArgs;

ActorUniverse *make_actor_universe() {
  ActorUniverse *actor_universe = malloc(sizeof(ActorUniverse));

  actor_universe->actor_queue =
      calloc(ACTOR_QUEUE_INIT_CAPACITY, sizeof(Actor *));
  actor_universe->actor_queue_max_capacity = ACTOR_QUEUE_INIT_CAPACITY;
  actor_universe->actor_queue_current_capacity = 0;
  actor_universe->actor_index = 0;
  pthread_mutex_init(&actor_universe->actor_queue_mutex, NULL);

  return actor_universe;
}

void free_actor_universe(ActorUniverse *actor_universe) {
  free(actor_universe->actor_queue);
  pthread_mutex_destroy(&actor_universe->actor_queue_mutex);
  free(actor_universe);
}

volatile int g_threadpool_continue = 1;

void *threadpool_thread_function(void *void_args) {
  ThreadpoolArgs *args = (ThreadpoolArgs *)void_args;
  while (g_threadpool_continue) {
    printf("thread: %d\n", args->thread_index);
    sleep(1);
  }
  return 0;
}

Threadpool *make_threadpool(ActorUniverse *actor_universe) {
  Threadpool *threadpool = malloc(sizeof(Threadpool));

  for (int thread_index = 0; thread_index < THREADPOOL_MAX_SIZE;
       thread_index++) {

    ThreadpoolArgs *args = malloc(sizeof(ThreadpoolArgs));
    args->thread_index = thread_index;
    args->actor_universe = actor_universe;

    pthread_create(&threadpool->threads[thread_index], NULL,
                   threadpool_thread_function, (void *)args);
  }

  return threadpool;
}

void free_threadpool(Threadpool *threadpool) {
  for (int thread_index = 0; thread_index < THREADPOOL_MAX_SIZE;
       thread_index++) {
    pthread_cancel(threadpool->threads[thread_index]);
  }
  free(threadpool);
}

Message *make_message(char *str) {
  Message *message = malloc(sizeof(Message));
  message->str = str;
  return message;
}

void free_message(Message *message) { free(message); }

Letter *make_letter(Actor *sender, Message *message) {
  Letter *letter = malloc(sizeof(Letter));
  letter->sender = sender;
  letter->message = message;
  return letter;
}

void free_letter(Letter *letter) {
  free_message(letter->message);
  free(letter);
}

Actor *make_actor(void (*behaviour_function)(Actor *, Letter *)) {
  Actor *actor = malloc(sizeof(Actor));
  actor->behaviour_function = behaviour_function;
  return actor;
}

void free_actor(Actor *actor) { free(actor); }

void my_actor(Actor *self, Letter *letter) {
  printf("%s\n", letter->message->str);
}

void sync_send(Actor *self, Actor *receiver, Message *message) {
  Letter *letter = make_letter(self, message);
  receiver->behaviour_function(self, letter);
  free_letter(letter);
}

void async_send(Actor *receiver, Message *message) {}

int main(int argc, char *argv[]) {
  ActorUniverse *actor_universe = make_actor_universe();
  Threadpool *threadpool = make_threadpool(actor_universe);
  Actor *actor = make_actor(&my_actor);
  Message *message = make_message("Hello World!");
  sync_send(NULL, actor, message);

  sleep(2);

  free_actor(actor);
  free_threadpool(threadpool);
  free_actor_universe(actor_universe);
  return 0;
}
