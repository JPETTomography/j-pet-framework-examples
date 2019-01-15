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

BOOST_AUTO_TEST_CASE(test_angle_middle)
{
  const float EPSILON = 0.001f;
  for (int i = 0; i < 180; i++) {
    float x = std::cos((M_PI * i) / 180 );
    float y = std::sin((M_PI * i) / 180 );
    BOOST_REQUIRE_CLOSE(SinogramCreatorTools::calculateAngle(x, y, -x, -y), i, EPSILON);
  }
}

BOOST_AUTO_TEST_CASE(test_angle)
{
  BOOST_REQUIRE_EQUAL(SinogramCreatorTools::calculateAngle(0.f, 0.f, 0.f, 0.f), 0);
  BOOST_REQUIRE_EQUAL(SinogramCreatorTools::calculateAngle(0.f, 0.f, -1.f, 0.f), 0);
  BOOST_REQUIRE_EQUAL(SinogramCreatorTools::calculateAngle(-1.f, 0.f, 1.f, 0.f), 180);
  BOOST_REQUIRE_EQUAL(SinogramCreatorTools::calculateAngle(1.f, 0.f, -1.f, 0.f), 0);
  BOOST_REQUIRE_EQUAL(SinogramCreatorTools::calculateAngle(0.f, 1.f, 0.f, -1.f), 90);
  BOOST_REQUIRE_EQUAL(SinogramCreatorTools::calculateAngle(0.f, -1.f, 0.f, 1.f), 90);
}

BOOST_AUTO_TEST_CASE(test_distance)
{
  const float EPSILON = 0.00001f;
  BOOST_REQUIRE_CLOSE(SinogramCreatorTools::calculateDistance(0.f, 0.f, 0.f, 0.f), 0, EPSILON);
  BOOST_REQUIRE_CLOSE(SinogramCreatorTools::calculateDistance(0.f, 0.f, -1.f, 0.f), -0, EPSILON);
  BOOST_REQUIRE_CLOSE(SinogramCreatorTools::calculateDistance(0.f, 0.f, 1.f, 0.f), 0, EPSILON);
  BOOST_REQUIRE_CLOSE(SinogramCreatorTools::calculateDistance(1.f, 1.f, -1.f, 1.f), -1, EPSILON);
  BOOST_REQUIRE_CLOSE(SinogramCreatorTools::calculateDistance(-1.f, 1.f, 1.f, 1.f), 1, EPSILON);
  BOOST_REQUIRE_CLOSE(SinogramCreatorTools::calculateDistance(0.f, 1.f, 1.f, 0.f), std::sqrt(0.5f), EPSILON);
  BOOST_REQUIRE_CLOSE(SinogramCreatorTools::calculateDistance(1.f, 0.f, 0.f, 1.f), -std::sqrt(0.5f), EPSILON);
  BOOST_REQUIRE_CLOSE(SinogramCreatorTools::calculateDistance(-1.f, 0.f, 1.f, 0.f), 0, EPSILON);
  BOOST_REQUIRE_CLOSE(SinogramCreatorTools::calculateDistance(1.f, 0.f, -1.f, 0.f), 0, EPSILON);
  BOOST_REQUIRE_CLOSE(SinogramCreatorTools::calculateDistance(0.f, 1.f, 0.f, -1.f), 0, EPSILON);
  BOOST_REQUIRE_CLOSE(SinogramCreatorTools::calculateDistance(0.f, -1.f, 0.f, 1.f), 0, EPSILON);
}

BOOST_AUTO_TEST_CASE(test_lor_slice)
{
  const float EPSILON = 0.0001f;

  float x1 = 0.f;
  float y1 = 0.f;
  float z1 = 0.f;
  float t1 = 0.f;
  float x2 = 0.f;
  float y2 = 0.f;
  float z2 = 0.f;
  float t2 = 0.f;
  BOOST_REQUIRE_CLOSE(SinogramCreatorTools::calculateLORSlice(x1, y1, z1, t1, x2, y2, z2, t2), 0.f, EPSILON);
  x1 = 1.f;
  x2 = -1.f;
  BOOST_REQUIRE_CLOSE(SinogramCreatorTools::calculateLORSlice(x1, y1, z1, t1, x2, y2, z2, t2), 0.f, EPSILON);
  t1 = 10 * 3.33564095; //speed-of-light * 10 * 3.33564095 ~= 1
  t2 = 10 * 3.33564095;
  BOOST_REQUIRE_CLOSE(SinogramCreatorTools::calculateLORSlice(x1, y1, z1, t1, x2, y2, z2, t2), 0.f, EPSILON);
  x1 = 0.f;
  x2 = 0.f;
  z1 = 1.f;
  z2 = -1.f;
  BOOST_REQUIRE_CLOSE(SinogramCreatorTools::calculateLORSlice(x1, y1, z1, t1, x2, y2, z2, t2), 0.f, EPSILON);
  z1 = -1.f;
  z2 = 1.f;
  BOOST_REQUIRE_CLOSE(SinogramCreatorTools::calculateLORSlice(x1, y1, z1, t1, x2, y2, z2, t2), 0.f, EPSILON);
}

BOOST_AUTO_TEST_SUITE_END()
