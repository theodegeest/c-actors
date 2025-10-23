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

// Test(server_client, test1) {

//   ActorUniverse *actor_universe = actor_universe_make();
//   Threadpool *threadpool = threadpool_make(actor_universe, 8);



//   threadpool_stop(threadpool);
//   threadpool_free(threadpool);
//   actor_universe_free(actor_universe);

//   int x = 42;
//   Message *message = message_make(&x);
//   cr_expect(*(int *)message->payload == 42, "Message should carry a payload.");
//   message_free(message);
// }
