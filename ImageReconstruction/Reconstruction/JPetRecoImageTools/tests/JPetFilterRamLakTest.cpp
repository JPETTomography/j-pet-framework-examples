#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE SinogramCreatorTest
#include <boost/test/unit_test.hpp>

#include "../JPetFilterRamLak.h"

BOOST_AUTO_TEST_SUITE(FirstSuite)

BOOST_AUTO_TEST_CASE(ramlak_test) {
  double cutoff = 0.5;
  JPetFilterRamLak ramlak(cutoff);

  int maxValue = 101;
  double EPSILON = 0.0001;
  double result = 0.00;
  double addResult = 0.01 / cutoff;
  for (int x = 0; x < maxValue - 1; x++) {
      double ramlakVal = ramlak(x, maxValue);
      BOOST_REQUIRE_CLOSE(ramlakVal, result, EPSILON);
      result += addResult;
      if (((double) (x + 1)) / ((double) (maxValue - 1)) >= cutoff) result = 0.;
  }
}

BOOST_AUTO_TEST_SUITE_END()