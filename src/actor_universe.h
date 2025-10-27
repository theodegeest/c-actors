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

void actor_universe_double_size(ActorUniverse *actor_universe);

int actor_universe_get_available_actor(ActorUniverse *actor_universe);

void actor_universe_reserve_available_actor(ActorUniverse *actor_universe,
                                            int actor_index);

void actor_universe_liberate_available_actor(ActorUniverse *actor_universe,
                                             int actor_index);

#endif // !ACTOR_UNIVERSE_H_
