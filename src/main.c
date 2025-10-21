#include "file.h"
#include <pthread.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct {
  void *payload;
} Message;

typedef struct {
  struct Actor *sender;
  Message *message;
} Letter;

typedef void (*BehaviourFunction)(struct Actor *, Letter *);

#define MAILBOX_MAX_CAPACITY 100
typedef struct Actor {
  Letter **mailbox;
  volatile int mailbox_capacity;
  int mailbox_begin_index;
  pthread_mutex_t mailbox_mutex;
  BehaviourFunction behaviour_function;
  void *memory;
} Actor;

#define THREADPOOL_MAX_SIZE 8
typedef struct {
  pthread_t threads[THREADPOOL_MAX_SIZE];
} Threadpool;

#define ACTOR_QUEUE_INIT_CAPACITY 5
typedef struct {
  Actor **actor_queue;
  char *actor_reservations;
  int actor_queue_max_capacity;
  int actor_queue_current_capacity;
  int actor_index;
  pthread_mutex_t actor_queue_mutex;
} ActorUniverse;

typedef struct {
  int thread_index;
  ActorUniverse *actor_universe;
} ThreadpoolArgs;

// #define LOGGING
#ifdef LOGGING
/* variadic macro that forwards everything after the prefix to printf.
   Use do{...}while(0) so it behaves like a statement in all contexts. */
#define log(...)                                                               \
  do {                                                                         \
    printf("LOGGING: " __VA_ARGS__);                                           \
  } while (0)
#else
/* compile-time disabled: no code, no argument evaluation */
#define log(...) ((void)0)
#endif

#define LOG_WARNING
#ifdef LOG_WARNING
#define warning(...)                                                           \
  do {                                                                         \
    printf("WARNING: " __VA_ARGS__);                                           \
  } while (0)
#else
#define warning(...) ((void)0)
#endif
void process_actor(Actor *actor);

