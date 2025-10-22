#include "bench_ping_pong.h"
#include "actor.h"
#include <semaphore.h>
#include <stdio.h>

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
      msg = malloc(sizeof(PingPongMessage));
      msg->type = Pong;
      msg->i = message->i - 1;
      async_send(self, ping_pong_memory->ref, make_message(msg));
    }
    break;
  case Pong:
    // printf("Pong %d\n", message->i);
    msg = malloc(sizeof(PingPongMessage));
    msg->type = Ping;
    msg->i = message->i - 1;
    async_send(self, ping_pong_memory->ref, make_message(msg));
    break;
  }
  free(message);
}

void bench_ping_pong(ActorUniverse *actor_universe, int rounds) {
  if (sem_init(&done, 0, 0) != 0) { 
    perror("sem_init");
    return;
  }

  Actor *ping_actor =
      spawn_actor(actor_universe, &ping_pong_actor, sizeof(PingPongMemory));
  Actor *pong_actor =
      spawn_actor(actor_universe, &ping_pong_actor, sizeof(PingPongMemory));

  PingPongMessage *ping_init_message = malloc(sizeof(PingPongMessage));
  ping_init_message->type = Init;
  ping_init_message->ref = pong_actor;
  ping_init_message->i = rounds;

  PingPongMessage *pong_init_message = malloc(sizeof(PingPongMessage));
  pong_init_message->type = Init;
  pong_init_message->ref = ping_actor;
  ping_init_message->i = rounds;

  async_send(NULL, ping_actor, make_message(ping_init_message));
  async_send(NULL, pong_actor, make_message(pong_init_message));

  PingPongMessage *ping_message = malloc(sizeof(PingPongMessage));
  ping_message->type = Ping;
  ping_message->i = rounds;

  clock_gettime(CLOCK_MONOTONIC, &start_time);
  async_send(NULL, ping_actor, make_message(ping_message));

  sem_wait(&done);
  sem_destroy(&done);
}
