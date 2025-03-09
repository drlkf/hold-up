#include <ArduinoFake.h>
#include <iostream>
#include <unity.h>

#include "manhattan.h"

void setUp(void) {
  ArduinoFakeReset();
}

void tearDown(void) {
  // clean stuff up here
}

void test_rotation_boutons(void) {
  char buf[BUF_SIZE];
  button b;
  button previous[NB_ELEM];

  button current[NB_ELEM] = {
    button(true, 0),
    button(false, 1),
    button(true, 2),
    button(true, 3),
    button(true, 4),
    button(true, 5),
    button(true, 6),
    button(true, 7),
    button(true, 8),
    button(true, 9),
  };

  // check that all buttons are uninitialized
  for (auto i = 0; i < NB_ELEM; i++) {
    previous[i].to_string(buf);
    TEST_ASSERT_MESSAGE(b == previous[i], buf);
  }

  // before copying, arrays should not be equal
  TEST_ASSERT_FALSE(memcmp(current, previous, sizeof(current)) == 0);
  rotation_boutons(previous, current);
  TEST_ASSERT_TRUE(memcmp(current, previous, sizeof(current)) == 0);
}

void test_verif_code(void) {
  int tmp;
  button boutons[NB_ELEM] = {
    button(true, 0),
    button(true, 1),
    button(true, 2),
    button(true, 3),
    button(true, 4),
    button(true, 5),
    button(true, 6),
    button(true, 7),
    button(true, 8),
    button(true, 9),
  };

  TEST_ASSERT_TRUE(verif_code(boutons));

  tmp = boutons[0].order;
  boutons[0].order = boutons[1].order;
  boutons[1].order = tmp;

  TEST_ASSERT_FALSE(verif_code(boutons));
}

void test_failure(void) {
  TEST_FAIL_MESSAGE("YOU SHALL NOT PASS");
}

/**
 * For native dev-platform or for some embedded frameworks
 */
int main(void) {
  UNITY_BEGIN();

  RUN_TEST(test_verif_code);
  RUN_TEST(test_rotation_boutons);
  RUN_TEST(test_failure);

  return UNITY_END();
}
