#include "message.h"
#include <stdlib.h>

Message *message_make(void *payload, PayloadDeallocator payload_deallocator) {
  Message *message = malloc(sizeof(Message));
  *message =
      (Message){.payload = payload, .payload_deallocator = payload_deallocator};
  return message;
}

void message_free(Message *message) {
  message->payload_deallocator(message->payload);
  free(message);
}
