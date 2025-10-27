#ifndef ACTOR_H_
#define ACTOR_H_

#include "actor_universe.h"
#include "letter.h"
#include <pthread.h>
#include <stdlib.h>

typedef void (*BehaviourFunction)(struct Actor *, Letter *);
typedef void *(*AllocatorFunction)(void *);
typedef void (*DeallocatorFunction)(void *);

#define MAILBOX_INIT_CAPACITY 10
typedef struct Actor {
  Letter **mailbox;
  volatile int mailbox_current_capacity;
  int mailbox_max_capacity;
  int mailbox_begin_index;
  pthread_mutex_t mailbox_mutex;
  BehaviourFunction behaviour_function;
  DeallocatorFunction deallocator_function;
  void *memory;
} Actor;

// This function spawn a new actor, the ownership is NOT given to the caller.
// The responsability of freeing this memory is given to the actor_universe.
Actor *actor_spawn(ActorUniverse *actor_universe,
                   BehaviourFunction behaviour_function,
                   AllocatorFunction allocator_function, void *allocator_arg,
                   DeallocatorFunction deallocator_function);

// This function should be called to free an actor.
void actor_free(Actor *actor);

// This function will process the mailbox of the actor with a certain
// granularity.
void actor_process(Actor *actor);

// This function will perform a synchronous send where the control flow will
// continue once the receiver has answered the message.
// The return value of this function is the responce value of the receiver.
// The ownership of the message is given to the receiver.
void *sync_send(Actor *sender, Actor *receiver, Message *message);

// This function will perform a asynchronous send where the control flow will
// not be interrupted. The ownership of the message is given to the receiver.
void async_send(Actor *sender, Actor *receiver, Message *message);

#endif // ACTOR_H_
