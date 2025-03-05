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
  int tableauCourant[NB_ELEM];

  memcpy(tableauCourant, tableauOrdRequis, NB_ELEM * sizeof(int));

  TEST_ASSERT_TRUE(verif_code(tableauCourant, tableauOrdRequis));

  tableauCourant[0] = tableauOrdRequis[0] + 1;

  TEST_ASSERT_FALSE(verif_code(tableauCourant, tableauOrdRequis));
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
