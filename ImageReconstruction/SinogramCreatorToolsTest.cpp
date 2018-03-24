#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE SinogramCreatorTest
#include <boost/test/unit_test.hpp>

#include <ostream>

#include "SinogramCreatorTools.h"
#include "JPetLoggerInclude.h"
#include <JPetHit/JPetHit.h>
#include "JPetCommonTools/JPetCommonTools.h"

//fix for printing pair values
//https://stackoverflow.com/questions/10976130/boost-check-equal-with-pairint-int-and-custom-operator
namespace boost
{
namespace test_tools
{
template <typename T, typename U, typename L>
struct print_log_value<std::tuple<T, U, L>> {
  void operator()(std::ostream& os, std::tuple<T, U, L> const& pr)
  {
    os << "<" << std::get<0>(pr) << "," << std::get<1>(pr) << "," << std::get<2>(pr) << ">";
  }
};
}
}

namespace boost
{
namespace test_tools
{
template <typename T, typename U>
struct print_log_value<std::pair<T, U>> {
  void operator()(std::ostream& os, std::pair<T, U> const& pr)
  {
    os << "<" << std::get<0>(pr) << "," << std::get<1>(pr) << ">";
  }
};
}
}

BOOST_AUTO_TEST_SUITE(FirstSuite)

BOOST_AUTO_TEST_CASE( bresenham_test )
{
  auto result = SinogramCreatorTools::calculateLineWithoutAntialiasing(0., 1., 6., 4.);
  BOOST_REQUIRE_EQUAL(result.size(), 7);
  BOOST_REQUIRE_EQUAL(result[0], std::make_tuple(0, 1, 255));
  BOOST_REQUIRE_EQUAL(result[1], std::make_tuple(1, 1, 255));
  BOOST_REQUIRE_EQUAL(result[2], std::make_tuple(2, 2, 255));
  BOOST_REQUIRE_EQUAL(result[3], std::make_tuple(3, 2, 255));
  BOOST_REQUIRE_EQUAL(result[4], std::make_tuple(4, 3, 255));
  BOOST_REQUIRE_EQUAL(result[5], std::make_tuple(5, 3, 255));
  BOOST_REQUIRE_EQUAL(result[6], std::make_tuple(6, 4, 255));
  result.clear();
  BOOST_REQUIRE_EQUAL(result.size(), 0);
  result = SinogramCreatorTools::calculateLineWithoutAntialiasing(6., 4., 0., 1.);
  BOOST_REQUIRE_EQUAL(result.size(), 7);
  BOOST_REQUIRE_EQUAL(result[0], std::make_tuple(0, 1, 255));
  BOOST_REQUIRE_EQUAL(result[1], std::make_tuple(1, 1, 255));
  BOOST_REQUIRE_EQUAL(result[2], std::make_tuple(2, 2, 255));
  BOOST_REQUIRE_EQUAL(result[3], std::make_tuple(3, 2, 255));
  BOOST_REQUIRE_EQUAL(result[4], std::make_tuple(4, 3, 255));
  BOOST_REQUIRE_EQUAL(result[5], std::make_tuple(5, 3, 255));
  BOOST_REQUIRE_EQUAL(result[6], std::make_tuple(6, 4, 255));
}

