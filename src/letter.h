#ifndef LETTER_H_
#define LETTER_H_

#include "message.h"

typedef struct {
  struct Actor *sender;
  Message *message;
} Letter;

Letter *make_letter(struct Actor *sender, Message *message);
void free_letter(Letter *letter);

#endif // !LETTER_H_
