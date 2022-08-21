#include <stdio.h>
#include <unity.h>

#include "puf.h"

// set stuff up here
void setUp(void) {}
// clean stuff up here
void tearDown(void) {}

// dummy stuff
puf *events;
#define EV_1 0x01
#define EV_2 0x02
#define EV_3 0x03
#define STR_LEN 5
int cb_1_ok(uint8_t *param) {
  char test[STR_LEN + 1] = {'\0'};
  PUF_READ_CHAR(test, param, STR_LEN);
  TEST_ASSERT_EQUAL_STRING("TEST", test);
  return 0;
}
int cb_2_err(uint8_t *param) { (void)param; return 0x10; }
int cb_3_ok(uint8_t *param) { (void)param; return 0; }

void test_can_create_an_instance(void) {
  TEST_ASSERT_NOT_NULL(events);
  TEST_ASSERT_EQUAL(5, events->sizeEvents);
  TEST_ASSERT_EQUAL(10, events->sizeCallbacks);
}

void test_all_null_after_reset() {
  events->reset(events);
  for (int i = 0; i < events->sizeEvents; i++) {
    TEST_ASSERT_EQUAL(0, events->events[i]);
    for (int j = 0; j < events->sizeCallbacks; j++) {
      TEST_ASSERT_NULL(events->callbacks[i][j]);
    }
  }
}

void test_reset_event() {
  puf * o = puf_new(2, 2);
  o->on(o, EV_1, cb_1_ok);
  o->on(o, EV_1, cb_2_err);

  TEST_ASSERT_EQUAL(o->callbacks[0][0], cb_1_ok);
  TEST_ASSERT_EQUAL(o->callbacks[0][1], cb_2_err);

  o->resetEvent(o, EV_1);

  TEST_ASSERT_EQUAL(o->callbacks[0][0], NULL);
  TEST_ASSERT_EQUAL(o->callbacks[0][1], NULL);

  // call to emit after remove all events
  int err = o->emit(o, EV_1, NULL);
  TEST_ASSERT_EQUAL(err, 11);
}

void test_first_listeners(void) {
  events->on(events, EV_1, cb_1_ok);
  events->on(events, EV_2, cb_2_err);
}

void test_check_listener_1() {
  TEST_ASSERT_EQUAL(events->events[0], EV_1);
  TEST_ASSERT_EQUAL(events->callbacks[0][0], cb_1_ok);
  // check that next index is yet null
  TEST_ASSERT_EQUAL(events->callbacks[0][1], NULL);
}

void test_check_listener_2() {
  TEST_ASSERT_EQUAL(events->events[1], EV_2);
  TEST_ASSERT_EQUAL(events->callbacks[1][0], cb_2_err);
}

void test_error_if_cb_exists() {
  // cb_1_ok has been previously added to EV_1 on test_first_listeners()
  // it should return error if we try to add it again
  int err = events->on(events, EV_1, cb_1_ok);
  TEST_ASSERT_EQUAL(err, 50); // 50 is the error code for this callback exists
}

void test_emit_listener_1() {
  char test[STR_LEN] = {"TEST"};
  int err = events->emit(events, EV_1, PUF_ARG(test));
  TEST_ASSERT_EQUAL(0, err);
}

void test_emit_listener_2() {
  int err = events->emit(events, EV_2, NULL);
  TEST_ASSERT_EQUAL(0x10, err);
}

void test_emit_unknow_event() {
  int err = events->emit(events, 0xFF, NULL);
  TEST_ASSERT_EQUAL(10, err); // 10 is the error code for unknown event
}

void test_off_event() {
  // be sure that actually there is a listener
  TEST_ASSERT_EQUAL(cb_1_ok, events->callbacks[0][0]);

  // remove the listener
  int err = events->off(events, EV_1, cb_1_ok);
  TEST_ASSERT_EQUAL(0, err); // 0 is the index of the callback cb_1_ok
  // check it is removed
  TEST_ASSERT_EQUAL(NULL, events->callbacks[0][0]);
  TEST_ASSERT_EQUAL(EV_1, events->events[0]);
  // try to remove the same listener again.
  // It has been previously removed, then should return error code 31
  // because does not exists
  err = events->off(events, EV_1, cb_1_ok);
  TEST_ASSERT_EQUAL(31, err);
  // check an invalid event name
  err = events->off(events, 0xFF, cb_1_ok);
  // 30 is the error code for unknown event when using off
  TEST_ASSERT_EQUAL(30, err);
}

void test_on_fails_if_event_full() {
  puf *o = puf_new(2, 2);  // two events with two callbacks
  int err = 0;

  err = o->on(o, EV_1, cb_1_ok);
  TEST_ASSERT_EQUAL(0, err);

  err = o->on(o, EV_2, cb_2_err);
  TEST_ASSERT_EQUAL(0, err);

  // this is the third event, max two events, should fail
  err = o->on(o, EV_3, cb_3_ok);
  TEST_ASSERT_EQUAL(20, err); // 20 is the error code for events full
}

void test_on_fails_if_cb_full() {
  puf *o = puf_new(2, 2);  // two events with two callbacks
  int err = 0;

  err = o->on(o, EV_1, cb_1_ok);
  TEST_ASSERT_EQUAL(0, err);

  err = o->on(o, EV_1, cb_2_err);
  TEST_ASSERT_EQUAL(0, err);

  err = o->on(o, EV_1, cb_3_ok);  // max 2 callbacks, should fail
  TEST_ASSERT_EQUAL(21, err); // 21 is the error code for callbacks full
}

void test_destroy_instance() {
  puf *o = puf_new(2, 2);
  printf("size %lu\n", sizeof(o));
  puf_destroy(o);
  printf("size %lu\n", sizeof(o));
  // TEST_ASSERT_EQUAL(NULL, o);
}

int main() {
  events = puf_new(5, 10);

  UNITY_BEGIN();
  RUN_TEST(test_can_create_an_instance);  // this should be the first test
  RUN_TEST(test_all_null_after_reset);
  RUN_TEST(test_reset_event);
  RUN_TEST(test_first_listeners);
  RUN_TEST(test_check_listener_1);
  RUN_TEST(test_check_listener_2);
  RUN_TEST(test_error_if_cb_exists);
  RUN_TEST(test_emit_listener_1);
  RUN_TEST(test_emit_listener_2);
  RUN_TEST(test_emit_unknow_event);
  RUN_TEST(test_off_event);
  RUN_TEST(test_on_fails_if_event_full);
  RUN_TEST(test_on_fails_if_cb_full);
  RUN_TEST(test_destroy_instance);
  UNITY_END();
  return 0;
}
