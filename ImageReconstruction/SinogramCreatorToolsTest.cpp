#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE SinogramCreatorTest
#include <boost/test/unit_test.hpp>

#include "SinogramCreatorTools.h"

BOOST_AUTO_TEST_SUITE(FirstSuite)

BOOST_AUTO_TEST_CASE(roundToNearesMultiplicity_test)
{
  BOOST_REQUIRE_EQUAL(SinogramCreatorTools::roundToNearesMultiplicity(0.0f, 1.f), 0u);
  BOOST_REQUIRE_EQUAL(SinogramCreatorTools::roundToNearesMultiplicity(0.4f, 1.f), 0u);
  BOOST_REQUIRE_EQUAL(SinogramCreatorTools::roundToNearesMultiplicity(0.5f, 1.f), 1u);
  BOOST_REQUIRE_EQUAL(SinogramCreatorTools::roundToNearesMultiplicity(30.f, 1.f), 30u);
  BOOST_REQUIRE_EQUAL(SinogramCreatorTools::roundToNearesMultiplicity(0.00f, 0.01f), 0u);
  BOOST_REQUIRE_EQUAL(SinogramCreatorTools::roundToNearesMultiplicity(0.01f, 0.01f), 1u);
  BOOST_REQUIRE_EQUAL(SinogramCreatorTools::roundToNearesMultiplicity(0.02f, 0.01f), 2u);
}

BOOST_AUTO_TEST_SUITE_END()
