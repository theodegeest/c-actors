#include <criterion/criterion.h>

#include "../src/c-actors/message.h"

void no_op(void *memory) {}

Test(message, allocation) {
  int x = 42;
  Message *message = message_make(&x, &no_op);
  int result = *(int *)message->payload;
  cr_expect_eq(result, x, "Message should carry a payload.\nGot %d instead of %d", result, x);
  message_free(message);
}
