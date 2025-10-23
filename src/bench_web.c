#include "bench_web.h"
#include "actor.h"
#include "letter.h"
#include "log.h"
#include <math.h>
#include <semaphore.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef enum { Next, ShareRef, ShareLeader, Init, Start, Done } WebEnum;

typedef struct {
  WebEnum type;
  Actor *ref;
  int i;
  int web_size;
} WebMessage;

typedef struct {
  Actor **refs;
  Actor *leader;
  int rounds;
  int web_size;
  int current_number_of_refs;
  int total_number_of_done;
  int number_of_done;
} WebMemory;

static struct timespec start_time, stop_time;
static sem_t done;

static int total_number_of_messages(int web_size, int rounds) {
  int count = 1;
  for (int r = 0; r <= rounds; r++) {
    count += pow(web_size, r + 1);
  }
  return count;
}

static int total_number_of_done(int web_size, int rounds) {
  return pow(web_size, rounds + 1);
}

static void *web_allocator(void *web_size) {
  WebMemory *memory = malloc(sizeof(WebMemory));
  memory->refs = calloc(*(int *)web_size, sizeof(WebMemory *));
  return memory;
}

static void web_deallocator(void *memory) {
  WebMemory *web_memory = (WebMemory *)memory;
  free(web_memory->refs);
  free(web_memory);
}

void leader_actor(Actor *self, Letter *letter) {
  WebMessage *message = letter->message->payload;
  WebMemory *web_memory = self->memory;

  WebMessage *msg;
  switch (message->type) {
  case Init:
    printf("Init leader\n");
    web_memory->rounds = message->i;
    web_memory->web_size = message->web_size;
    web_memory->current_number_of_refs = 0;
    web_memory->number_of_done = 0;
    web_memory->total_number_of_done =
        total_number_of_done(web_memory->web_size, web_memory->rounds);
    break;
  case ShareRef:
    printf("ShareRef\n");
    web_memory->refs[web_memory->current_number_of_refs++] = message->ref;
    break;
  case Start:
    printf("Start\n");
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    for (int i = 0; i < web_memory->web_size; i++) {
      msg = malloc(sizeof(WebMessage));
      msg->type = Next;
      msg->i = message->i;
      async_send(self, web_memory->refs[i], message_make(msg));
    }
    break;
  case Done:
    web_memory->number_of_done++;
    // printf("Done %d\n", web_memory->number_of_done);

    if (web_memory->number_of_done >= web_memory->total_number_of_done) {
      clock_gettime(CLOCK_MONOTONIC, &stop_time);
      long sec = stop_time.tv_sec - start_time.tv_sec;
      long nsec = stop_time.tv_nsec - start_time.tv_nsec;
      double elapsed_s = sec + nsec / 1e9;
      int total_number_of_messages_l =
          total_number_of_messages(web_memory->web_size, web_memory->rounds);
      double rps = (double)total_number_of_messages_l / elapsed_s;
      printf("Web: %d rounds (%d messages) in %.6f s -> %.0f messages per "
             "second\n",
             web_memory->rounds, total_number_of_messages_l, elapsed_s, rps);
      sem_post(&done);
    }
    break;
  case Next:
  case ShareLeader:
    warning("Leader received a message it did not understand\n");
    break;
  }
  free(message);
}

void web_actor(Actor *self, Letter *letter) {
  WebMessage *message = letter->message->payload;
  WebMemory *web_memory = self->memory;

  WebMessage *msg;
  switch (message->type) {
  case Init:
    printf("Init web\n");
    web_memory->current_number_of_refs = 0;
    break;
  case ShareRef:
    printf("ShareRef\n");
    web_memory->refs[web_memory->current_number_of_refs++] = message->ref;
    break;
  case ShareLeader:
    printf("ShareLeader\n");
    web_memory->leader = message->ref;
    break;

  case Next:
    // printf("Next %d\n", message->i);
    if (message->i > 0) {
      for (int i = 0; i < web_memory->current_number_of_refs; i++) {
        msg = malloc(sizeof(WebMessage));
        msg->type = Next;
        msg->i = message->i - 1;
        async_send(self, web_memory->refs[i], message_make(msg));
      }
    } else {
      msg = malloc(sizeof(WebMessage));
      msg->type = Done;
      async_send(self, web_memory->leader, message_make(msg));
    }
    break;
  case Start:
  case Done:
    warning("Web actor received a message it did not understand\n");
    break;
  }
  free(message);
}

void bench_web(ActorUniverse *actor_universe, int web_size, int rounds) {

  // for (int r = 0; r < 11; r++) {
  //   printf("web: %d, rounds: %d, res: %d\n", 4, r,
  //          total_number_of_messages(4, r));
  // }

  if (sem_init(&done, 0, 0) != 0) {
    perror("sem_init");
    return;
  }
  Actor *leader = actor_spawn(actor_universe, &leader_actor, &web_allocator,
                              &web_size, &web_deallocator);
  WebMessage *leader_init_message = malloc(sizeof(WebMessage));
  leader_init_message->type = Init;
  leader_init_message->i = rounds;
  leader_init_message->web_size = web_size;
  async_send(NULL, leader, message_make(leader_init_message));

  Actor *web_actors[web_size];
  for (int i = 0; i < web_size; i++) {
    web_actors[i] =
        actor_spawn(actor_universe, &web_actor, &web_allocator, &web_size, &web_deallocator);
    WebMessage *init_message = malloc(sizeof(WebMessage));
    init_message->type = Init;
    async_send(NULL, web_actors[i], message_make(init_message));
  }

  for (int web_index = 0; web_index < web_size; web_index++) {

    // Share the web actors with the leader
    WebMessage *share_ref_to_leader = malloc(sizeof(WebMessage));
    share_ref_to_leader->type = ShareRef;
    share_ref_to_leader->ref = web_actors[web_index];
    async_send(NULL, leader, message_make(share_ref_to_leader));

    // Share the leader with the web actors
    WebMessage *share_leader = malloc(sizeof(WebMessage));
    share_leader->type = ShareLeader;
    share_leader->ref = leader;
    async_send(NULL, web_actors[web_index], message_make(share_leader));

    // Share the web actors with the web actors
    for (int inner_web_index = 0; inner_web_index < web_size;
         inner_web_index++) {
      WebMessage *share_ref = malloc(sizeof(WebMessage));
      share_ref->type = ShareRef;
      share_ref->ref = web_actors[inner_web_index];
      async_send(NULL, web_actors[web_index], message_make(share_ref));
    }
  }

  WebMessage *start_message = malloc(sizeof(WebMessage));
  start_message->type = Start;
  start_message->i = rounds;
  async_send(NULL, leader, message_make(start_message));

  sem_wait(&done);
  sem_destroy(&done);
}
