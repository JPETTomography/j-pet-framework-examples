/**
 *  @copyright Copyright 2018 The J-PET Framework Authors. All rights reserved.
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may find a copy of the License in the LICENCE file.
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  @file SignalFinderToolsTest.cpp
 */

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE SignalFinderToolsTest

#include <boost/test/unit_test.hpp>
#include "SignalFinderTools.h"
#include "JPetLoggerInclude.h"

BOOST_AUTO_TEST_SUITE(SignalFinderTestSuite)

BOOST_AUTO_TEST_CASE(getSigChByPM_nullPointer_test)
{
  auto results = SignalFinderTools::getSigChByPM(nullptr, false);
  BOOST_REQUIRE(results.empty());
}

BOOST_AUTO_TEST_CASE(getSigChByPM_Test)
{
  JPetPM pm1(1, "first");
  JPetSigCh sigChA1(JPetSigCh::Leading, 10.0);
  JPetSigCh sigChA2(JPetSigCh::Leading, 11.0);
  JPetSigCh sigChA3(JPetSigCh::Leading, 12.5);
  sigChA1.setPM(pm1);
  sigChA2.setPM(pm1);
  sigChA3.setPM(pm1);
  sigChA1.setRecoFlag(JPetSigCh::Good);
  sigChA2.setRecoFlag(JPetSigCh::Good);
  sigChA3.setRecoFlag(JPetSigCh::Good);

  JPetPM pm2(2, "second");
  JPetSigCh sigChB1(JPetSigCh::Leading, 1.0);
  JPetSigCh sigChB2(JPetSigCh::Leading, 2.0);
  JPetSigCh sigChB3(JPetSigCh::Leading, 3.5);
  sigChB1.setPM(pm2);
  sigChB2.setPM(pm2);
  sigChB3.setPM(pm2);
  sigChB1.setRecoFlag(JPetSigCh::Corrupted);
  sigChB2.setRecoFlag(JPetSigCh::Corrupted);
  sigChB3.setRecoFlag(JPetSigCh::Corrupted);

  JPetPM pm3(3, "third");
  JPetSigCh sigChC1(JPetSigCh::Leading, 5.0);
  JPetSigCh sigChC2(JPetSigCh::Leading, 6.0);
  sigChC1.setPM(pm3);
  sigChC2.setPM(pm3);
  sigChC1.setRecoFlag(JPetSigCh::Good);
  sigChC2.setRecoFlag(JPetSigCh::Good);

  JPetTimeWindow slot("JPetSigCh");
  slot.add<JPetSigCh>(sigChA1);
  slot.add<JPetSigCh>(sigChA2);
  slot.add<JPetSigCh>(sigChA3);
  slot.add<JPetSigCh>(sigChB1);
  slot.add<JPetSigCh>(sigChB2);
  slot.add<JPetSigCh>(sigChB3);
  slot.add<JPetSigCh>(sigChC1);
  slot.add<JPetSigCh>(sigChC2);

  auto results1 = SignalFinderTools::getSigChByPM(&slot, false);
  auto results2 = SignalFinderTools::getSigChByPM(&slot, true);

  BOOST_REQUIRE_EQUAL(results1.size(), 3);
  BOOST_REQUIRE_EQUAL(results2.size(), 3);
  BOOST_REQUIRE_EQUAL(results1.at(0).size(), 3);
  BOOST_REQUIRE_EQUAL(results1.at(1).size(), 0);
  BOOST_REQUIRE_EQUAL(results1.at(2).size(), 2);
  BOOST_REQUIRE_EQUAL(results2.at(0).size(), 3);
  BOOST_REQUIRE_EQUAL(results2.at(1).size(), 3);
  BOOST_REQUIRE_EQUAL(results2.at(2).size(), 2);
}

BOOST_AUTO_TEST_CASE(buildRawSignals_empty)
{
  JPetStatistics stats;
  std::map<int, std::vector<JPetSigCh>> sigChByPM;
  auto results = SignalFinderTools::buildRawSignals(
    sigChByPM, 1, 5.0, 5.0, stats, false
  );
  BOOST_REQUIRE(results.empty());
}

BOOST_AUTO_TEST_CASE(buildRawSignals_wrong_one_signal_NumOfThresholdsNot4)
{
  JPetPM pm1(1, "first");
  JPetSigCh sigCh1(JPetSigCh::Leading, 10);
  sigCh1.setPM(pm1);
  std::vector<JPetSigCh> sigChVec;
  sigChVec.push_back(sigCh1);
  auto numOfThresholds = 1;
  JPetStatistics stats;
  auto results = SignalFinderTools::buildRawSignals(
    sigChVec, numOfThresholds, 5.0, 5.0, stats, false
  );
  BOOST_REQUIRE(results.empty());
}

