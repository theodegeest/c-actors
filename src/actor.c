#include "actor.h"
#include "log.h"
#include <stdio.h>
#include <string.h>

#define PROCESSING_GRANULARITY 10

Actor *make_actor(BehaviourFunction behaviour_function,
                  size_t actor_memory_size) {
  Actor *actor = malloc(sizeof(Actor));
  actor->behaviour_function = behaviour_function;
  actor->mailbox = calloc(MAILBOX_INIT_CAPACITY, sizeof(Letter *));
  actor->mailbox_current_capacity = 0;
  actor->mailbox_max_capacity = MAILBOX_INIT_CAPACITY;
  actor->mailbox_begin_index = 0;
  pthread_mutex_init(&actor->mailbox_mutex, NULL);
  actor->memory = malloc(actor_memory_size);
  return actor;
}

Actor *spawn_actor(ActorUniverse *actor_universe,
                   BehaviourFunction behaviour_function,
                   size_t actor_memory_size) {
  pthread_mutex_lock(&actor_universe->actor_queue_mutex);
  if (actor_universe->actor_queue_current_capacity >=
      actor_universe->actor_queue_max_capacity - 1) {
    actor_universe_double_size(actor_universe);
  }

  Actor *actor = make_actor(behaviour_function, actor_memory_size);

  actor_universe
      ->actor_reservations[actor_universe->actor_queue_current_capacity] = 0;
  actor_universe->actor_queue[actor_universe->actor_queue_current_capacity++] =
      actor;

  pthread_mutex_unlock(&actor_universe->actor_queue_mutex);
  return actor;
}

void free_actor(Actor *actor) {
  free(actor->mailbox);
  free(actor->memory);
  pthread_mutex_destroy(&actor->mailbox_mutex);
  free(actor);
}

static void process_one_letter_of_actor(Actor *actor) {
  // lock your mailbox to read the letter
  pthread_mutex_lock(&actor->mailbox_mutex);

  Letter *letter_p = actor->mailbox[actor->mailbox_begin_index];
  // printf("begin_index: %d, current_cap: %d, max_cap %d, letter_p %p\n",
  //        actor->mailbox_begin_index, actor->mailbox_current_capacity,
  //        actor->mailbox_max_capacity, letter_p);
  if (letter_p == NULL) {
    warning("This is a null pointer\n");
  }
  actor->mailbox_current_capacity--;
  actor->mailbox_begin_index =
      (actor->mailbox_begin_index + 1) % actor->mailbox_max_capacity;

  pthread_mutex_unlock(&actor->mailbox_mutex);

  // We have the letter that we want to process, and other threads are now
  // allowed to put letters in our mailbox

  actor->behaviour_function(actor, letter_p);
  free_letter(letter_p);
}

void process_actor(Actor *actor) {
  for (int i = 0; i < PROCESSING_GRANULARITY; i++) {
    if (actor->mailbox_current_capacity <= 0) {
      break;
    }
    process_one_letter_of_actor(actor);
  }
}

void sync_send(Actor *sender, Actor *receiver, Message *message) {
  // Letter *letter = make_letter(self, message);
  // receiver->behaviour_function(self, letter);
  // free_letter(letter);
}

static void actor_double_mailbox_size(Actor *actor) {
  log("Doubling the size of the mailbox of an actor\n");
  int actor_new_capacity = actor->mailbox_max_capacity * 2;
  Letter **new_mailbox = calloc(actor_new_capacity, sizeof(Letter *));
  if (new_mailbox == NULL) {
    warning("while doubling the size of the mailbox of an actor the allocation "
            "failed!\n");
  }
  for (int i = 0; i < actor->mailbox_max_capacity; i++) {
    int offset_i =
        (i + actor->mailbox_begin_index) % actor->mailbox_max_capacity;
    // printf("max: %d, i: %d, offset_i: %d\n", actor->mailbox_max_capacity, i,
    //        offset_i);
    new_mailbox[i] = actor->mailbox[offset_i];
  }

  free(actor->mailbox);
  actor->mailbox = new_mailbox;
  actor->mailbox_max_capacity = actor_new_capacity;
  actor->mailbox_begin_index = 0;
}

void async_send(Actor *sender, Actor *receiver, Message *message) {
  // lock on the mailbox of this actor to add a letter to it
  pthread_mutex_lock(&receiver->mailbox_mutex);

  if (receiver->mailbox_current_capacity >=
      receiver->mailbox_max_capacity - 1) {
    actor_double_mailbox_size(receiver);
  }

  int letter_index =
      (receiver->mailbox_begin_index + receiver->mailbox_current_capacity) %
      receiver->mailbox_max_capacity;
  // log("put letter '%s' in a mailbox at index: %d\n", (char
  // *)message->payload,
  //     letter_index);
  receiver->mailbox[letter_index] = make_letter(sender, message);
  receiver->mailbox_current_capacity++;

  pthread_mutex_unlock(&receiver->mailbox_mutex);
}
