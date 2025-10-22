#include <pthread.h>
#include <stddef.h>
#include <unistd.h>

#include "actor_universe.h"
#include "log.h"
#include "threadpool.h"

#include "bench_chain.h"
#include "bench_ping_pong.h"
#include "bench_web.h"

int main(int argc, char *argv[]) {
  ActorUniverse *actor_universe = actor_universe_make();
  Threadpool *threadpool = threadpool_make(actor_universe, 4);

  // bench_ping_pong(actor_universe, 500000);
  bench_chain(actor_universe, 500, 500);
  // bench_web(actor_universe, 10, 6);

  LOG("there are currently %d actors in the actor universe\n",
      actor_universe->actor_queue_current_capacity);

  threadpool_stop(threadpool);
  LOG("Threadpool stopped\n");

  threadpool_free(threadpool);
  actor_universe_free(actor_universe);
  return 0;
}
