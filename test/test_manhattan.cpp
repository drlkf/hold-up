#include "ArduinoFake.h"
#include "unity.h"
#include "manhattan.h"

void setUp(void) {
  ArduinoFakeReset();
}

void tearDown(void) {
  // clean stuff up here
}

void test_verif_code(void) {
  int tmp;
  button boutons[NB_ELEM] = {
    {true, 0},
    {true, 1},
    {true, 2},
    {true, 3},
    {true, 4},
    {true, 5},
    {true, 6},
    {true, 7},
    {true, 8},
    {true, 9},
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
  RUN_TEST(test_failure);

  return UNITY_END();
}
