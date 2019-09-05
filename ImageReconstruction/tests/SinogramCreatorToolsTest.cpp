#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE SinogramCreatorTest
#include <boost/test/unit_test.hpp>

#include "SinogramCreatorTools.h"
#include <iostream>

BOOST_AUTO_TEST_SUITE(FirstSuite)

BOOST_AUTO_TEST_CASE(roundToNearesMultiplicity_test) {
  BOOST_REQUIRE_EQUAL(
      SinogramCreatorTools::roundToNearesMultiplicity(0.0f, 1.f), 0u);
  BOOST_REQUIRE_EQUAL(
      SinogramCreatorTools::roundToNearesMultiplicity(0.4f, 1.f), 0u);
  BOOST_REQUIRE_EQUAL(
      SinogramCreatorTools::roundToNearesMultiplicity(0.5f, 1.f), 1u);
  BOOST_REQUIRE_EQUAL(
      SinogramCreatorTools::roundToNearesMultiplicity(30.f, 1.f), 30u);
  BOOST_REQUIRE_EQUAL(
      SinogramCreatorTools::roundToNearesMultiplicity(0.00f, 0.01f), 0u);
  BOOST_REQUIRE_EQUAL(
      SinogramCreatorTools::roundToNearesMultiplicity(0.01f, 0.01f), 1u);
  BOOST_REQUIRE_EQUAL(
      SinogramCreatorTools::roundToNearesMultiplicity(0.02f, 0.01f), 2u);
}

BOOST_AUTO_TEST_CASE(test_angle_middle) {
  const float EPSILON = 0.01f;
  const float r = 10;
  const float maxDistance = 20.f;
  const float accuracy = 0.1f;

  for (int i = 0; i < 360; i++) {
    const float x1 = r * std::cos((i) * (M_PI / 180.f));
    const float y1 = r * std::sin((i) * (M_PI / 180.f));
    const float x2 = r * std::cos((i + 180) * (M_PI / 180.f));
    const float y2 = r * std::sin((i + 180) * (M_PI / 180.f));
    const auto result = SinogramCreatorTools::getSinogramRepresentation(
        x1, y1, x2, y2, maxDistance, accuracy,
        std::ceil(maxDistance * 2.f * (1.f / accuracy)), 180);
    int resultAngle =
        i < 90 ? 90 + i : i < 180 ? i - 90 : i < 270 ? i - 90 : i - 270;
    BOOST_REQUIRE_EQUAL(result.second, resultAngle);
    const float distanceResult =
        SinogramCreatorTools::roundToNearesMultiplicity(maxDistance, accuracy);
    BOOST_REQUIRE_CLOSE(result.first, distanceResult, EPSILON);
  }
}

BOOST_AUTO_TEST_CASE(test_lor_slice) {
  const float EPSILON = 0.0001f;

  float x1 = 0.f;
  float y1 = 0.f;
  float z1 = 0.f;
  float t1 = 0.f;
  float x2 = 0.f;
  float y2 = 0.f;
  float z2 = 0.f;
  float t2 = 0.f;
  BOOST_REQUIRE_CLOSE(
      SinogramCreatorTools::calculateLORSlice(x1, y1, z1, t1, x2, y2, z2, t2),
      0.f, EPSILON);
  x1 = 1.f;
  x2 = -1.f;
  BOOST_REQUIRE_CLOSE(
      SinogramCreatorTools::calculateLORSlice(x1, y1, z1, t1, x2, y2, z2, t2),
      0.f, EPSILON);
  t1 = 10 * 3.33564095; // speed-of-light * 10 * 3.33564095 ~= 1
  t2 = 10 * 3.33564095;
  BOOST_REQUIRE_CLOSE(
      SinogramCreatorTools::calculateLORSlice(x1, y1, z1, t1, x2, y2, z2, t2),
      0.f, EPSILON);
  x1 = 0.f;
  x2 = 0.f;
  z1 = 1.f;
  z2 = -1.f;
  BOOST_REQUIRE_CLOSE(
      SinogramCreatorTools::calculateLORSlice(x1, y1, z1, t1, x2, y2, z2, t2),
      0.f, EPSILON);
  z1 = -1.f;
  z2 = 1.f;
  BOOST_REQUIRE_CLOSE(
      SinogramCreatorTools::calculateLORSlice(x1, y1, z1, t1, x2, y2, z2, t2),
      0.f, EPSILON);
}

