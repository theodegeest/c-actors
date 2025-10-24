#ifndef LETTER_H_
#define LETTER_H_

#include "message.h"
#include <semaphore.h>

typedef struct {
  struct Actor *sender;
  Message *message;
  char sync_letter;
  sem_t sync_semaphore;
  void **sync_return;
} Letter;

Letter *letter_make(struct Actor *sender, Message *message, void *sync_return);
void letter_free(Letter *letter);

#endif // !LETTER_H_
