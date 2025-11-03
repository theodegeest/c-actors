#include "actor.h"
#include "actor_universe.h"
#include "letter.h"
#include "log.h"
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PROCESSING_GRANULARITY 100

Actor *actor_make(BehaviourFunction behaviour_function,
                  AllocatorFunction allocator_function, void *allocator_arg,
                  DeallocatorFunction deallocator_function) {
  Actor *actor = malloc(sizeof(Actor));
  actor->behaviour_function = behaviour_function;
  actor->deallocator_function = deallocator_function;
  actor->mailbox = calloc(MAILBOX_INIT_CAPACITY, sizeof(Letter *));
  actor->mailbox_current_capacity = 0;
  actor->mailbox_max_capacity = MAILBOX_INIT_CAPACITY;
  actor->mailbox_begin_index = 0;
  pthread_mutex_init(&actor->mailbox_mutex, NULL);
  actor->memory = allocator_function(allocator_arg);
  return actor;
}

Actor *actor_spawn(ActorUniverse *actor_universe,
                   BehaviourFunction behaviour_function,
                   AllocatorFunction allocator_function, void *allocator_arg,
                   DeallocatorFunction deallocator_function) {

  Actor *actor = actor_make(behaviour_function, allocator_function,
                            allocator_arg, deallocator_function);

  actor_universe_add_actor(actor_universe, actor);
  return actor;
}

void actor_free(Actor *actor) {
  // free all the letter that might still be in the mailbox.

  for (int i = 0; i < actor->mailbox_current_capacity; i++) {
    letter_free(actor->mailbox[i]);
  }

  free(actor->mailbox);
  actor->deallocator_function(actor->memory);
  pthread_mutex_destroy(&actor->mailbox_mutex);
  free(actor);
}

static void actor_process_one_letter(Actor *actor) {
  // lock your mailbox to read the letter
  pthread_mutex_lock(&actor->mailbox_mutex);

  Letter *letter_p = actor->mailbox[actor->mailbox_begin_index];
  // LOG("begin_index: %d, current_cap: %d, max_cap %d, letter_p %p\n",
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
  if (letter_p->sync_letter) {
    sem_post(&letter_p->sync_semaphore);
  }
  letter_free(letter_p);
}

void actor_process(Actor *actor) {
  for (int i = 0; i < PROCESSING_GRANULARITY; i++) {
    if (actor->mailbox_current_capacity <= 0) {
      break;
    }
    actor_process_one_letter(actor);
  }
}

static void actor_double_mailbox_size(Actor *actor) {
  LOG("Doubling the size of the mailbox of an actor\n");
  int actor_new_capacity = actor->mailbox_max_capacity * 2;
  Letter **new_mailbox = calloc(actor_new_capacity, sizeof(Letter *));
  if (new_mailbox == NULL) {
    warning("while doubling the size of the mailbox of an actor the allocation "
            "failed!\n");
  }
  for (int i = 0; i < actor->mailbox_max_capacity; i++) {
    int offset_i =
        (i + actor->mailbox_begin_index) % actor->mailbox_max_capacity;
    // LOG("max: %d, i: %d, offset_i: %d\n", actor->mailbox_max_capacity, i,
    //        offset_i);
    new_mailbox[i] = actor->mailbox[offset_i];
  }

  free(actor->mailbox);
  actor->mailbox = new_mailbox;
  actor->mailbox_max_capacity = actor_new_capacity;
  actor->mailbox_begin_index = 0;
}

static void put_letter_in_mailbox(Actor *receiver,
                                  Letter *letter) {
  // lock on the mailbox of this actor to add a letter to it
  pthread_mutex_lock(&receiver->mailbox_mutex);

  if (receiver->mailbox_current_capacity >=
      receiver->mailbox_max_capacity - 1) {
    actor_double_mailbox_size(receiver);
  }

  int letter_index =
      (receiver->mailbox_begin_index + receiver->mailbox_current_capacity) %
      receiver->mailbox_max_capacity;
  // LOG("put letter '%s' in a mailbox at index: %d\n", (char
  // *)message->payload,
  //     letter_index);
  receiver->mailbox[letter_index] = letter;
  receiver->mailbox_current_capacity++;

  pthread_mutex_unlock(&receiver->mailbox_mutex);
}

void *sync_send(Actor *sender, Actor *receiver, Message *message) {
  void *return_value;
  Letter *letter = letter_make(sender, message, &return_value);
  put_letter_in_mailbox(receiver, letter);
  sem_wait(&letter->sync_semaphore);
  return return_value;
}

void async_send(Actor *sender, Actor *receiver, Message *message) {
  Letter *letter = letter_make(sender, message, NULL);
  put_letter_in_mailbox(receiver, letter);
}
