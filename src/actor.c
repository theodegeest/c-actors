#include "actor.h"
#include "log.h"

#define PROCESSING_GRANULARITY 10

Actor *make_actor(BehaviourFunction behaviour_function,
                  size_t actor_memory_size) {
  Actor *actor = malloc(sizeof(Actor));
  actor->behaviour_function = behaviour_function;
  actor->mailbox = calloc(MAILBOX_MAX_CAPACITY, sizeof(Letter *));
  actor->mailbox_capacity = 0;
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
      actor_universe->actor_queue_max_capacity) {
    actor_universe_double_size(actor_universe);
  }

  Actor *actor = make_actor(behaviour_function, actor_memory_size);

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

  Letter *letter = actor->mailbox[actor->mailbox_begin_index];
  actor->mailbox_capacity--;
  actor->mailbox_begin_index =
      (actor->mailbox_begin_index + 1) % MAILBOX_MAX_CAPACITY;

  pthread_mutex_unlock(&actor->mailbox_mutex);

  // We have the letter that we want to process, and other threads are now
  // allowed to put letters in our mailbox

  actor->behaviour_function(actor, letter);
  free_letter(letter);
}

void process_actor(Actor *actor) {
  for (int i = 0; i < PROCESSING_GRANULARITY; i++) {
    if (actor->mailbox_capacity <= 0) {
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

void async_send(Actor *sender, Actor *receiver, Message *message) {
  // lock on the mailbox of this actor to add a letter to it
  pthread_mutex_lock(&receiver->mailbox_mutex);

  if (receiver->mailbox_capacity < MAILBOX_MAX_CAPACITY) {
    int letter_index =
        (receiver->mailbox_begin_index + receiver->mailbox_capacity) %
        MAILBOX_MAX_CAPACITY;
    // log("put letter '%s' in a mailbox at index: %d\n", (char
    // *)message->payload,
    //     letter_index);
    receiver->mailbox[letter_index] = make_letter(sender, message);
    receiver->mailbox_capacity++;
  } else {
    warning("WARNING: a message has been dropped!\n");
  }

  pthread_mutex_unlock(&receiver->mailbox_mutex);
}
