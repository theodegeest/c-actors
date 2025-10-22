#include "letter.h"
#include <stdlib.h>

Letter *letter_make(struct Actor *sender, Message *message) {
  Letter *letter = malloc(sizeof(Letter));
  letter->sender = sender;
  letter->message = message;
  return letter;
}

void letter_free(Letter *letter) {
  message_free(letter->message);
  free(letter);
}
