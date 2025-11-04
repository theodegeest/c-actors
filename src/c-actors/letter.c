#include "letter.h"
#include "../safe_alloc/safe_alloc.h"
#include <semaphore.h>
#include <stdlib.h>

Letter *letter_make(struct Actor *sender, Message *message, void *sync_return) {
  Letter *letter = safe_malloc(sizeof(Letter));
  letter->sender = sender;
  letter->message = message;
  if (sync_return != NULL) {
    letter->sync_letter = 1;
    letter->sync_return = sync_return;
    sem_init(&letter->sync_semaphore, 0, 0);
  } else {
    letter->sync_letter = 0;
  }
  return letter;
}

void letter_free(Letter *letter) {
  message_free(letter->message);
  if (letter->sync_letter) {
    sem_destroy(&letter->sync_semaphore);
  }
  free(letter);
}