BOOST_AUTO_TEST_CASE(buildRawSignals_one_signal)
{
  JPetStatistics stats;
  JPetBarrelSlot bs1(1, true, "some_slot", 57.7, 123);
  JPetPM pm1(1, "first");
  pm1.setBarrelSlot(bs1);
  JPetSigCh sigCh1(JPetSigCh::Leading, 10);
  sigCh1.setPM(pm1);
  sigCh1.setThresholdNumber(1);
  std::vector<JPetSigCh> sigChVec;
  sigChVec.push_back(sigCh1);
  int numOfThresholds = 4;
  double sigChEdgeMaxTime = 5.0;
  double sigChLeadTrailMaxTime = 5.0;
  auto results = SignalFinderTools::buildRawSignals(
    sigChVec, numOfThresholds, sigChEdgeMaxTime, sigChLeadTrailMaxTime, stats, false
  );
  auto points_trail = results.at(0).getPoints(JPetSigCh::Trailing);
  auto points_lead = results.at(0).getPoints(JPetSigCh::Leading);
  auto epsilon = 0.0001;
  BOOST_REQUIRE_EQUAL(results.size(), 1);
  BOOST_REQUIRE_EQUAL(points_trail.size(), 0);
  BOOST_REQUIRE_EQUAL(points_lead.size(), 1);
  BOOST_REQUIRE_CLOSE(points_lead.at(0).getValue(), 10.0, epsilon);
}

BOOST_AUTO_TEST_CASE(buildRawSignals_2)
{
  JPetBarrelSlot bs1(1, true, "some_slot", 57.7, 123);
  JPetPM pm1(1, "first");
  pm1.setBarrelSlot(bs1);
  JPetSigCh sigCh1(JPetSigCh::Leading, 9.);
  JPetSigCh sigCh2(JPetSigCh::Leading, 5.);
  JPetSigCh sigCh3(JPetSigCh::Trailing, 6.);
  sigCh1.setPM(pm1);
  sigCh2.setPM(pm1);
  sigCh3.setPM(pm1);
  sigCh1.setThresholdNumber(1);
  sigCh2.setThresholdNumber(2);
  sigCh3.setThresholdNumber(1);
  std::vector<JPetSigCh> sigChFromSamePM;
  sigChFromSamePM.push_back(sigCh1);
  sigChFromSamePM.push_back(sigCh2);
  sigChFromSamePM.push_back(sigCh3);
  auto numOfThresholds = 4;
  double sigChEdgeMaxTime = 5.;
  double sigChLeadTrailMaxTime = 5.;
  JPetStatistics stats;
  auto results =  SignalFinderTools::buildRawSignals(
    sigChFromSamePM, numOfThresholds, sigChEdgeMaxTime , sigChLeadTrailMaxTime, stats, false
  );
  auto points_trail = results.at(0).getPoints(JPetSigCh::Trailing);
  auto points_lead = results.at(0).getPoints(JPetSigCh::Leading);
  auto epsilon = 0.0001;
  BOOST_REQUIRE_EQUAL(results.size(), 1);
  BOOST_REQUIRE_EQUAL(points_trail.size(), 1);
  BOOST_REQUIRE_EQUAL(points_lead.size(), 2);
  BOOST_REQUIRE_CLOSE(points_lead.at(0).getValue(), 9.0, epsilon);
  BOOST_REQUIRE_CLOSE(points_lead.at(1).getValue(), 5.0, epsilon);
  BOOST_REQUIRE_CLOSE(points_trail.at(0).getValue(), 6.0, epsilon);
}

BOOST_AUTO_TEST_CASE(buildRawSignals_test_flag_inherit)
{
  JPetBarrelSlot bs1(1, true, "some_slot", 57.7, 123);
  JPetPM pm1(1, "first");
  pm1.setBarrelSlot(bs1);

  JPetSigCh sigChA1(JPetSigCh::Leading, 1.0);
  JPetSigCh sigChA2(JPetSigCh::Trailing, 2.0);
  sigChA1.setPM(pm1);
  sigChA2.setPM(pm1);
  sigChA1.setThresholdNumber(1);
  sigChA2.setThresholdNumber(1);
  sigChA1.setRecoFlag(JPetSigCh::Good);
  sigChA2.setRecoFlag(JPetSigCh::Good);

  JPetSigCh sigChB1(JPetSigCh::Leading, 10.0);
  JPetSigCh sigChB2(JPetSigCh::Trailing, 12.0);
  sigChB1.setPM(pm1);
  sigChB2.setPM(pm1);
  sigChB1.setThresholdNumber(1);
  sigChB2.setThresholdNumber(1);
  sigChB1.setRecoFlag(JPetSigCh::Corrupted);
  sigChB2.setRecoFlag(JPetSigCh::Good);

  std::vector<JPetSigCh> sigChFromSamePM;
  sigChFromSamePM.push_back(sigChA1);
  sigChFromSamePM.push_back(sigChA2);
  sigChFromSamePM.push_back(sigChB1);
  sigChFromSamePM.push_back(sigChB2);

  auto numOfThresholds = 4;
  double sigChEdgeMaxTime = 5.0;
  double sigChLeadTrailMaxTime = 12.0;
  JPetStatistics stats;
  auto results = SignalFinderTools::buildRawSignals(
    sigChFromSamePM, numOfThresholds, sigChEdgeMaxTime, sigChLeadTrailMaxTime, stats, false
  );
  BOOST_REQUIRE_EQUAL(results.size(), 2);
  BOOST_REQUIRE_EQUAL(results.at(0).getRecoFlag(), JPetBaseSignal::Good);
  BOOST_REQUIRE_EQUAL(results.at(1).getRecoFlag(), JPetBaseSignal::Corrupted);
}

