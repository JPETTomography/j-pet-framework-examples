/**
 *  @copyright Copyright 2019 The J-PET Framework Authors. All rights reserved.
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
#include "../SignalFinderTools.h"
#include <boost/test/unit_test.hpp>
#include <JPetParamBank/JPetParamBank.h>
#include "JPetLoggerInclude.h"

BOOST_AUTO_TEST_SUITE(SignalFinderTestSuite)

BOOST_AUTO_TEST_CASE(getSigChByPM_nullPointer_test)
{
  auto results = SignalFinderTools::getSigChByPM(nullptr, false, -1);
  BOOST_REQUIRE(results.empty());
}

BOOST_AUTO_TEST_CASE(getSigChByPM_Test) {
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

  auto results1 = SignalFinderTools::getSigChByPM(&slot, false, -1);
  auto results2 = SignalFinderTools::getSigChByPM(&slot, true, -1);

  BOOST_REQUIRE_EQUAL(results1.size(), 2);
  BOOST_REQUIRE_EQUAL(results2.size(), 3);
  BOOST_REQUIRE_EQUAL(results1[1].size(), 3);
  BOOST_REQUIRE_EQUAL(results1[3].size(), 2);
  BOOST_REQUIRE_EQUAL(results2[1].size(), 3);
  BOOST_REQUIRE_EQUAL(results2[2].size(), 3);
  BOOST_REQUIRE_EQUAL(results2[3].size(), 2);
}

BOOST_AUTO_TEST_CASE(getSigChByPM_refDet_test)
{
  JPetPM pm(1, "not_reference");
  JPetPM pmRef(234, "reference");
  JPetSigCh sigCh1(JPetSigCh::Leading, 10.0);
  JPetSigCh sigCh2(JPetSigCh::Trailing, 11.0);
  JPetSigCh sigCh3(JPetSigCh::Leading, 12.0);
  JPetSigCh sigCh4(JPetSigCh::Trailing, 13.0);
  JPetSigCh sigCh5(JPetSigCh::Leading, 14.0);
  JPetSigCh sigCh6(JPetSigCh::Trailing, 15.0);
  sigCh1.setPM(pm);
  sigCh2.setPM(pmRef);
  sigCh3.setPM(pm);
  sigCh4.setPM(pmRef);
  sigCh5.setPM(pmRef);
  sigCh6.setPM(pm);
  sigCh1.setRecoFlag(JPetSigCh::Corrupted);
  sigCh2.setRecoFlag(JPetSigCh::Corrupted);
  sigCh3.setRecoFlag(JPetSigCh::Corrupted);
  sigCh4.setRecoFlag(JPetSigCh::Corrupted);
  sigCh5.setRecoFlag(JPetSigCh::Corrupted);
  sigCh6.setRecoFlag(JPetSigCh::Corrupted);

  JPetTimeWindow slot("JPetSigCh");
  slot.add<JPetSigCh>(sigCh1);
  slot.add<JPetSigCh>(sigCh2);
  slot.add<JPetSigCh>(sigCh3);
  slot.add<JPetSigCh>(sigCh4);
  slot.add<JPetSigCh>(sigCh5);
  slot.add<JPetSigCh>(sigCh6);

  auto results1 = SignalFinderTools::getSigChByPM(&slot, true, 234);
  auto results2 = SignalFinderTools::getSigChByPM(&slot, false, 234);

  BOOST_REQUIRE_EQUAL(results1.size(), 2);
  BOOST_REQUIRE_EQUAL(results2.size(), 1);
  BOOST_REQUIRE_EQUAL(results1[1].size(), 3);
  BOOST_REQUIRE_EQUAL(results1[234].size(), 3);
  BOOST_REQUIRE_EQUAL(results2[234].size(), 3);
}

BOOST_AUTO_TEST_CASE(buildRawSignals_empty)
{
  JPetStatistics stats;
  std::vector<JPetSigCh> sigChByPM;
  auto results = SignalFinderTools::buildRawSignals(
    sigChByPM, 5.0, 5.0, stats, false
  );
  BOOST_REQUIRE(results.empty());
}

BOOST_AUTO_TEST_CASE(buildRawSignals_one_signal) {
  JPetStatistics stats;
  JPetBarrelSlot bs1(1, true, "some_slot", 57.7, 123);
  JPetPM pm1(1, "first");
  pm1.setBarrelSlot(bs1);
  JPetSigCh sigCh1(JPetSigCh::Leading, 10);
  sigCh1.setPM(pm1);
  sigCh1.setThresholdNumber(1);
  std::vector<JPetSigCh> sigChVec;
  sigChVec.push_back(sigCh1);
  double sigChEdgeMaxTime = 5.0;
  double sigChLeadTrailMaxTime = 5.0;
  auto results = SignalFinderTools::buildRawSignals(
    sigChVec, sigChEdgeMaxTime, sigChLeadTrailMaxTime, stats, false
  );
  auto points_trail = results.at(0).getPoints(JPetSigCh::Trailing);
  auto points_lead = results.at(0).getPoints(JPetSigCh::Leading);
  auto epsilon = 0.0001;
  BOOST_REQUIRE_EQUAL(results.size(), 1);
  BOOST_REQUIRE_EQUAL(points_trail.size(), 0);
  BOOST_REQUIRE_EQUAL(points_lead.size(), 1);
  BOOST_REQUIRE_CLOSE(points_lead.at(0).getValue(), 10.0, epsilon);
}

BOOST_AUTO_TEST_CASE(buildRawSignals_2) {
  JPetBarrelSlot bs1(1, true, "some_slot", 57.7, 123);
  JPetPM pm1(1, "first");
  pm1.setBarrelSlot(bs1);
  JPetSigCh sigCh1(JPetSigCh::Leading, 9.);
  JPetSigCh sigCh2(JPetSigCh::Leading, 5.);
  JPetSigCh sigCh3(JPetSigCh::Trailing, 16.);
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
  double sigChEdgeMaxTime = 5.;
  double sigChLeadTrailMaxTime = 10.;
  JPetStatistics stats;
  auto results =  SignalFinderTools::buildRawSignals(
    sigChFromSamePM, sigChEdgeMaxTime , sigChLeadTrailMaxTime, stats, false
  );
  BOOST_REQUIRE_EQUAL(results.size(), 1);
  auto points_trail = results.at(0).getPoints(JPetSigCh::Trailing);
  auto points_lead = results.at(0).getPoints(JPetSigCh::Leading);
  auto epsilon = 0.0001;
  BOOST_REQUIRE_EQUAL(points_lead.size(), 2);
  BOOST_REQUIRE_EQUAL(points_trail.size(), 1);
  BOOST_REQUIRE_CLOSE(points_lead.at(0).getValue(), 9.0, epsilon);
  BOOST_REQUIRE_CLOSE(points_lead.at(1).getValue(), 5.0, epsilon);
  BOOST_REQUIRE_CLOSE(points_trail.at(0).getValue(), 16.0, epsilon);
}

BOOST_AUTO_TEST_CASE(buildRawSignals_test_flag_inherit) {
  JPetBarrelSlot bs1(1, true, "some_slot", 57.7, 123);
  JPetPM pm1(1, "first");
  pm1.setBarrelSlot(bs1);

  JPetSigCh sigChA1(JPetSigCh::Leading, 1.0);
  JPetSigCh sigChA2(JPetSigCh::Leading, 2.0);
  JPetSigCh sigChA3(JPetSigCh::Leading, 3.0);
  JPetSigCh sigChA4(JPetSigCh::Trailing, 4.0);
  JPetSigCh sigChA5(JPetSigCh::Trailing, 5.0);
  JPetSigCh sigChA6(JPetSigCh::Trailing, 6.0);
  sigChA1.setPM(pm1);
  sigChA2.setPM(pm1);
  sigChA3.setPM(pm1);
  sigChA4.setPM(pm1);
  sigChA5.setPM(pm1);
  sigChA6.setPM(pm1);
  sigChA1.setThresholdNumber(1);
  sigChA2.setThresholdNumber(2);
  sigChA3.setThresholdNumber(3);
  sigChA4.setThresholdNumber(3);
  sigChA5.setThresholdNumber(2);
  sigChA6.setThresholdNumber(1);
  sigChA1.setRecoFlag(JPetSigCh::Good);
  sigChA2.setRecoFlag(JPetSigCh::Good);
  sigChA3.setRecoFlag(JPetSigCh::Good);
  sigChA4.setRecoFlag(JPetSigCh::Good);
  sigChA5.setRecoFlag(JPetSigCh::Good);
  sigChA6.setRecoFlag(JPetSigCh::Good);

  JPetSigCh sigChB1(JPetSigCh::Leading, 10.0);
  JPetSigCh sigChB2(JPetSigCh::Leading, 11.0);
  JPetSigCh sigChB3(JPetSigCh::Leading, 12.0);
  JPetSigCh sigChB4(JPetSigCh::Leading, 13.0);
  JPetSigCh sigChB5(JPetSigCh::Trailing, 14.0);
  JPetSigCh sigChB6(JPetSigCh::Trailing, 15.0);
  JPetSigCh sigChB7(JPetSigCh::Trailing, 16.0);
  JPetSigCh sigChB8(JPetSigCh::Trailing, 17.0);
  sigChB1.setPM(pm1);
  sigChB2.setPM(pm1);
  sigChB3.setPM(pm1);
  sigChB4.setPM(pm1);
  sigChB5.setPM(pm1);
  sigChB6.setPM(pm1);
  sigChB7.setPM(pm1);
  sigChB8.setPM(pm1);

  sigChB1.setThresholdNumber(1);
  sigChB2.setThresholdNumber(2);
  sigChB3.setThresholdNumber(3);
  sigChB4.setThresholdNumber(4);
  sigChB5.setThresholdNumber(4);
  sigChB6.setThresholdNumber(3);
  sigChB7.setThresholdNumber(2);
  sigChB8.setThresholdNumber(1);

  sigChB1.setRecoFlag(JPetSigCh::Good);
  sigChB2.setRecoFlag(JPetSigCh::Good);
  sigChB3.setRecoFlag(JPetSigCh::Corrupted);
  sigChB4.setRecoFlag(JPetSigCh::Good);
  sigChB5.setRecoFlag(JPetSigCh::Corrupted);
  sigChB6.setRecoFlag(JPetSigCh::Good);
  sigChB7.setRecoFlag(JPetSigCh::Good);
  sigChB8.setRecoFlag(JPetSigCh::Good);

  std::vector<JPetSigCh> sigChFromSamePM;
  sigChFromSamePM.push_back(sigChA1);
  sigChFromSamePM.push_back(sigChA2);
  sigChFromSamePM.push_back(sigChA3);
  sigChFromSamePM.push_back(sigChA4);
  sigChFromSamePM.push_back(sigChA5);
  sigChFromSamePM.push_back(sigChA6);
  sigChFromSamePM.push_back(sigChB1);
  sigChFromSamePM.push_back(sigChB2);
  sigChFromSamePM.push_back(sigChB3);
  sigChFromSamePM.push_back(sigChB4);
  sigChFromSamePM.push_back(sigChB5);
  sigChFromSamePM.push_back(sigChB6);
  sigChFromSamePM.push_back(sigChB7);
  sigChFromSamePM.push_back(sigChB8);

  double sigChEdgeMaxTime = 5.0;
  double sigChLeadTrailMaxTime = 12.0;
  JPetStatistics stats;
  auto results = SignalFinderTools::buildRawSignals(
    sigChFromSamePM, sigChEdgeMaxTime, sigChLeadTrailMaxTime, stats, false
  );
  BOOST_REQUIRE_EQUAL(results.size(), 2);
  BOOST_REQUIRE_EQUAL(results.at(0).getRecoFlag(), JPetBaseSignal::Good);
  BOOST_REQUIRE_EQUAL(results.at(1).getRecoFlag(), JPetBaseSignal::Corrupted);
}

BOOST_AUTO_TEST_CASE(findSigChOnNextThr_empty) {
  std::vector<JPetSigCh> empty;
  BOOST_REQUIRE_EQUAL(SignalFinderTools::findSigChOnNextThr(1.0, 10.0, empty),
                      -1);
}

BOOST_AUTO_TEST_CASE(findSigChOnNextThr) {
  std::vector<JPetSigCh> sigCh1 = {JPetSigCh(JPetSigCh::Leading, 2),
                                   JPetSigCh(JPetSigCh::Leading, 5)};
  BOOST_REQUIRE_EQUAL(SignalFinderTools::findSigChOnNextThr(1.0, 10.0, sigCh1),
                      0);

  std::vector<JPetSigCh> sigCh2 = {JPetSigCh(JPetSigCh::Leading, 7),
                                   JPetSigCh(JPetSigCh::Leading, 3)};
  BOOST_REQUIRE_EQUAL(SignalFinderTools::findSigChOnNextThr(1.0, 10.0, sigCh2),
                      0);

  std::vector<JPetSigCh> sigCh3 = {JPetSigCh(JPetSigCh::Leading, 15),
                                   JPetSigCh(JPetSigCh::Leading, 3)};
  BOOST_REQUIRE_EQUAL(SignalFinderTools::findSigChOnNextThr(1.0, 10.0, sigCh3),
                      1);

  std::vector<JPetSigCh> sigCh4 = {JPetSigCh(JPetSigCh::Leading, 15),
                                   JPetSigCh(JPetSigCh::Leading, 20)};
  BOOST_REQUIRE_EQUAL(SignalFinderTools::findSigChOnNextThr(1.0, 10.0, sigCh4),
                      -1);
}

BOOST_AUTO_TEST_CASE(findTrailingSigCh_empty) {
  JPetSigCh lead(JPetSigCh::Leading, 1);
  std::vector<JPetSigCh> trailings;
  BOOST_REQUIRE_EQUAL(
      SignalFinderTools::findTrailingSigCh(lead, 10.0, trailings), -1);
}

BOOST_AUTO_TEST_CASE(findTrailingSigCh) {
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
  BOOST_REQUIRE_EQUAL(
      SignalFinderTools::findTrailingSigCh(lead, 10.0, trailings), 0);

  JPetSigCh trail3(JPetSigCh::Trailing, 12.0);
  JPetSigCh trail4(JPetSigCh::Trailing, 2.0);
  trail3.setPM(pm1);
  trail4.setPM(pm1);
  std::vector<JPetSigCh> trailings2;
  trailings2.push_back(trail3);
  trailings2.push_back(trail4);
  BOOST_REQUIRE_EQUAL(
      SignalFinderTools::findTrailingSigCh(lead, 10.0, trailings2), 1);

  JPetSigCh trail5(JPetSigCh::Trailing, 2.0);
  JPetSigCh trail6(JPetSigCh::Trailing, 4.0);
  trail5.setPM(pm1);
  trail6.setPM(pm1);
  std::vector<JPetSigCh> trailings3;
  trailings3.push_back(trail5);
  trailings3.push_back(trail6);
  BOOST_REQUIRE_EQUAL(
      SignalFinderTools::findTrailingSigCh(lead, 10.0, trailings3), 0);
}

BOOST_AUTO_TEST_CASE(buildRawSignals_realdata_flag_test) {
  JPetBarrelSlot bs1(1, true, "some_slot", 57.7, 123);
  JPetPM pm1(1, "first");
  pm1.setBarrelSlot(bs1);

  JPetSigCh sigCh01(JPetSigCh::Leading, -1.73608);
  sigCh01.setPM(pm1);
  sigCh01.setThresholdNumber(1);
  sigCh01.setRecoFlag(JPetSigCh::Good);

  JPetSigCh sigCh02(JPetSigCh::Trailing, -1.73553);
  sigCh02.setPM(pm1);
  sigCh02.setThresholdNumber(1);
  sigCh02.setRecoFlag(JPetSigCh::Good);

  JPetSigCh sigCh03(JPetSigCh::Leading, -1.62030);
  sigCh03.setPM(pm1);
  sigCh03.setThresholdNumber(1);
  sigCh03.setRecoFlag(JPetSigCh::Good);

  JPetSigCh sigCh04(JPetSigCh::Trailing, -1.62026);
  sigCh04.setPM(pm1);
  sigCh04.setThresholdNumber(1);
  sigCh04.setRecoFlag(JPetSigCh::Good);

  JPetSigCh sigCh05(JPetSigCh::Leading, -1.37056);
  sigCh05.setPM(pm1);
  sigCh05.setThresholdNumber(1);
  sigCh05.setRecoFlag(JPetSigCh::Good);

  JPetSigCh sigCh06(JPetSigCh::Trailing, -1.37000);
  sigCh06.setPM(pm1);
  sigCh06.setThresholdNumber(1);
  sigCh06.setRecoFlag(JPetSigCh::Good);

  JPetSigCh sigCh07(JPetSigCh::Leading, -1.73605);
  sigCh07.setPM(pm1);
  sigCh07.setThresholdNumber(2);
  sigCh07.setRecoFlag(JPetSigCh::Good);

  JPetSigCh sigCh08(JPetSigCh::Trailing, -1.73569);
  sigCh08.setPM(pm1);
  sigCh08.setThresholdNumber(2);
  sigCh08.setRecoFlag(JPetSigCh::Corrupted);

  JPetSigCh sigCh09(JPetSigCh::Trailing, -1.62032);
  sigCh09.setPM(pm1);
  sigCh09.setThresholdNumber(2);
  sigCh09.setRecoFlag(JPetSigCh::Corrupted);

  JPetSigCh sigCh10(JPetSigCh::Leading, -1.62029);
  sigCh10.setPM(pm1);
  sigCh10.setThresholdNumber(2);
  sigCh10.setRecoFlag(JPetSigCh::Corrupted);

  JPetSigCh sigCh11(JPetSigCh::Leading, -1.37054);
  sigCh11.setPM(pm1);
  sigCh11.setThresholdNumber(2);
  sigCh11.setRecoFlag(JPetSigCh::Corrupted);

  JPetSigCh sigCh12(JPetSigCh::Trailing, -1.37006);
  sigCh12.setPM(pm1);
  sigCh12.setThresholdNumber(2);
  sigCh12.setRecoFlag(JPetSigCh::Corrupted);

  JPetSigCh sigCh13(JPetSigCh::Leading, -1.73601);
  sigCh13.setPM(pm1);
  sigCh13.setThresholdNumber(3);
  sigCh13.setRecoFlag(JPetSigCh::Good);

  JPetSigCh sigCh14(JPetSigCh::Trailing, -1.73572);
  sigCh14.setPM(pm1);
  sigCh14.setThresholdNumber(3);
  sigCh14.setRecoFlag(JPetSigCh::Good);

  JPetSigCh sigCh15(JPetSigCh::Leading, -1.37053);
  sigCh15.setPM(pm1);
  sigCh15.setThresholdNumber(3);
  sigCh15.setRecoFlag(JPetSigCh::Good);

  JPetSigCh sigCh16(JPetSigCh::Trailing, -1.37013);
  sigCh16.setPM(pm1);
  sigCh16.setThresholdNumber(3);
  sigCh16.setRecoFlag(JPetSigCh::Good);

  JPetSigCh sigCh17(JPetSigCh::Leading, -1.73598);
  sigCh17.setPM(pm1);
  sigCh17.setThresholdNumber(4);
  sigCh17.setRecoFlag(JPetSigCh::Good);

  JPetSigCh sigCh18(JPetSigCh::Trailing, -1.73581);
  sigCh18.setPM(pm1);
  sigCh18.setThresholdNumber(4);
  sigCh18.setRecoFlag(JPetSigCh::Good);

  JPetSigCh sigCh19(JPetSigCh::Leading, -1.37051);
  sigCh19.setPM(pm1);
  sigCh19.setThresholdNumber(4);
  sigCh19.setRecoFlag(JPetSigCh::Good);

  JPetSigCh sigCh20(JPetSigCh::Trailing, -1.37028);
  sigCh20.setPM(pm1);
  sigCh20.setThresholdNumber(4);
  sigCh20.setRecoFlag(JPetSigCh::Good);

  std::vector<JPetSigCh> sigChFromSamePM;
  sigChFromSamePM.push_back(sigCh01);
  sigChFromSamePM.push_back(sigCh02);
  sigChFromSamePM.push_back(sigCh03);
  sigChFromSamePM.push_back(sigCh04);
  sigChFromSamePM.push_back(sigCh05);
  sigChFromSamePM.push_back(sigCh06);
  sigChFromSamePM.push_back(sigCh07);
  sigChFromSamePM.push_back(sigCh08);
  sigChFromSamePM.push_back(sigCh09);
  sigChFromSamePM.push_back(sigCh10);
  sigChFromSamePM.push_back(sigCh11);
  sigChFromSamePM.push_back(sigCh12);
  sigChFromSamePM.push_back(sigCh13);
  sigChFromSamePM.push_back(sigCh14);
  sigChFromSamePM.push_back(sigCh15);
  sigChFromSamePM.push_back(sigCh16);
  sigChFromSamePM.push_back(sigCh17);
  sigChFromSamePM.push_back(sigCh18);
  sigChFromSamePM.push_back(sigCh19);
  sigChFromSamePM.push_back(sigCh20);

  double sigChEdgeMaxTime = 0.0005;
  double sigChLeadTrailMaxTime = 0.0023;
  JPetStatistics stats;
  auto results = SignalFinderTools::buildRawSignals(
    sigChFromSamePM, sigChEdgeMaxTime, sigChLeadTrailMaxTime, stats, false
  );
  BOOST_REQUIRE_EQUAL(results.size(), 3);
  BOOST_REQUIRE_EQUAL(results.at(0).getRecoFlag(), JPetBaseSignal::Corrupted);
  BOOST_REQUIRE_EQUAL(results.at(1).getRecoFlag(), JPetBaseSignal::Corrupted);
  BOOST_REQUIRE_EQUAL(results.at(2).getRecoFlag(), JPetBaseSignal::Corrupted);
}

BOOST_AUTO_TEST_CASE(reorderThresholdsByValue){

  SignalFinderTools::Permutation new_order = SignalFinderTools::kIdentity;
  SignalFinderTools::ThresholdValues values = {100., 70., 20., 90.};
  SignalFinderTools::permuteThresholdsByValue(values, new_order);
  BOOST_REQUIRE_EQUAL(new_order[0], 3);
  BOOST_REQUIRE_EQUAL(new_order[1], 1);
  BOOST_REQUIRE_EQUAL(new_order[2], 0);
  BOOST_REQUIRE_EQUAL(new_order[3], 2);

  SignalFinderTools::ThresholdValues sorted_values;
  for(int i=0;i<4;++i){
    sorted_values[new_order[i]] = values[i];
  }

  BOOST_REQUIRE_LE(sorted_values[0], sorted_values[1]);
  BOOST_REQUIRE_LE(sorted_values[1], sorted_values[2]);
  BOOST_REQUIRE_LE(sorted_values[2], sorted_values[3]);
}

BOOST_AUTO_TEST_CASE(findThresholdOrders){
  JPetParamBank bank;
  JPetPM pm1(JPetPM::SideA, 221, 32, 64, std::make_pair(16.f, 32.f), "test_pm1");
  JPetPM pm2(JPetPM::SideB, 222, 32, 64, std::make_pair(16.f, 32.f), "test_pm2");

  bank.addPM(pm1);
  bank.addPM(pm2);

  SignalFinderTools::ThresholdValues values_pm1 = {40., 3., 20., 10.};
  SignalFinderTools::ThresholdValues values_pm2 = {100., 400., 200., 300.};

  JPetTOMBChannel channels[8];

  for(int i=0;i<4;++i){

    channels[i] = JPetTOMBChannel(i);
    channels[4+i] = JPetTOMBChannel(4+i);

    channels[i].setLocalChannelNumber(i+1);
    channels[4+i].setLocalChannelNumber(i+1);

    channels[i].setThreshold(values_pm1[i]);
    channels[4+i].setThreshold(values_pm2[i]);

    channels[i].setPM(pm1);
    channels[4+i].setPM(pm2);

    bank.addTOMBChannel(channels[i]);
    bank.addTOMBChannel(channels[4+i]);
  }

  SignalFinderTools::ThresholdOrderings orderings =  SignalFinderTools::findThresholdOrders(bank);
  BOOST_REQUIRE_EQUAL(orderings.size(), 2);

  for(auto& pm: orderings){
    SignalFinderTools::ThresholdValues sorted_values;
    SignalFinderTools::ThresholdValues& orig_values = (pm.first==221 ? values_pm1 : values_pm2);
    for(int i=0;i<4;++i){
      sorted_values[pm.second[i]] = orig_values[i];
    }

    BOOST_REQUIRE_LE(sorted_values[0], sorted_values[1]);
    BOOST_REQUIRE_LE(sorted_values[1], sorted_values[2]);
    BOOST_REQUIRE_LE(sorted_values[2], sorted_values[3]);
  }
}

BOOST_AUTO_TEST_SUITE_END()
