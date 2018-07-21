#define WASM_EXPORT __attribute__((visibility("default")))

#include "v1x1.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>

struct v1x1_buffer mkbuffer(int32_t length, char *data) {
  struct v1x1_buffer buffer = {
    length, data
  };
  return buffer;
}

struct v1x1_buffer mkstrbuffer(char *str) {
  return mkbuffer(strlen(str), str);
}

void send_message(struct v1x1_channel *channel, char *message) {
  struct v1x1_buffer buffer = mkstrbuffer(message);
  v1x1_send_message(channel, &buffer);
}

void send_messagef(struct v1x1_channel *channel, char *format, ...) {
  char buf[4096];
  va_list args;
  va_start(args, format);
  vsnprintf(buf, 4096, format, args);
  send_message(channel, buf);
  va_end(args);
}

void do_log(char *message) {
  struct v1x1_buffer buffer = mkstrbuffer(message);
  v1x1_log(&buffer);
}

void do_logf(char *format, ...) {
  char buf[4096];
  va_list args;
  va_start(args, format);
  vsnprintf(buf, 4096, format, args);
  do_log(buf);
  va_end(args);
};

WASM_EXPORT
void event_handler(void) {
  int event_size = v1x1_event_size();
  if(event_size < 0)
    return;
  struct v1x1_event *evt = malloc(event_size);
  v1x1_read_event(evt, event_size);
  do_logf("Got event type: %d", evt->event_type);
  if(evt->event_type == MESSAGE) {
    struct v1x1_message *msg = &evt->data.message_event.message;
    do_logf("Message text: %s", msg->text.buf);
    if(!strcmp(msg->text.buf, "!hellowasm")) {
      do_logf("Replying in %s", msg->channel.display_name.buf);
      send_messagef(&msg->channel, "Hi, %s!", msg->sender.display_name.buf);
    }
  }
  free(evt);
}
