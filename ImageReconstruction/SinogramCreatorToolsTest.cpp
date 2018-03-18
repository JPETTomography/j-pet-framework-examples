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

BOOST_AUTO_TEST_CASE( test_line_intersection )
{
  const float EPSILON = 0.00001f;
  std::pair<float, float> p1 = std::make_pair(0.f, 0.f);
  std::pair<float, float> p2 = std::make_pair(1.f, 0.f);
  std::pair<float, float> p3 = std::make_pair(0.5f, 0.5f);
  std::pair<float, float> p4 = std::make_pair(0.5f, -0.5f);
  BOOST_REQUIRE_EQUAL(SinogramCreatorTools::lineIntersection(p1, p2, p3, p4), std::make_pair(0.5f, 0.0f));

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

/*BOOST_AUTO_TEST_CASE( test_MC_data )
{
  const std::string inFile = "unitTestData/SinogramCreatorToolsTest/mc_data.txt";
  const std::string fOutFileName = "result.ppm";
  std::vector<std::pair<SinogramCreatorTools::Point, SinogramCreatorTools::Point>> hits;
  std::ifstream in(inFile);
  BOOST_REQUIRE(in);
  float x1, y1, x2, y2;
  while (in.peek() != EOF) {
    in >> x1 >> y1 >> x2 >> y2;
    hits.push_back(std::make_pair(std::make_pair(x1, y1), std::make_pair(x2, y2)));
  }
  float fReconstructionStartAngle = 0.;
  float fReconstructionEndAngle = 180.;
  float fReconstructionAngleStep = 1.;
  float fReconstructionLayerRadius =  10.;
  float fReconstructionDistanceAccuracy = 1;
  using SinogramResultType = std::vector<std::vector<unsigned int>>;
  SinogramResultType* fSinogram = nullptr;
  unsigned int maxThetaNumber = std::ceil(180. / fReconstructionAngleStep);
  unsigned int maxDistanceNumber = std::ceil(fReconstructionLayerRadius * 2 * (1.f / fReconstructionDistanceAccuracy)) + 1;
  if (fSinogram == nullptr) {
    fSinogram = new SinogramResultType(maxDistanceNumber, (std::vector<unsigned int>(maxThetaNumber)));
  }
  unsigned int currentValueInSinogram = 0;
  unsigned int fMaxValueInSinogram = 0;
  std::cout << "Number of hits: " << hits.size() << std::endl;
  std::cout << "Hit[0]: x:" << hits[0].first.first << " y:" << hits[0].first.second << " x:" << hits[0].second.first << " y:" << hits[0].second.second << std::endl;
  for (int i = 0; i < hits.size(); i++) {
    for (float theta = fReconstructionStartAngle; theta < fReconstructionEndAngle; theta += fReconstructionAngleStep) {
      float x = fReconstructionLayerRadius * std::cos(theta * (M_PI / 180.f)); // calculate x,y positon of line with theta angle from line (0,0) = theta
      float y = fReconstructionLayerRadius * std::sin(theta * (M_PI / 180.f));
      std::pair<float, float> intersectionPoint = SinogramCreatorTools::lineIntersection(std::make_pair(-x, -y), std::make_pair(x, y),
          hits[i].first, hits[i].second); //find intersection point
      if (intersectionPoint.first != std::numeric_limits<float>::max() && intersectionPoint.second != std::numeric_limits<float>::max()) {
        // check is there is intersection point
        float distance = SinogramCreatorTools::length2D(intersectionPoint.first, intersectionPoint.second);
        if (std::abs(distance) < 0.000001) {
          std::cout << "Distance: " << distance << " intersectionPoint: " << intersectionPoint.first << "," << intersectionPoint.second << " x,y: " << x << "," << y << " p1: " << hits[i].first.first << "," << hits[i].first.second << " p2: " << hits[i].second.first << "," << hits[i].second.second << std::endl;
        }
        if (distance >= fReconstructionLayerRadius) // if distance is greather then our max reconstuction layer radius, it cant be placed in sinogram
          continue;
        if (intersectionPoint.first < 0.f)
          distance = -distance;
        int distanceRound = std::floor((fReconstructionLayerRadius / fReconstructionDistanceAccuracy) + fReconstructionDistanceAccuracy) + std::floor((distance / fReconstructionDistanceAccuracy) + (fReconstructionDistanceAccuracy / 2.)); //clever way of rounding to nearest multipicity of accuracy digit and change it to int
        if (distanceRound >= maxDistanceNumber)
          distanceRound = maxDistanceNumber - 1;
        int thetaNumber = std::round(theta / fReconstructionAngleStep);                                                                                                                                                                // round because of floating point
        currentValueInSinogram = ++fSinogram->at(distanceRound).at(thetaNumber);                                                                                                                                                       // add to sinogram
        if (currentValueInSinogram >= fMaxValueInSinogram)
          fMaxValueInSinogram = currentValueInSinogram;                                    // save max value of sinogram
      }
    }
  }
  std::ofstream res(fOutFileName);
  res << "P2" << std::endl;
  res << (*fSinogram)[0].size() << " " << fSinogram->size() << std::endl;
  res << fMaxValueInSinogram << std::endl;
  for (unsigned int i = 0; i < fSinogram->size(); i++) {
    for (unsigned int j = 0; j < (*fSinogram)[0].size(); j++) {
      res << (*fSinogram)[i][j] << " ";
    }
    res << std::endl;
  }
  res.close();
  BOOST_REQUIRE(JPetCommonTools::ifFileExisting(fOutFileName));
}*/

BOOST_AUTO_TEST_SUITE_END()