BOOST_AUTO_TEST_CASE(remap_to_single_layer) {
  const float EPSILON = 0.1f;
  float x1 = 0.f;
  float y1 = 0.f;
  float z1 = 0.f;
  float x2 = 1.f;
  float y2 = 0.f;
  float z2 = 0.f;

  float radius = 40.f;

  auto result = SinogramCreatorTools::remapToSingleLayer(
      TVector3(x1, y1, z1), TVector3(x2, y2, z2), radius);
  BOOST_REQUIRE_CLOSE(result.first.X(), radius, EPSILON);
  BOOST_REQUIRE_CLOSE(result.first.Y(), 0.f, EPSILON);
  BOOST_REQUIRE_CLOSE(result.first.Z(), 0.f, EPSILON);

  BOOST_REQUIRE_CLOSE(result.second.X(), -radius, EPSILON);
  BOOST_REQUIRE_CLOSE(result.second.Y(), 0.f, EPSILON);
  BOOST_REQUIRE_CLOSE(result.second.Z(), 0.f, EPSILON);

  radius = 45.75f;
  result = SinogramCreatorTools::remapToSingleLayer(
      TVector3(x1, y1, z1), TVector3(x2, y2, z2), radius);
  BOOST_REQUIRE_CLOSE(result.first.X(), radius, EPSILON);
  BOOST_REQUIRE_CLOSE(result.first.Y(), 0.f, EPSILON);
  BOOST_REQUIRE_CLOSE(result.first.Z(), 0.f, EPSILON);

  BOOST_REQUIRE_CLOSE(result.second.X(), -radius, EPSILON);
  BOOST_REQUIRE_CLOSE(result.second.Y(), 0.f, EPSILON);
  BOOST_REQUIRE_CLOSE(result.second.Z(), 0.f, EPSILON);

  x1 = 0.f;
  y1 = 30.f;
  x2 = 0.f;
  y2 = -30.f;

  result = SinogramCreatorTools::remapToSingleLayer(
      TVector3(x1, y1, z1), TVector3(x2, y2, z2), radius);
  BOOST_REQUIRE_CLOSE(result.first.X(), 0.f, EPSILON);
  BOOST_REQUIRE_CLOSE(result.first.Y(), radius, EPSILON);
  BOOST_REQUIRE_CLOSE(result.first.Z(), 0.f, EPSILON);

  BOOST_REQUIRE_CLOSE(result.second.X(), 0.f, EPSILON);
  BOOST_REQUIRE_CLOSE(result.second.Y(), -radius, EPSILON);
  BOOST_REQUIRE_CLOSE(result.second.Z(), 0.f, EPSILON);

  x1 = 10.f;
  y1 = 30.f;
  x2 = 10.f;
  y2 = -30.f;

  result = SinogramCreatorTools::remapToSingleLayer(
      TVector3(x1, y1, z1), TVector3(x2, y2, z2), radius);
  BOOST_REQUIRE_CLOSE(result.first.X(), 10.f, EPSILON);
  BOOST_REQUIRE_CLOSE(result.first.Y(), radius, EPSILON);
  BOOST_REQUIRE_CLOSE(result.first.Z(), 0.f, EPSILON);

  BOOST_REQUIRE_CLOSE(result.second.X(), 10.f, EPSILON);
  BOOST_REQUIRE_CLOSE(result.second.Y(), -radius, EPSILON);
  BOOST_REQUIRE_CLOSE(result.second.Z(), 0.f, EPSILON);

  x1 = 10.f;
  y1 = 30.f;
  x2 = -10.f;
  y2 = -30.f;
  result = SinogramCreatorTools::remapToSingleLayer(
      TVector3(x1, y1, z1), TVector3(x2, y2, z2), radius);
  BOOST_REQUIRE_CLOSE(result.first.X(), 14.4674, EPSILON);
  BOOST_REQUIRE_CLOSE(result.first.Y(), 43.4023, EPSILON);
  BOOST_REQUIRE_CLOSE(result.first.Z(), 0.f, EPSILON);

  BOOST_REQUIRE_CLOSE(result.second.X(), -14.4674, EPSILON);
  BOOST_REQUIRE_CLOSE(result.second.Y(), -43.4023, EPSILON);
  BOOST_REQUIRE_CLOSE(result.second.Z(), 0.f, EPSILON);

  x1 = 0.f;
  y1 = 30.f;
  z1 = 10.f;

  x2 = 0.f;
  y2 = -30.f;
  z2 = 10.f;

  result = SinogramCreatorTools::remapToSingleLayer(
      TVector3(x1, y1, z1), TVector3(x2, y2, z2), radius);
  BOOST_REQUIRE_CLOSE(result.first.X(), 0.f, EPSILON);
  BOOST_REQUIRE_CLOSE(result.first.Y(), radius, EPSILON);
  BOOST_REQUIRE_CLOSE(result.first.Z(), z1, EPSILON);

  BOOST_REQUIRE_CLOSE(result.second.X(), 0.f, EPSILON);
  BOOST_REQUIRE_CLOSE(result.second.Y(), -radius, EPSILON);
  BOOST_REQUIRE_CLOSE(result.second.Z(), z2, EPSILON);

  x1 = 0.f;
  y1 = 30.f;
  z1 = -25.f;

  x2 = 0.f;
  y2 = -30.f;
  z2 = 25.f;

  result = SinogramCreatorTools::remapToSingleLayer(
      TVector3(x1, y1, z1), TVector3(x2, y2, z2), radius);
  BOOST_REQUIRE_CLOSE(result.first.X(), 0.f, EPSILON);
  BOOST_REQUIRE_CLOSE(result.first.Y(), radius, EPSILON);
  BOOST_REQUIRE_CLOSE(result.first.Z(), z1, EPSILON);

  BOOST_REQUIRE_CLOSE(result.second.X(), 0.f, EPSILON);
  BOOST_REQUIRE_CLOSE(result.second.Y(), -radius, EPSILON);
  BOOST_REQUIRE_CLOSE(result.second.Z(), z2, EPSILON);
}

BOOST_AUTO_TEST_CASE(polyfit_test) {
  BOOST_REQUIRE_CLOSE(
      SinogramCreatorTools::getPolyFit(
          {std::sqrt((9.39 * 9.39) + (-10.75 * -10.75)), -std::abs(3.39)}),
      4.367437643607859e-01, kEPSILON);
  BOOST_REQUIRE_CLOSE(
      SinogramCreatorTools::getPolyFit(
          {std::sqrt((9.57 * 9.57) + (-0.87 * -0.87)), -std::abs(2.49)}),
      7.197118953414579e-01, kEPSILON);
  BOOST_REQUIRE_CLOSE(
      SinogramCreatorTools::getPolyFit(
          {std::sqrt((-11.83 * -11.83) + (2.66 * 2.66)), -std::abs(-2.72)}),
      5.820874582833866e-01, kEPSILON);
  BOOST_REQUIRE_CLOSE(
      SinogramCreatorTools::getPolyFit(
          {std::sqrt((1.84 * 1.84) + (-8.89 * -8.89)), -std::abs(-4.77)}),
      7.430144030486940e-01, kEPSILON);
}

BOOST_AUTO_TEST_SUITE_END()