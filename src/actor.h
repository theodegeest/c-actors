#ifndef ACTOR_H_
#define ACTOR_H_

#include "actor_universe.h"
#include "letter.h"
#include <pthread.h>
#include <stdlib.h>

typedef void (*BehaviourFunction)(struct Actor *, Letter *);

#define MAILBOX_MAX_CAPACITY 10000
typedef struct Actor {
  Letter **mailbox;
  volatile int mailbox_capacity;
  int mailbox_begin_index;
  pthread_mutex_t mailbox_mutex;
  BehaviourFunction behaviour_function;
  void *memory;
} Actor;

Actor *make_actor(BehaviourFunction behaviour_function,
                  size_t actor_memory_size);

Actor *spawn_actor(ActorUniverse *actor_universe,
                   BehaviourFunction behaviour_function,
                   size_t actor_memory_size);

void free_actor(Actor *actor);

void process_actor(Actor *actor);

void sync_send(Actor *sender, Actor *receiver, Message *message);

void async_send(Actor *sender, Actor *receiver, Message *message);

#endif // ACTOR_H_
