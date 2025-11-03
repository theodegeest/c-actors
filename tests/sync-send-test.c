#include <criterion/criterion.h>
#include <semaphore.h>
#include <stdlib.h>

#include "../src/c-actors/actor.h"
#include "../src/c-actors/actor_universe.h"
#include "../src/c-actors/log.h"
#include "../src/c-actors/message.h"
#include "../src/c-actors/threadpool.h"

typedef enum { Get } ClientEnum;
typedef enum { GetValue } ServerEnum;

typedef struct {
  ClientEnum type;
  int ret;
} ClientMessage;

typedef struct {
  ServerEnum type;
} ServerMessage;

typedef struct {
  Actor *server;
} ClientMemory;

typedef struct {
  int information;
} ServerMemory;

static void *client_allocator(void *server) {
  ClientMemory *memory = malloc(sizeof(ClientMemory));
  memory->server = server;
  return memory;
}

static void *server_allocator(void *value) {
  ServerMemory *memory = malloc(sizeof(ServerMemory));
  memory->information = *(int *)value;
  return memory;
}

static void client_deallocator(void *memory) { free(memory); }
static void server_deallocator(void *memory) { free(memory); }

static sem_t done;
static volatile int result;

void client_actor(Actor *self, Letter *letter) {
  ClientMessage *message = letter->message->payload;
  ClientMemory *memory = self->memory;

  ServerMessage *msg;
  switch (message->type) {
  case Get:
    // printf("Pong %d\n", message->i);
    msg = malloc(sizeof(ServerMessage));
    *msg = (ServerMessage){.type = GetValue};
    int *ret = sync_send(self, memory->server, message_make(msg, &free));
    result = *ret;
    free(ret);
    sem_post(&done);
    break;
  }
}

void server_actor(Actor *self, Letter *letter) {
  ServerMessage *message = letter->message->payload;
  ServerMemory *memory = self->memory;

  int *ret;
  switch (message->type) {
  case GetValue:
    ret = malloc(sizeof(int));
    *ret = memory->information;
    *letter->sync_return = ret;
    break;
  }
}

Test(sync_server_client, test1) {
  if (sem_init(&done, 0, 0) != 0) {
    perror("sem_init");
    return;
  }

  ActorUniverse *actor_universe = actor_universe_make();
  Threadpool *threadpool = threadpool_make(actor_universe, 8);

  int value = 42;

  Actor *server = actor_spawn(actor_universe, &server_actor, &server_allocator,
                              &value, &server_deallocator);
  Actor *client = actor_spawn(actor_universe, &client_actor, &client_allocator,
                              server, &client_deallocator);

  ClientMessage *client_message = malloc(sizeof(ClientMessage));
  *client_message = (ClientMessage){.type = Get};

  async_send(NULL, client, message_make(client_message, &free));

  sem_wait(&done);
  sem_destroy(&done);
  cr_expect(result == 42,
            "The client did not get the right value from the server.");

  threadpool_stop(threadpool);
  threadpool_free(threadpool);
  actor_universe_free(actor_universe);
}
