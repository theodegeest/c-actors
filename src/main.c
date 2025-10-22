#include <pthread.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "actor.h"
#include "actor_universe.h"
#include "letter.h"
#include "log.h"
#include "message.h"
#include "threadpool.h"

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

#define ROUNDS 10000000
struct timespec start_time, stop_time;

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
    if (message->i <= 0) {
      clock_gettime(CLOCK_MONOTONIC, &stop_time);
      long sec = stop_time.tv_sec - start_time.tv_sec;
      long nsec = stop_time.tv_nsec - start_time.tv_nsec;
      double elapsed_s = sec + nsec / 1e9;
      double rps = (double)ROUNDS / elapsed_s;
      printf("Ping-pong: %d rounds in %.6f s -> %.0f messages per second\n",
             ROUNDS, elapsed_s, rps);
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
  ping_message->i = ROUNDS;

  clock_gettime(CLOCK_MONOTONIC, &start_time);
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

  sleep(3);
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
