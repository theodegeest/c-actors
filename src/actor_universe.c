#include "actor_universe.h"
#include "actor.h"
#include "log.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

ActorUniverse *actor_universe_make() {
  ActorUniverse *actor_universe = malloc(sizeof(ActorUniverse));

  actor_universe->actor_queue =
      calloc(ACTOR_QUEUE_INIT_CAPACITY, sizeof(Actor *));
  actor_universe->actor_reservations =
      calloc(ACTOR_QUEUE_INIT_CAPACITY, sizeof(char));
  actor_universe->actor_queue_max_capacity = ACTOR_QUEUE_INIT_CAPACITY;
  actor_universe->actor_queue_current_capacity = 0;
  actor_universe->actor_index = 0;
  pthread_mutex_init(&actor_universe->actor_queue_mutex, NULL);

  return actor_universe;
}

void actor_universe_free(ActorUniverse *actor_universe) {
  for (int i = 0; i < actor_universe->actor_queue_current_capacity; i++) {
    actor_free(actor_universe->actor_queue[i]);
  }
  free(actor_universe->actor_queue);
  free(actor_universe->actor_reservations);
  pthread_mutex_destroy(&actor_universe->actor_queue_mutex);
  free(actor_universe);
}

void actor_universe_double_size(ActorUniverse *actor_universe) {
  LOG("Doubling universe, cur: %d, max %d\n",
      actor_universe->actor_queue_current_capacity,
      actor_universe->actor_queue_max_capacity);
  int actor_queue_new_capacity = actor_universe->actor_queue_max_capacity * 2;
  actor_universe->actor_queue = realloc(
      actor_universe->actor_queue, sizeof(Actor *) * actor_queue_new_capacity);
  actor_universe->actor_reservations =
      realloc(actor_universe->actor_reservations,
              sizeof(char) * actor_queue_new_capacity);
  actor_universe->actor_queue_max_capacity = actor_queue_new_capacity;
}

int actor_universe_get_available_actor(ActorUniverse *actor_universe) {
  int available_actor_index = -1;
  for (int actor_index = 0;
       actor_index < actor_universe->actor_queue_current_capacity;
       actor_index++) {
    int offset_actor_index = (actor_index + actor_universe->actor_index) %
                             actor_universe->actor_queue_current_capacity;
    // printf("id: %d, res: %d, mailbox_cap: %d\n", offset_actor_index,
    //        actor_universe->actor_reservations[offset_actor_index],
    //        actor_universe->actor_queue[offset_actor_index]
    //            ->mailbox_current_capacity);
    if (!actor_universe->actor_reservations[offset_actor_index] &&
        actor_universe->actor_queue[offset_actor_index]
                ->mailbox_current_capacity > 0) {
      // This actor is not reserved and has mail, so it is available
      available_actor_index = offset_actor_index;
      actor_universe->actor_index =
          (offset_actor_index + 1) %
          actor_universe->actor_queue_current_capacity;
      break;
    }
  }
  return available_actor_index;
}

void actor_universe_reserve_available_actor(ActorUniverse *actor_universe,
                                            int actor_index) {
  actor_universe->actor_reservations[actor_index] = 1;
}

void actor_universe_liberate_available_actor(ActorUniverse *actor_universe,
                                             int actor_index) {
  actor_universe->actor_reservations[actor_index] = 0;
}
