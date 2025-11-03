#include "c-actors/actor_universe.h"
#include "c-actors/log.h"
#include "c-actors/threadpool.h"

#include "benchmarks/benchmarks.h"

int main(__attribute__((unused)) int argc,
         __attribute__((unused)) char *argv[]) {
  ActorUniverse *actor_universe = actor_universe_make();
  Threadpool *threadpool = threadpool_make(actor_universe, 4);

  // bench_ping_pong(actor_universe, 500000);
  bench_chain(actor_universe, 0, 500, 500);
  // bench_web(actor_universe, 10, 6);

  LOG("there are currently %d actors in the actor universe\n",
      actor_universe->actor_queue_current_capacity);

  threadpool_stop(threadpool);
  LOG("Threadpool stopped\n");

  threadpool_free(threadpool);
  actor_universe_free(actor_universe);
  return 0;
}