BOOST_AUTO_TEST_CASE(xiaolin_test)
{
  auto result = SinogramCreatorTools::calculateLineWithAntialiasing(0., 1., 6., 4.);
  BOOST_REQUIRE_EQUAL(result.size(), 14);
  BOOST_REQUIRE_EQUAL(result[0], std::make_tuple(0, 1, 128));
  BOOST_REQUIRE_EQUAL(result[1], std::make_tuple(0, 2, 0));
  BOOST_REQUIRE_EQUAL(result[2], std::make_tuple(6, 4, 128));
  BOOST_REQUIRE_EQUAL(result[3], std::make_tuple(6, 5, 0));
  BOOST_REQUIRE_EQUAL(result[4], std::make_tuple(1, 1, 128));
  BOOST_REQUIRE_EQUAL(result[5], std::make_tuple(1, 2, 128));
  BOOST_REQUIRE_EQUAL(result[6], std::make_tuple(2, 2, 255));
  BOOST_REQUIRE_EQUAL(result[7], std::make_tuple(2, 3, 0));
  BOOST_REQUIRE_EQUAL(result[8], std::make_tuple(3, 2, 128));
  BOOST_REQUIRE_EQUAL(result[9], std::make_tuple(3, 3, 128));
  BOOST_REQUIRE_EQUAL(result[10], std::make_tuple(4, 3, 255));
  BOOST_REQUIRE_EQUAL(result[11], std::make_tuple(4, 4, 0));
  BOOST_REQUIRE_EQUAL(result[12], std::make_tuple(5, 3, 128));
  BOOST_REQUIRE_EQUAL(result[13], std::make_tuple(5, 4, 128));
  result.clear();
  BOOST_REQUIRE_EQUAL(result.size(), 0);
  result = SinogramCreatorTools::calculateLineWithAntialiasing(6., 4., 0., 1.);
  BOOST_REQUIRE_EQUAL(result.size(), 14);
  BOOST_REQUIRE_EQUAL(result[0], std::make_tuple(0, 1, 128));
  BOOST_REQUIRE_EQUAL(result[1], std::make_tuple(0, 2, 0));
  BOOST_REQUIRE_EQUAL(result[2], std::make_tuple(6, 4, 128));
  BOOST_REQUIRE_EQUAL(result[3], std::make_tuple(6, 5, 0));
  BOOST_REQUIRE_EQUAL(result[4], std::make_tuple(1, 1, 128));
  BOOST_REQUIRE_EQUAL(result[5], std::make_tuple(1, 2, 128));
  BOOST_REQUIRE_EQUAL(result[6], std::make_tuple(2, 2, 255));
  BOOST_REQUIRE_EQUAL(result[7], std::make_tuple(2, 3, 0));
  BOOST_REQUIRE_EQUAL(result[8], std::make_tuple(3, 2, 128));
  BOOST_REQUIRE_EQUAL(result[9], std::make_tuple(3, 3, 128));
  BOOST_REQUIRE_EQUAL(result[10], std::make_tuple(4, 3, 255));
  BOOST_REQUIRE_EQUAL(result[11], std::make_tuple(4, 4, 0));
  BOOST_REQUIRE_EQUAL(result[12], std::make_tuple(5, 3, 128));
  BOOST_REQUIRE_EQUAL(result[13], std::make_tuple(5, 4, 128));
}

BOOST_AUTO_TEST_CASE(distance_from_center)
{
  float distance = SinogramCreatorTools::calculateDistanceFromCenter(1.f, 1.f, 2.f, 2.f, 0.f, 0.f);
  BOOST_REQUIRE_CLOSE(distance, 0.f, 0.0001f);
  distance = SinogramCreatorTools::calculateDistanceFromCenter(1.f, 1.f, 1.f, 2.f, 0.f, 0.f);
  BOOST_REQUIRE_CLOSE(distance, 1.f, 0.0001f);
  distance = SinogramCreatorTools::calculateDistanceFromCenter(0.f, 1.f, 1.f, 0.f, 0.f, 0.f);
  BOOST_REQUIRE_CLOSE(distance, std::sqrt(2.f) / 2, 0.0001f);
}

