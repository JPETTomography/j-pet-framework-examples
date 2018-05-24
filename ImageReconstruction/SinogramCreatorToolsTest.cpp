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

BOOST_AUTO_TEST_CASE(test_angle)
{
  BOOST_REQUIRE_EQUAL(SinogramCreatorTools::calculateAngle(0.f, 0.f, 0.f, 0.f), 0);
  BOOST_REQUIRE_EQUAL(SinogramCreatorTools::calculateAngle(0.f, 0.f, -1.f, 0.f), 0);
}

BOOST_AUTO_TEST_CASE(test_distance)
{
  const float EPSILON = 0.00001f;
  BOOST_REQUIRE_CLOSE(SinogramCreatorTools::calculateDistance(0.f, 0.f, 0.f, 0.f), 0, EPSILON);
  BOOST_REQUIRE_CLOSE(SinogramCreatorTools::calculateDistance(0.f, 0.f, -1.f, 0.f), -0, EPSILON);
  BOOST_REQUIRE_CLOSE(SinogramCreatorTools::calculateDistance(0.f, 0.f, 1.f, 0.f), 0, EPSILON);
  BOOST_REQUIRE_CLOSE(SinogramCreatorTools::calculateDistance(1.f, 1.f, -1.f, 1.f), 1, EPSILON);
  BOOST_REQUIRE_CLOSE(SinogramCreatorTools::calculateDistance(-1.f, 1.f, 1.f, 1.f), 1, EPSILON);
  BOOST_REQUIRE_CLOSE(SinogramCreatorTools::calculateDistance(0.f, 1.f, 1.f, 0.f), std::sqrt(0.5f), EPSILON);
  BOOST_REQUIRE_CLOSE(SinogramCreatorTools::calculateDistance(1.f, 0.f, 0.f, 1.f), std::sqrt(0.5f), EPSILON);
  BOOST_REQUIRE_CLOSE(SinogramCreatorTools::calculateDistance(-1.f, 0.f, 1.f, 0.f), 0, EPSILON);
  BOOST_REQUIRE_CLOSE(SinogramCreatorTools::calculateDistance(1.f, 0.f, -1.f, 0.f), 0, EPSILON);
  BOOST_REQUIRE_CLOSE(SinogramCreatorTools::calculateDistance(0.f, 1.f, 0.f, -1.f), 0, EPSILON);
  BOOST_REQUIRE_CLOSE(SinogramCreatorTools::calculateDistance(0.f, -1.f, 0.f, 1.f), 0, EPSILON);
}

BOOST_AUTO_TEST_SUITE_END()
