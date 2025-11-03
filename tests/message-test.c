#include <criterion/criterion.h>

#include "../src/c-actors/message.h"

void no_op(void *memory) {}

Test(message, allocation) {
  int x = 42;
  Message *message = message_make(&x, &no_op);
  cr_expect(*(int *)message->payload == 42, "Message should carry a payload.");
  message_free(message);
}
