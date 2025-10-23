#include <criterion/criterion.h>

#include "../src/actor_universe.h"
#include "../src/message.h"
#include "../src/log.h"
#include "../src/threadpool.h"

Test(message, allocation) {
  int x = 42;
  Message *message = message_make(&x);
  cr_expect(*(int *)message->payload == 42, "Message should carry a payload.");
  message_free(message);
}
