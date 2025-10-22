#ifndef MESSAGE_H_
#define MESSAGE_H_

typedef struct {
  void *payload;
} Message;

Message *make_message(void *payload);
void free_message(Message *message);

#endif // MESSAGE_H_
