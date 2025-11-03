#ifndef ACTOR_UNIVERSE_H_
#define ACTOR_UNIVERSE_H_

#include <pthread.h>

struct Actor;

#define ACTOR_QUEUE_INIT_CAPACITY 5
typedef struct {
  struct Actor **actor_queue;
  char *actor_reservations;
  int actor_queue_max_capacity;
  int actor_queue_current_capacity;
  int actor_index;
  pthread_mutex_t actor_queue_mutex;
} ActorUniverse;

// This function will make an actor universe that will manage to actor system.
// The ownership is given to the caller.
ActorUniverse *actor_universe_make();

// This function should be used to free an actor universe.
void actor_universe_free(ActorUniverse *actor_universe);

// This function adds an actor to the given actor universe.
// This will make sure that the actor can be scheduled on threads.
void actor_universe_add_actor(ActorUniverse *actor_universe, struct Actor *actor);

// This function will give back an index of an available actor.
// If no actor is available then it will return -1.
int actor_universe_get_available_actor(ActorUniverse *actor_universe);

// This function will reserve an actor which makes sure it cannot be scheduled on another thread.
void actor_universe_reserve_actor(ActorUniverse *actor_universe,
                                            int actor_index);

// This function will liberate an actor which makes it able to scheduled on another thread.
void actor_universe_liberate_actor(ActorUniverse *actor_universe,
                                             int actor_index);

#endif // !ACTOR_UNIVERSE_H_
