#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE SignalFinderToolsTest

#include <boost/test/unit_test.hpp>
#include "SignalFinderTools.h"
#include "JPetLoggerInclude.h"

BOOST_AUTO_TEST_SUITE(FirstSuite)

BOOST_AUTO_TEST_CASE(getSigChsPMMapById_nullPointer)
{
  auto results =  SignalFinderTools::getSigChsPMMapById(nullptr);
  BOOST_REQUIRE(results.empty());
}

BOOST_AUTO_TEST_CASE(getSigChsPMMapById)
{
  JPetPM pm1(1, "first");
  JPetPM pm2(2, "second");

  auto sigCh1 = JPetSigCh(JPetSigCh::Leading, 10);
  auto sigCh2 = JPetSigCh(JPetSigCh::Leading, 11);
  auto sigCh3 = JPetSigCh(JPetSigCh::Leading, 12.5);

  sigCh1.setThreshold(1);
  sigCh2.setThreshold(2);
  sigCh3.setThreshold(3);

  sigCh1.setPM(pm1);
  sigCh2.setPM(pm2);
  sigCh3.setPM(pm2);

  JPetTimeWindow window("JPetSigCh");
  window.add<JPetSigCh>(sigCh1);
  window.add<JPetSigCh>(sigCh2);
  window.add<JPetSigCh>(sigCh3);

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

BOOST_AUTO_TEST_CASE(findTrailingSigCh_empty)
{
  auto window = 10;
  JPetSigCh lead(JPetSigCh::Leading, 1);
  std::vector<JPetSigCh> trailings;
  BOOST_REQUIRE_EQUAL(SignalFinderTools::findTrailingSigCh(lead, trailings, window), -1);
}

BOOST_AUTO_TEST_CASE(findTrailingSigCh)
{
  auto window = 10;
  JPetSigCh lead(JPetSigCh::Leading, 1);
  std::vector<JPetSigCh> trailings = {JPetSigCh(JPetSigCh::Trailing, 2), JPetSigCh(JPetSigCh::Trailing, 5)};
  BOOST_REQUIRE_EQUAL(SignalFinderTools::findTrailingSigCh(lead, trailings, window), 0);

  std::vector<JPetSigCh> trailings2 = {JPetSigCh(JPetSigCh::Trailing, 12), JPetSigCh(JPetSigCh::Trailing, 2)};
  BOOST_REQUIRE_EQUAL(SignalFinderTools::findTrailingSigCh(lead, trailings2, window),  1);

  std::vector<JPetSigCh> trailings3 = {JPetSigCh(JPetSigCh::Trailing, 2), JPetSigCh(JPetSigCh::Trailing, 4)};
  BOOST_REQUIRE_EQUAL(SignalFinderTools::findTrailingSigCh(lead, trailings3, window), 0);
}

BOOST_AUTO_TEST_CASE(findSigChOnNextThr_empty)
{
  auto window = 10;
  auto lead = 1;
  std::vector<JPetSigCh> others;
  BOOST_REQUIRE_EQUAL(SignalFinderTools::findSigChOnNextThr(lead, others, window), -1);
}

BOOST_AUTO_TEST_CASE(findSigChOnNextThr)
{
  auto window = 10;
  auto lead = 1;
  std::vector<JPetSigCh> others = {JPetSigCh(JPetSigCh::Leading, 2), JPetSigCh(JPetSigCh::Leading, 5)};
  BOOST_REQUIRE_EQUAL(SignalFinderTools::findSigChOnNextThr(lead, others, window), 0);

  std::vector<JPetSigCh> others2 = {JPetSigCh(JPetSigCh::Leading, 7), JPetSigCh(JPetSigCh::Leading, 3)};
  BOOST_REQUIRE_EQUAL(SignalFinderTools::findSigChOnNextThr(lead, others2, window), 0);

  std::vector<JPetSigCh> others3 = {JPetSigCh(JPetSigCh::Leading, 15), JPetSigCh(JPetSigCh::Leading, 3)};
  BOOST_REQUIRE_EQUAL(SignalFinderTools::findSigChOnNextThr(lead, others3, window), 1);

  std::vector<JPetSigCh> others4 = {JPetSigCh(JPetSigCh::Leading, 15), JPetSigCh(JPetSigCh::Leading, 20)};
  BOOST_REQUIRE_EQUAL(SignalFinderTools::findSigChOnNextThr(lead, others4, window), -1);
}

BOOST_AUTO_TEST_CASE(buildRawSignals_empty)
{
  JPetStatistics stats;
  std::vector<JPetSigCh> sigChFromSamePM;
  auto results =  SignalFinderTools::buildRawSignals(sigChFromSamePM, 1, stats, 5, 5, false);
  BOOST_REQUIRE(results.empty());
}

BOOST_AUTO_TEST_CASE(buildRawSignals_wrong_one_signal_NumOfThresholdsNot4)
{
  JPetStatistics stats;
  auto sigCh1 = JPetSigCh(JPetSigCh::Leading, 10);
  std::vector<JPetSigCh> sigChFromSamePM = {sigCh1};
  auto numOfThresholds = 1;
  auto results = SignalFinderTools::buildRawSignals(sigChFromSamePM, numOfThresholds, stats, 5, 5, false);
  BOOST_REQUIRE(results.empty());
}

BOOST_AUTO_TEST_CASE(buildRawSignals_one_signal)
{
  JPetStatistics stats;
  JPetBarrelSlot bs1(1, true, "some_slot", 57.7, 123);
  JPetPM pm1(1, "first");
  pm1.setBarrelSlot(bs1);
  auto sigCh1 = JPetSigCh(JPetSigCh::Leading, 10);
  sigCh1.setPM(pm1);
  sigCh1.setThresholdNumber(1);
  std::vector<JPetSigCh> sigChFromSamePM = {sigCh1};
  int numOfThresholds = 4;
  double sigChEdgeMaxTime = 5.;
  double sigChLeadTrailMaxTime = 5.;
  auto results = SignalFinderTools::buildRawSignals(
    sigChFromSamePM, numOfThresholds, stats, sigChEdgeMaxTime, sigChLeadTrailMaxTime, false);
  auto points_trail = results.at(0).getPoints(JPetSigCh::Trailing);
  auto points_lead = results.at(0).getPoints(JPetSigCh::Leading);
  auto epsilon = 0.0001;
  BOOST_REQUIRE_EQUAL(results.size(), 1);
  BOOST_REQUIRE_EQUAL(points_trail.size(), 0);
  BOOST_REQUIRE_EQUAL(points_lead.size(), 1);
  BOOST_REQUIRE_CLOSE(points_lead.at(0).getValue(), 10, epsilon);
}

BOOST_AUTO_TEST_CASE(buildRawSignals_2)
{
  JPetStatistics stats;
  auto sigCh1 = JPetSigCh(JPetSigCh::Leading, 9.);
  auto sigCh2 = JPetSigCh(JPetSigCh::Leading, 5.);
  auto sigCh3 = JPetSigCh(JPetSigCh::Trailing, 6.);
  sigCh1.setThresholdNumber(1);
  sigCh2.setThresholdNumber(2);
  sigCh3.setThresholdNumber(1);
  std::vector<JPetSigCh> sigChFromSamePM = {sigCh1, sigCh2, sigCh3};
  auto numOfThresholds = 4;
  double sigChEdgeMaxTime = 5.;
  double sigChLeadTrailMaxTime = 5.;
  auto results =  SignalFinderTools::buildRawSignals(
    sigChFromSamePM, numOfThresholds, stats,
    sigChEdgeMaxTime , sigChLeadTrailMaxTime, false);
  auto points_trail = results.at(0).getPoints(JPetSigCh::Trailing);
  auto points_lead = results.at(0).getPoints(JPetSigCh::Leading);
  auto epsilon = 0.0001;
  BOOST_REQUIRE_EQUAL(results.size(), 1);
  BOOST_REQUIRE_EQUAL(points_trail.size(), 1);
  BOOST_REQUIRE_EQUAL(points_lead.size(), 2);
  BOOST_REQUIRE_CLOSE(points_lead.at(0).getValue(), 9., epsilon);
  BOOST_REQUIRE_CLOSE(points_lead.at(1).getValue(), 5., epsilon);
  BOOST_REQUIRE_CLOSE(points_trail.at(0).getValue(), 6., epsilon);
}

BOOST_AUTO_TEST_SUITE_END()
