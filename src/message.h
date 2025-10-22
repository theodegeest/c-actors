#ifndef MESSAGE_H_
#define MESSAGE_H_

typedef struct {
  void *payload;
} Message;

Message *message_make(void *payload);
void message_free(Message *message);

#endif // MESSAGE_H_