BOOST_AUTO_TEST_CASE(findSigChOnNextThr_empty)
{
  std::vector<JPetSigCh> empty;
  BOOST_REQUIRE_EQUAL(SignalFinderTools::findSigChOnNextThr(1.0, 10.0, empty), -1);
}

BOOST_AUTO_TEST_CASE(findSigChOnNextThr)
{
  std::vector<JPetSigCh> sigCh1 = {JPetSigCh(JPetSigCh::Leading, 2), JPetSigCh(JPetSigCh::Leading, 5)};
  BOOST_REQUIRE_EQUAL(SignalFinderTools::findSigChOnNextThr(1.0, 10.0, sigCh1), 0);

  std::vector<JPetSigCh> sigCh2 = {JPetSigCh(JPetSigCh::Leading, 7), JPetSigCh(JPetSigCh::Leading, 3)};
  BOOST_REQUIRE_EQUAL(SignalFinderTools::findSigChOnNextThr(1.0, 10.0, sigCh2), 0);

  std::vector<JPetSigCh> sigCh3 = {JPetSigCh(JPetSigCh::Leading, 15), JPetSigCh(JPetSigCh::Leading, 3)};
  BOOST_REQUIRE_EQUAL(SignalFinderTools::findSigChOnNextThr(1.0, 10.0, sigCh3), 1);

  std::vector<JPetSigCh> sigCh4 = {JPetSigCh(JPetSigCh::Leading, 15), JPetSigCh(JPetSigCh::Leading, 20)};
  BOOST_REQUIRE_EQUAL(SignalFinderTools::findSigChOnNextThr(1.0, 10.0, sigCh4), -1);
}

BOOST_AUTO_TEST_CASE(findTrailingSigCh_empty)
{
  JPetSigCh lead(JPetSigCh::Leading, 1);
  std::vector<JPetSigCh> trailings;
  BOOST_REQUIRE_EQUAL(SignalFinderTools::findTrailingSigCh(lead, 10.0, trailings), -1);
}

BOOST_AUTO_TEST_CASE(findTrailingSigCh)
{
  JPetBarrelSlot bs1(1, true, "some_slot", 57.7, 123);
  JPetPM pm1(1, "first");
  pm1.setBarrelSlot(bs1);

  JPetSigCh lead(JPetSigCh::Leading, 1.0);
  lead.setPM(pm1);

  JPetSigCh trail1(JPetSigCh::Trailing, 2.0);
  JPetSigCh trail2(JPetSigCh::Trailing, 5.0);
  trail1.setPM(pm1);
  trail2.setPM(pm1);
  std::vector<JPetSigCh> trailings;
  trailings.push_back(trail1);
  trailings.push_back(trail2);
  BOOST_REQUIRE_EQUAL(SignalFinderTools::findTrailingSigCh(lead, 10.0, trailings), 0);

  JPetSigCh trail3(JPetSigCh::Trailing, 12.0);
  JPetSigCh trail4(JPetSigCh::Trailing, 2.0);
  trail3.setPM(pm1);
  trail4.setPM(pm1);
  std::vector<JPetSigCh> trailings2;
  trailings2.push_back(trail3);
  trailings2.push_back(trail4);
  BOOST_REQUIRE_EQUAL(SignalFinderTools::findTrailingSigCh(lead, 10.0, trailings2), 1);

  JPetSigCh trail5(JPetSigCh::Trailing, 2.0);
  JPetSigCh trail6(JPetSigCh::Trailing, 4.0);
  trail5.setPM(pm1);
  trail6.setPM(pm1);
  std::vector<JPetSigCh> trailings3;
  trailings3.push_back(trail5);
  trailings3.push_back(trail6);
  BOOST_REQUIRE_EQUAL(SignalFinderTools::findTrailingSigCh(lead, 10.0, trailings3), 0);
}

BOOST_AUTO_TEST_SUITE_END()
