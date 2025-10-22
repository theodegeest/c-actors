#ifndef ACTOR_H_
#define ACTOR_H_

#include "actor_universe.h"
#include "letter.h"
#include <pthread.h>
#include <stdlib.h>

typedef void (*BehaviourFunction)(struct Actor *, Letter *);

#define MAILBOX_INIT_CAPACITY 10
typedef struct Actor {
  Letter **mailbox;
  volatile int mailbox_current_capacity;
  int mailbox_max_capacity;
  int mailbox_begin_index;
  pthread_mutex_t mailbox_mutex;
  BehaviourFunction behaviour_function;
  void *memory;
} Actor;

Actor *actor_make(BehaviourFunction behaviour_function,
                  size_t actor_memory_size);

Actor *actor_spawn(ActorUniverse *actor_universe,
                   BehaviourFunction behaviour_function,
                   size_t actor_memory_size);

void actor_free(Actor *actor);

void actor_process(Actor *actor);

void sync_send(Actor *sender, Actor *receiver, Message *message);

void async_send(Actor *sender, Actor *receiver, Message *message);

#endif // ACTOR_H_
