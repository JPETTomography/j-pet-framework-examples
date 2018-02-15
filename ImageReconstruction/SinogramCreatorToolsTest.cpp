#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE SinogramCreatorTest
#include <boost/test/unit_test.hpp>

#include "SinogramCreatorTools.h"
#include "JPetLoggerInclude.h"
#include <JPetHit/JPetHit.h>

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
  //for (const auto& r : result) {
  //  std::cout << std::get<0>(r) << " " << std::get<1>(r) << " " << std::get<2>(r) << std::endl;
  //}
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

BOOST_AUTO_TEST_SUITE_END()