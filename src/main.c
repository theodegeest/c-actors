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

#include "bench_ping_pong.h"
#include "bench_chain.h"

void my_actor(Actor *self, Letter *letter) {
  printf("%s\n", (char *)letter->message->payload);
}

int main(int argc, char *argv[]) {
  ActorUniverse *actor_universe = make_actor_universe();
  Threadpool *threadpool = make_threadpool(actor_universe);

  // bench_ping_pong(actor_universe);
  bench_chain(actor_universe, 5);

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

  free_threadpool(threadpool);
  free_actor_universe(actor_universe);
  return 0;
}
