#ifndef MESSAGE_H_
#define MESSAGE_H_

typedef void (*PayloadDeallocator)(void *);

typedef struct {
  void *payload;
  PayloadDeallocator payload_deallocator;
} Message;

// This function makes a message given a payload and gives the ownership to the
// caller.
Message *message_make(void *payload, PayloadDeallocator payload_deallocator);

// IMPORTANT: This function should not be used by the user.
// This function should be used to free a message.
void message_free(Message *message);

#endif // MESSAGE_H_
