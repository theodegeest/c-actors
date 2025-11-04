#include "bench_ping_pong.h"
#include "../c-actors/actor.h"
#include "safe_alloc/safe_alloc.h"
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>

typedef enum { Ping, Pong, Init } PingPongEnum;

typedef struct {
  PingPongEnum type;
  Actor *ref;
  int i;
} PingPongMessage;

typedef struct {
  Actor *ref;
  int rounds;
} PingPongMemory;

static struct timespec start_time, stop_time;
static sem_t done;

static void *ping_pong_allocator(__attribute__((unused)) void *arg) {
  return safe_malloc(sizeof(PingPongMemory));
}

static void ping_pong_deallocator(void *memory) { free(memory); }

void ping_pong_actor(Actor *self, Letter *letter) {
  PingPongMessage *message = letter->message->payload;
  PingPongMemory *ping_pong_memory = self->memory;

  PingPongMessage *msg;
  switch (message->type) {
  case Init:
    printf("Init\n");
    ping_pong_memory->ref = message->ref;
    ping_pong_memory->rounds = message->i;
    break;
  case Ping:
    if (message->i <= 0) {
      clock_gettime(CLOCK_MONOTONIC, &stop_time);
      long sec = stop_time.tv_sec - start_time.tv_sec;
      long nsec = stop_time.tv_nsec - start_time.tv_nsec;
      double elapsed_s = sec + nsec / 1e9;
      double rps = (double)ping_pong_memory->rounds / elapsed_s;
      printf("Ping-pong: %d rounds in %.6f s -> %.0f messages per second\n",
             ping_pong_memory->rounds, elapsed_s, rps);
      sem_post(&done);
    } else {
      // printf("Ping %d\n", message->i);
      msg = safe_malloc(sizeof(PingPongMessage));
      *msg = (PingPongMessage){.type = Pong, .i = message->i - 1};
      async_send(self, ping_pong_memory->ref, message_make(msg, &free));
    }
    break;
  case Pong:
    // printf("Pong %d\n", message->i);
    msg = safe_malloc(sizeof(PingPongMessage));
    *msg = (PingPongMessage){.type = Ping, .i = message->i - 1};
    async_send(self, ping_pong_memory->ref, message_make(msg, &free));
    break;
  }
}

void bench_ping_pong(ActorUniverse *actor_universe, int rounds) {
  if (sem_init(&done, 0, 0) != 0) {
    perror("sem_init");
    return;
  }

  Actor *ping_actor =
      actor_spawn(actor_universe, &ping_pong_actor, &ping_pong_allocator, NULL,
                  &ping_pong_deallocator);
  Actor *pong_actor =
      actor_spawn(actor_universe, &ping_pong_actor, &ping_pong_allocator, NULL,
                  &ping_pong_deallocator);

  PingPongMessage *ping_init_message = safe_malloc(sizeof(PingPongMessage));
  *ping_init_message =
      (PingPongMessage){.type = Init, .ref = pong_actor, .i = rounds};

  PingPongMessage *pong_init_message = safe_malloc(sizeof(PingPongMessage));
  *pong_init_message =
      (PingPongMessage){.type = Init, .ref = ping_actor, .i = rounds};

  async_send(NULL, ping_actor, message_make(ping_init_message, &free));
  async_send(NULL, pong_actor, message_make(pong_init_message, &free));

  PingPongMessage *ping_message = safe_malloc(sizeof(PingPongMessage));
  *ping_message = (PingPongMessage){.type = Ping, .i = rounds};

  clock_gettime(CLOCK_MONOTONIC, &start_time);
  async_send(NULL, ping_actor, message_make(ping_message, &free));

  sem_wait(&done);
  sem_destroy(&done);
}
