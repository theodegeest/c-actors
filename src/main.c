#include <stdlib.h>
#include <unistd.h>

#include "c-actors/actor_universe.h"
#include "c-actors/log.h"
#include "c-actors/threadpool.h"

#include "benchmarks/benchmarks.h"

static int threads = 4;
static unsigned long long total_iters = 1000000;

static inline double time_diff(struct timespec a, struct timespec b) {
  return (a.tv_sec - b.tv_sec) + (a.tv_nsec - b.tv_nsec) / 1e9;
}

int main(__attribute__((unused)) int argc,
         __attribute__((unused)) char *argv[]) {

  int opt;

  while ((opt = getopt(argc, argv, "t:n:c:")) != -1) {
    switch (opt) {
    case 't':
      threads = atoi(optarg);
      break;
    case 'n':
      total_iters = strtoull(optarg, NULL, 10);
      break;
    default:
      fprintf(stderr, "Usage: %s [-t threads] [-n total_iters] \n", argv[0]);
      exit(1);
    }
  }

  ActorUniverse *actor_universe = actor_universe_make();
  Threadpool *threadpool = threadpool_make(actor_universe, threads);

  struct timespec start, end;
  clock_gettime(CLOCK_MONOTONIC, &start);

  // bench_ping_pong(actor_universe, 5000000);
  // bench_chain(actor_universe, 0, 500, 5000);
  bench_web(actor_universe, 12, 5);

  clock_gettime(CLOCK_MONOTONIC, &end);

  double runtime = time_diff(end, start);

  LOG("there are currently %d actors in the actor universe\n",
      actor_universe->actor_queue_current_capacity);

  threadpool_stop(threadpool);
  LOG("Threadpool stopped\n");

  threadpool_free(threadpool);
  actor_universe_free(actor_universe);

  printf("duration: %.6f\n", runtime);
  return 0;
}
