#include "file.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

typedef struct {
  char *str;
} Message;

typedef struct {
  struct Actor *sender;
  Message *message;
} Letter;

#define MAILBOX_SIZE 5

typedef struct Actor {
  Message *mailbox[MAILBOX_SIZE];
  pthread_mutex_t mailbox_mutex;
  void (*behaviour_function)(struct Actor *, Letter *);
} Actor;

#define THREADPOOL_SIZE 2
typedef struct {
  pthread_t threads[THREADPOOL_SIZE];
} Threadpool;

typedef struct {
  Threadpool *threadpool;
  Actor *actor_queue;
  int actor_index;
  pthread_mutex_t actor_queue_mutex;
} ActorUniverse ;

Message *make_message(char *str) {
  Message *message = malloc(sizeof(Message));
  message->str = str;
  return message;
}

void free_message(Message *message) {
  free(message);
}

Letter *make_letter(Actor *sender, Message *message) {
  Letter *letter = malloc(sizeof(Letter));
  letter->sender = sender;
  letter->message = message;
  return letter;
}

void free_letter(Letter *letter) {
  free_message(letter->message);
  free(letter);
}

Actor *make_actor(void (*behaviour_function)(Actor *, Letter *)) {
  Actor *actor = malloc(sizeof(Actor));
  actor->behaviour_function = behaviour_function;
  return actor;
}

void free_actor(Actor *actor) { free(actor); }

void my_actor(Actor *self, Letter *letter) {
  printf("%s\n", letter->message->str);
}

void sync_send(Actor*self, Actor *receiver, Message *message) {
  Letter *letter = make_letter(self, message);
  receiver->behaviour_function(self, letter);
  free_letter(letter);
}

void async_send(Actor *receiver, Message *message) {}


int main(int argc, char *argv[]) {
  Actor *actor = make_actor(&my_actor);
  Message *message = make_message("Hello World!");
  sync_send(NULL, actor, message);

  sleep(2);

  free_actor(actor);
  return 0;
}
