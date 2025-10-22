#include "letter.h"
#include <stdlib.h>

Letter *make_letter(struct Actor *sender, Message *message) {
  Letter *letter = malloc(sizeof(Letter));
  letter->sender = sender;
  letter->message = message;
  return letter;
}

void free_letter(Letter *letter) {
  free_message(letter->message);
  free(letter);
}

