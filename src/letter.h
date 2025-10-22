#ifndef LETTER_H_
#define LETTER_H_

#include "message.h"

typedef struct {
  struct Actor *sender;
  Message *message;
} Letter;

Letter *letter_make(struct Actor *sender, Message *message);
void letter_free(Letter *letter);

#endif // !LETTER_H_
