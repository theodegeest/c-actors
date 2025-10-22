#include "message.h"
#include <stdlib.h>

Message *make_message(void *payload) {
  Message *message = malloc(sizeof(Message));
  message->payload = payload;
  return message;
}

void free_message(Message *message) { free(message); }
