#include "bench_chain.h"
#include "actor.h"
#include "letter.h"
#include "log.h"
#include <stdio.h>

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
      printf("Chain: %d rounds in %.6f s -> %.0f messages per second\n", chain_memory->rounds,
             elapsed_s, rps * chain_memory->chain_length);
    }
    break;
  }
  free(message);
}

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
    msg = malloc(sizeof(ChainMessage));
    msg->type = Next;
    msg->i = message->i;
    async_send(self, chain_memory->next, make_message(msg));
    break;
  }
  free(message);
}

void bench_chain(ActorUniverse *actor_universe, int chain_length, int rounds) {
  Actor *chain_actors[chain_length];
  chain_actors[chain_length - 1] =
      spawn_actor(actor_universe, &chain_end_actor, sizeof(ChainMemory));
  for (int i = chain_length - 2; i >= 0; i--) {
    chain_actors[i] =
        spawn_actor(actor_universe, &chain_actor, sizeof(ChainMemory));
    ChainMessage *init_message = malloc(sizeof(ChainMessage));
    init_message->type = Init;
    init_message->next = chain_actors[i + 1];
    async_send(NULL, chain_actors[i], make_message(init_message));
  }

  ChainMessage *end_init_message = malloc(sizeof(ChainMessage));
  end_init_message->type = Init;
  end_init_message->i = rounds;
  end_init_message->chain_length = chain_length;
  async_send(NULL, chain_actors[chain_length - 1], make_message(end_init_message));

  for (int message_index = rounds; message_index >= 0; message_index--) {
    ChainMessage *chain_message = malloc(sizeof(ChainMessage));
    chain_message->type = Next;
    chain_message->i = message_index;
    async_send(NULL, chain_actors[0], make_message(chain_message));
  }
}
