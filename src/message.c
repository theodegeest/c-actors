#include "message.h"
#include <stdlib.h>

Message *message_make(void *payload) {
  Message *message = malloc(sizeof(Message));
  message->payload = payload;
  return message;
}

void message_free(Message *message) { free(message); }
