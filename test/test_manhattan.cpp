#include "ArduinoFake.h"
#include "unity.h"

void setUp(void) {
  ArduinoFakeReset();
}

void tearDown(void) {
  // clean stuff up here
}

void test_failure(void) {
  TEST_FAIL_MESSAGE("YOU SHALL NOT PASS");
}

/**
 * For native dev-platform or for some embedded frameworks
 */
int main(void) {
  UNITY_BEGIN();

  RUN_TEST(test_failure);

  return UNITY_END();
}
