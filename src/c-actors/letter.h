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

// IMPORTANT: This function should not be used by the user.
// This function will make a letter.
// This letter specifies if it is a synchronous one or not.
// The ownership is given to the caller.
Letter *letter_make(struct Actor *sender, Message *message, void *sync_return);


// IMPORTANT: This function should not be used by the user.
// This function should be used to free a letter
void letter_free(Letter *letter);

#endif // !LETTER_H_