BOOST_AUTO_TEST_CASE( test_line_intersection_and_distance )
{
  const float EPSILON = 0.0001f;
  std::pair<float, float> p1 = std::make_pair(0.f, 0.f);
  std::pair<float, float> p2 = std::make_pair(1.f, 0.f);
  std::pair<float, float> p3 = std::make_pair(0.5f, 0.5f);
  std::pair<float, float> p4 = std::make_pair(0.5f, -0.5f);
  BOOST_REQUIRE_EQUAL(SinogramCreatorTools::lineIntersection(p1, p2, p3, p4), std::make_pair(0.5f, 0.0f));

  for (float alpha = 0.1f; alpha < 180.f; alpha += 0.1f) {
    float x = cos(alpha * (M_PI / 180.f));
    float y = sin(alpha * (M_PI / 180.f));
    auto result = SinogramCreatorTools::lineIntersection(std::make_pair(1.f, 0.f), std::make_pair(-1.f, 0.f), std::make_pair(x, y), std::make_pair(-x, -y));
    BOOST_REQUIRE_CLOSE(result.first, 0.f, EPSILON);
    BOOST_REQUIRE_CLOSE(result.second, 0.f, EPSILON);
    BOOST_REQUIRE_CLOSE(SinogramCreatorTools::length2D(result.first, result.second), 0.f, EPSILON);
  }

  auto result = SinogramCreatorTools::lineIntersection(std::make_pair(0.f, .5f), std::make_pair(0.5f, -0.5f), std::make_pair(0.f, -0.5f), std::make_pair(0.5f, -0.5f));
  BOOST_REQUIRE_CLOSE(result.first, 0.5f, EPSILON);
  BOOST_REQUIRE_CLOSE(result.second, -0.5f, EPSILON);
  BOOST_REQUIRE_CLOSE(SinogramCreatorTools::length2D(result.first, result.second), 0.707106781f, EPSILON);

  float interesectionScale = 1.f / 10.f;
  for (float alpha = 0.1f; alpha < 180.f; alpha += 0.1f) {
    float x = 10 * cos(alpha * (M_PI / 180.f));
    float y = 10 * sin(alpha * (M_PI / 180.f));
    float scaledX = x * interesectionScale;
    float scaledY = y * interesectionScale; // intersection point should be in (scaledX, scaledY)
    float dx = scaledX + scaledX;
    float dy = scaledY + scaledY;
    float a1 = scaledX - dy / 2;
    float b1 = scaledY + dx / 2;
    float a2 = scaledX + dy / 2;
    float b2 = scaledY - dx / 2;
    auto result = SinogramCreatorTools::lineIntersection(std::make_pair(a1, b1), std::make_pair(a2, b2), std::make_pair(x, y), std::make_pair(-x, -y));
    float distance = std::sqrt((scaledX * scaledX) + (scaledY * scaledY));
    BOOST_REQUIRE_CLOSE(result.first, scaledX, EPSILON);
    BOOST_REQUIRE_CLOSE(result.second, scaledY, EPSILON);
    BOOST_REQUIRE_CLOSE(SinogramCreatorTools::length2D(result.first, result.second), distance, EPSILON);
  }

  interesectionScale = 1.f / 100.f;
  for (float alpha = 0.1f; alpha < 180.f; alpha += 0.1f) {
    float x = 10 * cos(alpha * (M_PI / 180.f));
    float y = 10 * sin(alpha * (M_PI / 180.f));
    float scaledX = x * interesectionScale;
    float scaledY = y * interesectionScale; // intersection point should be in (scaledX, scaledY)
    float dx = scaledX + scaledX;
    float dy = scaledY + scaledY;
    float a1 = scaledX - dy / 2;
    float b1 = scaledY + dx / 2;
    float a2 = scaledX + dy / 2;
    float b2 = scaledY - dx / 2;
    auto result = SinogramCreatorTools::lineIntersection(std::make_pair(a1, b1), std::make_pair(a2, b2), std::make_pair(x, y), std::make_pair(-x, -y));
    float distance = std::sqrt((scaledX * scaledX) + (scaledY * scaledY));
    BOOST_REQUIRE_CLOSE(result.first, scaledX, EPSILON);
    BOOST_REQUIRE_CLOSE(result.second, scaledY, EPSILON);
    BOOST_REQUIRE_CLOSE(SinogramCreatorTools::length2D(result.first, result.second), distance, EPSILON);
  }
}

BOOST_AUTO_TEST_CASE(roundToNearesMultiplicity_test)
{
  BOOST_REQUIRE_EQUAL(SinogramCreatorTools::roundToNearesMultiplicity(0.0f, 1.f, 30.f), 30u);
  BOOST_REQUIRE_EQUAL(SinogramCreatorTools::roundToNearesMultiplicity(-30.f, 1.f, 30.f), 0u);
  BOOST_REQUIRE_EQUAL(SinogramCreatorTools::roundToNearesMultiplicity(-29.9f, 1.f, 30.f), 0u);
  BOOST_REQUIRE_EQUAL(SinogramCreatorTools::roundToNearesMultiplicity(-29.5f, 1.f, 30.f), 1u);
  BOOST_REQUIRE_EQUAL(SinogramCreatorTools::roundToNearesMultiplicity(-29.4f, 1.f, 30.f), 1u);
  BOOST_REQUIRE_EQUAL(SinogramCreatorTools::roundToNearesMultiplicity(-29.f, 1.f, 30.f), 1u);
  BOOST_REQUIRE_EQUAL(SinogramCreatorTools::roundToNearesMultiplicity(-28.6f, 1.f, 30.f), 1u);
  BOOST_REQUIRE_EQUAL(SinogramCreatorTools::roundToNearesMultiplicity(-28.5f, 1.f, 30.f), 2u);
  BOOST_REQUIRE_EQUAL(SinogramCreatorTools::roundToNearesMultiplicity(30.f, 1.f, 30.f), 60u);
  BOOST_REQUIRE_EQUAL(SinogramCreatorTools::roundToNearesMultiplicity(0.00f, 0.01f, 30.f), 3000u);
  BOOST_REQUIRE_EQUAL(SinogramCreatorTools::roundToNearesMultiplicity(0.01f, 0.01f, 30.f), 3001u);
  BOOST_REQUIRE_EQUAL(SinogramCreatorTools::roundToNearesMultiplicity(0.02f, 0.01f, 30.f), 3002u);
  BOOST_REQUIRE_EQUAL(SinogramCreatorTools::roundToNearesMultiplicity(-30.f, 0.01f, 30.f), 0u);
  BOOST_REQUIRE_EQUAL(SinogramCreatorTools::roundToNearesMultiplicity(-29.9f, 0.01f, 30.f), 10u);
}

BOOST_AUTO_TEST_SUITE_END()
