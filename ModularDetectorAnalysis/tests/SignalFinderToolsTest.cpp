/**
 *  @copyright Copyright 2020 The J-PET Framework Authors. All rights reserved.
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

#include <JPetParamBank/JPetParamBank.h>
#include <boost/test/unit_test.hpp>
#include "../SignalFinderTools.h"

BOOST_AUTO_TEST_SUITE(SignalFinderTestSuite)

BOOST_AUTO_TEST_CASE(getSigChByPM_nullPointer_test)
{
  auto results = SignalFinderTools::getSigChByPM(nullptr);
  BOOST_REQUIRE(results.empty());
}

BOOST_AUTO_TEST_CASE(getSigChByPM_Test)
{
  JPetPM pm1(1, JPetPM::SideA, "first", 1);
  JPetChannel channel1(1, 1, 50.0);
  channel1.setPM(pm1);

  JPetSigCh sigChA1(JPetSigCh::Leading, 10.0);
  JPetSigCh sigChA2(JPetSigCh::Leading, 11.0);
  JPetSigCh sigChA3(JPetSigCh::Leading, 12.5);
  sigChA1.setChannel(channel1);
  sigChA2.setChannel(channel1);
  sigChA3.setChannel(channel1);

  JPetPM pm2(2, JPetPM::SideA, "second", 2);
  JPetChannel channel2(2, 2, 100.0);
  channel2.setPM(pm2);

  JPetSigCh sigChB1(JPetSigCh::Leading, 1.0);
  JPetSigCh sigChB2(JPetSigCh::Leading, 2.0);
  JPetSigCh sigChB3(JPetSigCh::Leading, 3.5);
  sigChB1.setChannel(channel2);
  sigChB2.setChannel(channel2);
  sigChB3.setChannel(channel2);

  JPetPM pm3(3, JPetPM::SideA, "third", 3);
  JPetChannel channel3(3, 3, 456.0);
  channel3.setPM(pm3);

  JPetSigCh sigChC1(JPetSigCh::Leading, 5.0);
  JPetSigCh sigChC2(JPetSigCh::Leading, 6.0);
  sigChC1.setChannel(channel3);
  sigChC2.setChannel(channel3);

  JPetTimeWindow slot("JPetSigCh");
  slot.add<JPetSigCh>(sigChA1);
  slot.add<JPetSigCh>(sigChA2);
  slot.add<JPetSigCh>(sigChA3);
  slot.add<JPetSigCh>(sigChB1);
  slot.add<JPetSigCh>(sigChB2);
  slot.add<JPetSigCh>(sigChB3);
  slot.add<JPetSigCh>(sigChC1);
  slot.add<JPetSigCh>(sigChC2);

  auto results = SignalFinderTools::getSigChByPM(&slot);

  BOOST_REQUIRE_EQUAL(results.size(), 3);
  BOOST_REQUIRE_EQUAL(results[1].size(), 3);
  BOOST_REQUIRE_EQUAL(results[2].size(), 3);
  BOOST_REQUIRE_EQUAL(results[3].size(), 2);
}

BOOST_AUTO_TEST_CASE(buildRawSignals_empty)
{
  JPetStatistics stats;
  std::vector<JPetSigCh> sigChByPM;
  auto results = SignalFinderTools::buildRawSignals(
    sigChByPM, 5.0, 5.0, 2, stats, false
  );
  BOOST_REQUIRE(results.empty());
}

BOOST_AUTO_TEST_CASE(buildRawSignals_one_signal)
{
  JPetPM pm1(1, JPetPM::SideA, "first", 1);
  JPetChannel channel1(1, 1, 50.0);
  channel1.setPM(pm1);

  JPetSigCh sigCh1(JPetSigCh::Leading, 10.0);
  sigCh1.setChannel(channel1);

  std::vector<JPetSigCh> sigChVec;
  sigChVec.push_back(sigCh1);

  JPetStatistics stats;
  double sigChEdgeMaxTime = 5.0;
  double sigChLeadTrailMaxTime = 5.0;
  auto results = SignalFinderTools::buildRawSignals(
    sigChVec, sigChEdgeMaxTime, sigChLeadTrailMaxTime, 2, stats, false
  );

  auto points_lead = results.at(0).getPoints(JPetSigCh::Leading);
  auto points_trail = results.at(0).getPoints(JPetSigCh::Trailing);

  auto epsilon = 0.0001;
  BOOST_REQUIRE_EQUAL(results.size(), 1);
  BOOST_REQUIRE_EQUAL(points_lead.size(), 1);
  BOOST_REQUIRE_EQUAL(points_trail.size(), 0);
  BOOST_REQUIRE_CLOSE(points_lead.at(0).getTime(), 10.0, epsilon);
  BOOST_REQUIRE_EQUAL(points_lead.at(0).getChannel().getID(), 1);
  BOOST_REQUIRE_EQUAL(points_lead.at(0).getChannel().getThresholdNumber(), 1);
  BOOST_REQUIRE_CLOSE(points_lead.at(0).getChannel().getThresholdValue(), 50.0, epsilon);
}

BOOST_AUTO_TEST_CASE(buildRawSignals_2) {
  JPetPM pm1(1, JPetPM::SideA, "first", 1);
  JPetChannel channel1(1, 1, 50.0);
  JPetChannel channel2(2, 2, 150.0);
  channel1.setPM(pm1);
  channel2.setPM(pm1);

  JPetSigCh sigCh1(JPetSigCh::Leading, 9.);
  JPetSigCh sigCh2(JPetSigCh::Leading, 5.);
  JPetSigCh sigCh3(JPetSigCh::Trailing, 16.);

  sigCh1.setChannel(channel1);
  sigCh2.setChannel(channel2);
  sigCh3.setChannel(channel1);

  std::vector<JPetSigCh> sigChFromSamePM;
  sigChFromSamePM.push_back(sigCh1);
  sigChFromSamePM.push_back(sigCh2);
  sigChFromSamePM.push_back(sigCh3);

  double sigChEdgeMaxTime = 5.;
  double sigChLeadTrailMaxTime = 10.;

  JPetStatistics stats;
  auto results =  SignalFinderTools::buildRawSignals(
    sigChFromSamePM, sigChEdgeMaxTime , sigChLeadTrailMaxTime, 2, stats, false
  );
  BOOST_REQUIRE_EQUAL(results.size(), 1);
  auto points_trail = results.at(0).getPoints(JPetSigCh::Trailing);
  auto points_lead = results.at(0).getPoints(JPetSigCh::Leading);
  auto epsilon = 0.0001;
  BOOST_REQUIRE_EQUAL(points_lead.size(), 2);
  BOOST_REQUIRE_EQUAL(points_trail.size(), 1);
  BOOST_REQUIRE_CLOSE(points_lead.at(0).getTime(), 9.0, epsilon);
  BOOST_REQUIRE_CLOSE(points_lead.at(1).getTime(), 5.0, epsilon);
  BOOST_REQUIRE_CLOSE(points_trail.at(0).getTime(), 16.0, epsilon);
}

BOOST_AUTO_TEST_CASE(findSigChOnNextThr_empty)
{
  std::vector<JPetSigCh> empty;
  BOOST_REQUIRE_EQUAL(SignalFinderTools::findSigChOnNextThr(1.0, 10.0, empty), -1);
}

BOOST_AUTO_TEST_CASE(findSigChOnNextThr)
{
  std::vector<JPetSigCh> sigCh1 = {
    JPetSigCh(JPetSigCh::Leading, 2), JPetSigCh(JPetSigCh::Leading, 5)
  };

  BOOST_REQUIRE_EQUAL(SignalFinderTools::findSigChOnNextThr(1.0, 10.0, sigCh1), 0);

  std::vector<JPetSigCh> sigCh2 = {
    JPetSigCh(JPetSigCh::Leading, 7), JPetSigCh(JPetSigCh::Leading, 3)
  };
  BOOST_REQUIRE_EQUAL(SignalFinderTools::findSigChOnNextThr(1.0, 10.0, sigCh2), 0);

  std::vector<JPetSigCh> sigCh3 = {
    JPetSigCh(JPetSigCh::Leading, 15), JPetSigCh(JPetSigCh::Leading, 3)
  };
  BOOST_REQUIRE_EQUAL(SignalFinderTools::findSigChOnNextThr(1.0, 10.0, sigCh3), 1);

  std::vector<JPetSigCh> sigCh4 = {
    JPetSigCh(JPetSigCh::Leading, 15), JPetSigCh(JPetSigCh::Leading, 20)
  };
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
  JPetPM pm1(1, JPetPM::SideA, "first", 1);
  JPetChannel channel1(1, 1, 50.0);

  JPetSigCh lead(JPetSigCh::Leading, 1.0);
  lead.setChannel(channel1);

  JPetSigCh trail1(JPetSigCh::Trailing, 2.0);
  JPetSigCh trail2(JPetSigCh::Trailing, 5.0);

  trail1.setChannel(channel1);
  trail2.setChannel(channel1);

  std::vector<JPetSigCh> trailings;
  trailings.push_back(trail1);
  trailings.push_back(trail2);

  BOOST_REQUIRE_EQUAL(SignalFinderTools::findTrailingSigCh(lead, 10.0, trailings), 0);

  JPetSigCh trail3(JPetSigCh::Trailing, 12.0);
  JPetSigCh trail4(JPetSigCh::Trailing, 2.0);

  trail3.setChannel(channel1);
  trail4.setChannel(channel1);

  std::vector<JPetSigCh> trailings2;
  trailings2.push_back(trail3);
  trailings2.push_back(trail4);
  BOOST_REQUIRE_EQUAL(SignalFinderTools::findTrailingSigCh(lead, 10.0, trailings2), 1);

  JPetSigCh trail5(JPetSigCh::Trailing, 2.0);
  JPetSigCh trail6(JPetSigCh::Trailing, 4.0);
  trail5.setChannel(channel1);
  trail6.setChannel(channel1);

  std::vector<JPetSigCh> trailings3;
  trailings3.push_back(trail5);
  trailings3.push_back(trail6);
  BOOST_REQUIRE_EQUAL(SignalFinderTools::findTrailingSigCh(lead, 10.0, trailings3), 0);
}

BOOST_AUTO_TEST_SUITE_END()
