#include "bench_chain.h"
#include "../c-actors/actor.h"
#include "../c-actors/letter.h"
#include "../c-actors/log.h"
#include "../safe_alloc/safe_alloc.h"
#include <semaphore.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef enum { Next, Init } ChainEnum;

typedef struct {
  ChainEnum type;
  Actor *next;
  int i;
  int chain_length;
} ChainMessage;

typedef struct {
  Actor *next;
  int rounds;
  int chain_length;
} ChainMemory;

static struct timespec start_time, stop_time;
static sem_t done;

static void *chain_allocator(__attribute__((unused)) void *arg) {
  return safe_malloc(sizeof(ChainMemory));
}

static void chain_deallocator(void *memory) { free(memory); }

void chain_end_actor(Actor *self, Letter *letter) {
  ChainMessage *message = letter->message->payload;
  ChainMemory *chain_memory = self->memory;

  switch (message->type) {
  case Init:
    printf("Init end\n");
    chain_memory->rounds = message->i;
    chain_memory->chain_length = message->chain_length;
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    break;
  case Next:
    if (message->i <= 0) {
      clock_gettime(CLOCK_MONOTONIC, &stop_time);
      long sec = stop_time.tv_sec - start_time.tv_sec;
      long nsec = stop_time.tv_nsec - start_time.tv_nsec;
      double elapsed_s = sec + nsec / 1e9;
      double rps = (double)chain_memory->rounds / elapsed_s;
      printf("Chain: %d rounds in %.6f s -> %.0f messages per second\n",
             chain_memory->rounds, elapsed_s, rps * chain_memory->chain_length);
      sem_post(&done);
    }
    break;
  }
}

static _Atomic int sink = 0;

void chain_actor(Actor *self, Letter *letter) {
  ChainMessage *message = letter->message->payload;
  ChainMemory *chain_memory = self->memory;

  ChainMessage *msg;
  switch (message->type) {
  case Init:
    printf("Init chain\n");
    chain_memory->next = message->next;
    break;
  case Next:
    info("Next %d\n", message->i);
    msg = safe_malloc(sizeof(ChainMessage));
    *msg = (ChainMessage){.type = Next, .i = message->i};
    float local = 0;
    for (int i = 0; i < message->i; i++) {
      local += (double)10000000 / (i + 1);
    }
    atomic_fetch_add(&sink, local);
    async_send(self, chain_memory->next, message_make(msg, &free));
    break;
  }
}

void bench_chain(ActorUniverse *actor_universe, int dummy_count,
                 int chain_length, int rounds) {

  if (sem_init(&done, 0, 0) != 0) {
    perror("sem_init");
    return;
  }

  // Dummy actors
  for (int i = 0; i < dummy_count; i++) {
    actor_spawn(actor_universe, &chain_actor, &chain_allocator, NULL,
                &chain_deallocator);
  }

  Actor *chain_actors[chain_length];
  chain_actors[chain_length - 1] =
      actor_spawn(actor_universe, &chain_end_actor, &chain_allocator, NULL,
                  &chain_deallocator);
  for (int i = chain_length - 2; i >= 0; i--) {
    chain_actors[i] = actor_spawn(actor_universe, &chain_actor,
                                  &chain_allocator, NULL, &chain_deallocator);
    ChainMessage *init_message = safe_malloc(sizeof(ChainMessage));
    *init_message = (ChainMessage){.type = Init, .next = chain_actors[i + 1]};
    async_send(NULL, chain_actors[i], message_make(init_message, &free));
  }

  ChainMessage *end_init_message = safe_malloc(sizeof(ChainMessage));
  *end_init_message =
      (ChainMessage){.type = Init, .i = rounds, .chain_length = chain_length};
  async_send(NULL, chain_actors[chain_length - 1],
             message_make(end_init_message, &free));

  for (int message_index = rounds; message_index >= 0; message_index--) {
    ChainMessage *chain_message = safe_malloc(sizeof(ChainMessage));
    *chain_message = (ChainMessage){.type = Next, .i = message_index};
    async_send(NULL, chain_actors[0], message_make(chain_message, &free));
  }
  // sleep(5);

  sem_wait(&done);
  printf("sink: %d\n", atomic_load(&sink));
  sem_destroy(&done);
}