ActorUniverse *make_actor_universe() {
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

void free_actor_universe(ActorUniverse *actor_universe) {
  free(actor_universe->actor_queue);
  free(actor_universe->actor_reservations);
  pthread_mutex_destroy(&actor_universe->actor_queue_mutex);
  free(actor_universe);
}

void actor_universe_double_size(ActorUniverse *actor_universe) {
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
    if (!actor_universe->actor_reservations[offset_actor_index] &&
        actor_universe->actor_queue[offset_actor_index]->mailbox_capacity > 0) {
      // This actor is not reserved and has mail, so it is available
      available_actor_index = offset_actor_index;
      actor_universe->actor_index =
          (offset_actor_index + 1) %
          actor_universe->actor_queue_current_capacity;
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

volatile int g_threadpool_continue = 1;

void *threadpool_thread_function(void *void_args) {
  ThreadpoolArgs *args = (ThreadpoolArgs *)void_args;
  while (g_threadpool_continue) {
    pthread_mutex_lock(&args->actor_universe->actor_queue_mutex);

    int available_actor_index =
        actor_universe_get_available_actor(args->actor_universe);

    if (available_actor_index >= 0) {
      // There is an available actor, so reserve it
      actor_universe_reserve_available_actor(args->actor_universe,
                                             available_actor_index);
      Actor *actor = args->actor_universe->actor_queue[available_actor_index];
      pthread_mutex_unlock(&args->actor_universe->actor_queue_mutex);

      // We have reserved the rights to an actor, now process the message that
      // it had received
      process_actor(actor);
      log("thread: %d processed actor: %d\n", args->thread_index,
          available_actor_index);
      actor_universe_liberate_available_actor(args->actor_universe,
                                              available_actor_index);
    } else {
      // No available, so unlock and give a chance to another thread
      pthread_mutex_unlock(&args->actor_universe->actor_queue_mutex);
    }

    // usleep(1000 * 100);
  }
  free(args);
  pthread_exit(NULL);
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

void stop_threadpool(Threadpool *threadpool) {
  g_threadpool_continue = 0;
  for (int thread_index = 0; thread_index < THREADPOOL_MAX_SIZE;
       thread_index++) {
    void *ret;
    pthread_join(threadpool->threads[thread_index], &ret);
  }
}

void free_threadpool(Threadpool *threadpool) { free(threadpool); }

Message *make_message(void *payload) {
  Message *message = malloc(sizeof(Message));
  message->payload = payload;
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

void process_actor(Actor *actor) {
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
    log("put letter '%s' in a mailbox at index: %d\n", (char *)message->payload,
        letter_index);
    receiver->mailbox[letter_index] = make_letter(sender, message);
    receiver->mailbox_capacity++;
  } else {
    warning("WARNING: a message '%s' has been dropped!\n",
            (char *)message->payload);
  }

  pthread_mutex_unlock(&receiver->mailbox_mutex);
}

void my_actor(Actor *self, Letter *letter) {
  printf("%s\n", (char *)letter->message->payload);
}

typedef enum { Ping, Pong, Init } PingPongEnum;

typedef struct {
  PingPongEnum type;
  Actor *ref;
  int i;
} PingPongMessage;

typedef struct {
  Actor *ref;
} PingPongMemory;

void ping_pong_actor(Actor *self, Letter *letter) {
  PingPongMessage *message = letter->message->payload;
  PingPongMemory *ping_pong_memory = self->memory;

  PingPongMessage *msg;
  switch (message->type) {
  case Init:
    printf("Init\n");
    ping_pong_memory->ref = message->ref;
    break;
  case Ping:
    printf("Ping %d\n", message->i);
    msg = malloc(sizeof(PingPongMessage));
    msg->type = Pong;
    msg->i = message->i + 1;
    async_send(self, ping_pong_memory->ref, make_message(msg));
    break;
  case Pong:
    printf("Pong %d\n", message->i);
    msg = malloc(sizeof(PingPongMessage));
    msg->type = Ping;
    msg->i = message->i + 1;
    async_send(self, ping_pong_memory->ref, make_message(msg));
    break;
  }
  free(message);
}

int main(int argc, char *argv[]) {
  ActorUniverse *actor_universe = make_actor_universe();
  Threadpool *threadpool = make_threadpool(actor_universe);

  Actor *ping_actor =
      spawn_actor(actor_universe, &ping_pong_actor, sizeof(PingPongMemory));
  Actor *pong_actor =
      spawn_actor(actor_universe, &ping_pong_actor, sizeof(PingPongMemory));

  PingPongMessage *ping_init_message = malloc(sizeof(PingPongMessage));
  ping_init_message->type = Init;
  ping_init_message->ref = pong_actor;
  PingPongMessage *pong_init_message = malloc(sizeof(PingPongMessage));
  pong_init_message->type = Init;
  pong_init_message->ref = ping_actor;

  async_send(NULL, ping_actor, make_message(ping_init_message));
  async_send(NULL, pong_actor, make_message(pong_init_message));

  PingPongMessage *ping_message = malloc(sizeof(PingPongMessage));
  ping_message->type = Ping;
  ping_message->i = 0;
  async_send(NULL, ping_actor, make_message(ping_message));

  // Actor *actor = spawn_actor(actor_universe, &my_actor);
  // for (int i = 0; i < 100; i++) {
  //   char buffer[64];
  //   snprintf(buffer, sizeof(buffer), "Hello World! %d", i);
  //   char *copy = strdup(buffer);
  //   Message *message = make_message(copy);
  //   async_send(NULL, actor, message);
  //   // usleep(1000);
  // }

  sleep(1);
  log("there are currently %d actors in the actor universe\n",
      actor_universe->actor_queue_current_capacity);

  stop_threadpool(threadpool);
  log("Threadpool stopped\n");

  free_actor(ping_actor);
  free_actor(pong_actor);
  free_threadpool(threadpool);
  free_actor_universe(actor_universe);
  return 0;
}
