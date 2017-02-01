#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE SignalFinderToolsTest
#include <boost/test/unit_test.hpp>

#include "SignalFinderTools.h"


BOOST_AUTO_TEST_SUITE(FirstSuite)
BOOST_AUTO_TEST_CASE( getSigChsPMMapById )
{
  JPetTimeWindow window;
  auto sigCh1 = JPetSigCh(JPetSigCh::Leading, 10);
  sigCh1.setThreshold(1);
  JPetPM pm1(1);
  sigCh1.setPM(pm1);

  auto sigCh2 = JPetSigCh(JPetSigCh::Leading, 11);
  sigCh2.setThreshold(2);
  auto sigCh3 = JPetSigCh(JPetSigCh::Leading, 12.5);
  sigCh2.setThreshold(1);

  JPetPM pm2(2);
  sigCh2.setPM(pm2);
  sigCh3.setPM(pm2);

  window.addCh(sigCh1);
  window.addCh(sigCh2);
  window.addCh(sigCh3);

  auto results =  SignalFinderTools::getSigChsPMMapById(&window);
  BOOST_REQUIRE_EQUAL(results.size(), 2);
  BOOST_REQUIRE(results.find(1) != results.end());
  BOOST_REQUIRE(results.find(2) != results.end());
  BOOST_REQUIRE_EQUAL(results.at(1).size(), 1);
  BOOST_REQUIRE_EQUAL(results.at(2).size(), 2);
  auto epsilon = 0.0001;
  BOOST_REQUIRE_CLOSE(results.at(1).at(0).getValue(), 10, epsilon);
  BOOST_REQUIRE_CLOSE(results.at(2).at(0).getValue(), 11, epsilon);
  BOOST_REQUIRE_CLOSE(results.at(2).at(1).getValue(), 12.5, epsilon);
}

BOOST_AUTO_TEST_CASE( buildRawSignals_empty )
{
  JPetStatistics stats;
  std::vector<JPetSigCh> sigChFromSamePM;
  auto results =  SignalFinderTools::buildRawSignals(4, sigChFromSamePM, 1, stats, false, 5, 5);
  BOOST_REQUIRE(results.empty());
}
BOOST_AUTO_TEST_CASE( buildRawSignals_one_signal )
{
  JPetStatistics stats;
  auto sigCh1 = JPetSigCh(JPetSigCh::Leading, 10);
  sigCh1.setThreshold(1);

  std::vector<JPetSigCh> sigChFromSamePM = {sigCh1};
  auto results =  SignalFinderTools::buildRawSignals(4, sigChFromSamePM, 1, stats, false, 5, 5);
  BOOST_REQUIRE_EQUAL(results.size(), 1);
//BOOST_REQUIRE_EQUAL(results[0].getTimeWindowIndex(), 4);
}

//BOOST_AUTO_TEST_CASE( buildRawSignals )
//{
//JPetStatistics stats;
//auto sigCh1 =JPetSigCh(JPetSigCh::Leading, 10);
//sigCh1.setThreshold(1);

//std::vector<JPetSigCh> sigChFromSamePM = {sigCh1};
//auto results =  SignalFinderTools::buildRawSignals(4, sigChFromSamePM, 1, stats, false, 5,5);
//BOOST_REQUIRE_EQUAL(results.size(), 1);
////BOOST_REQUIRE_EQUAL(results[0].getTimeWindowIndex(), 4);
//}

BOOST_AUTO_TEST_SUITE_END()
