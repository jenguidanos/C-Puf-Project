#include <stdio.h>

#include "puf.h"

#define VALUE_SIZE 5
#define PUF_MAX_EVENTS 5
#define PUF_MAX_CALLBACKS 10

// This is the event to fire, it should be a uint8_t value
// Creating definitions makes your events more meaningful and descriptive.
#define EV_BUTTON_CLICKED 0x01

// This is myCallback, the function to call
// once an event has been triggered.
int myCallback(uint8_t *param);

// The main will call this methods as an example.
void listenToEvent(puf *ev, uint8_t eventId);
void triggerEvent(puf *ev, char *param);

puf *ev;

int main() {
  ev = puf_new(PUF_MAX_EVENTS, PUF_MAX_CALLBACKS);

  // create some listeners
  for (int i = 0; i < PUF_MAX_EVENTS; i++) {
    listenToEvent(ev, EV_BUTTON_CLICKED);
  }

  char param[VALUE_SIZE + 1] = {"Test"};
  triggerEvent(ev, param);

  // the result will print: "Receive Value: Test"
  return 0;
}

void listenToEvent(puf *ev, uint8_t eventId) {
  // When the event EV_BUTTON_CLICKED is emitted,
  // the callback myCallback will be called.
  int err = ev->on(ev, eventId, myCallback);
  if (err) {
    printf("ERROR ON %d\n", err);
  }
}

void triggerEvent(puf *ev, char *param) {
  // Simulate in your mind that a button is pressed.
  // Emit the event EV_BUTTON_CLICKED.
  // myCallback will be called passing "param" as uint8*
  int err = ev->emit(ev, 0x01, PUF_ARG_P(param));
  if (err != 0) {
    printf("ERROR EMIT %d\n", err);
  }
}

int myCallback(uint8_t *param) {
  static int count = 0;

  // this callback does not accept NULL (because no reason)
  if (param == NULL) {
    printf("ERROR: No param, I need a param\n");
    return 1;  // this is an error code
  }

  // process the event
  char value[VALUE_SIZE + 1] = {0};
  PUF_READ_CHAR(value, param, VALUE_SIZE)
  printf("Receive: %s. Count %d. \n", value, count);
  count++;
  return 0;
}
