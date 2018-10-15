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
  JPetParamBank paramBank;
  JPetStatistics stats;
  auto results = SignalFinderTools::getSigChByPM(nullptr, paramBank, false);
  BOOST_REQUIRE(results.empty());
}

BOOST_AUTO_TEST_CASE(getSigChByPM_Test)
{
  JPetParamBank paramBank;
  JPetPM pm1(1, "first");
  paramBank.addPM(pm1);
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
  paramBank.addPM(pm2);
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
  paramBank.addPM(pm3);
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
  JPetStatistics stats;

  auto results1 = SignalFinderTools::getSigChByPM(&slot, paramBank, false);
  auto results2 = SignalFinderTools::getSigChByPM(&slot, paramBank, true);

  BOOST_REQUIRE_EQUAL(results1.size(), 3);
  BOOST_REQUIRE_EQUAL(results2.size(), 3);
  BOOST_REQUIRE_EQUAL(results1.at(0).size(), 3);
  BOOST_REQUIRE_EQUAL(results1.at(1).size(), 0);
  BOOST_REQUIRE_EQUAL(results1.at(2).size(), 2);
  BOOST_REQUIRE_EQUAL(results2.at(0).size(), 3);
  BOOST_REQUIRE_EQUAL(results2.at(1).size(), 3);
  BOOST_REQUIRE_EQUAL(results2.at(2).size(), 2);
}

// BOOST_AUTO_TEST_CASE(buildRawSignals_empty)
// {
//   JPetStatistics stats;
//   std::vector<JPetSigCh> sigChFromSamePM;
//   auto results =  SignalFinderTools::buildRawSignals(sigChFromSamePM, 4, 5.0, 5.0, stats, false);
//   BOOST_REQUIRE(results.empty());
// }

// BOOST_AUTO_TEST_CASE(buildRawSignals_no4thr)
// {
//   JPetStatistics stats;
//   std::vector<JPetSigCh> sigChFromSamePM;
//   JPetSigCh sigCh1(JPetSigCh::Leading, 5.0);
//   JPetSigCh sigCh2(JPetSigCh::Leading, 10.0);
//   sigChFromSamePM.push_back(sigCh1);
//   sigChFromSamePM.push_back(sigCh2);
//   auto results =  SignalFinderTools::buildRawSignals(sigChFromSamePM, 2, 5.0, 5.0, stats, false);
//   BOOST_REQUIRE(results.empty());
// }

// BOOST_AUTO_TEST_CASE(getSigChsByEdge_test)
// {
//   JPetSigCh sigCh1(JPetSigCh::Leading, 5.0);
//   JPetSigCh sigCh2(JPetSigCh::Trailing, 6.0);
//   JPetSigCh sigCh3(JPetSigCh::Leading, 5.0);
//   JPetSigCh sigCh4(JPetSigCh::Trailing, 6.0);
//   JPetSigCh sigCh5(JPetSigCh::Trailing, 7.0);
//   JPetSigCh sigCh6(JPetSigCh::Trailing, 8.0);
//   JPetSigCh sigCh7(JPetSigCh::Trailing, 9.0);
//   JPetSigCh sigCh8(JPetSigCh::Leading, 10.0);
//
//   std::vector<JPetSigCh> sigChFromSamePM;
//   sigChFromSamePM.push_back(sigCh1);
//   sigChFromSamePM.push_back(sigCh2);
//   sigChFromSamePM.push_back(sigCh3);
//   sigChFromSamePM.push_back(sigCh4);
//   sigChFromSamePM.push_back(sigCh5);
//   sigChFromSamePM.push_back(sigCh6);
//   sigChFromSamePM.push_back(sigCh7);
//   sigChFromSamePM.push_back(sigCh8);
//   auto result1 = SignalFinderTools::getSigChsByEdge(sigChFromSamePM, JPetSigCh::Leading);
//   auto result2 = SignalFinderTools::getSigChsByEdge(sigChFromSamePM, JPetSigCh::Trailing);
//
//   BOOST_REQUIRE_EQUAL(result1.size(), 3);
//   BOOST_REQUIRE_EQUAL(result2.size(), 5);
// }

// BOOST_AUTO_TEST_CASE(validateSignal_test)
// {
//   JPetSigCh sigChA1(JPetSigCh::Leading, 1.0);
//   JPetSigCh sigChA2(JPetSigCh::Leading, 2.0);
//   JPetSigCh sigChA3(JPetSigCh::Trailing, 3.0);
//   JPetSigCh sigChA4(JPetSigCh::Trailing, 4.0);
//   sigChA1.setThresholdNumber(1);
//   sigChA2.setThresholdNumber(2);
//   sigChA3.setThresholdNumber(2);
//   sigChA4.setThresholdNumber(1);
//   sigChA1.setRecoFlag(JPetSigCh::Good);
//   sigChA2.setRecoFlag(JPetSigCh::Good);
//   sigChA3.setRecoFlag(JPetSigCh::Good);
//   sigChA4.setRecoFlag(JPetSigCh::Good);
//   JPetRawSignal rawSignalA;
//   rawSignalA.addPoint(sigChA1);
//   rawSignalA.addPoint(sigChA2);
//   rawSignalA.addPoint(sigChA3);
//   rawSignalA.addPoint(sigChA4);
//   BOOST_REQUIRE(SignalFinderTools::validateSignal(rawSignalA, 4));
//
//   JPetSigCh sigChB1(JPetSigCh::Leading, 1.0);
//   JPetSigCh sigChB2(JPetSigCh::Leading, 2.0);
//   JPetSigCh sigChB3(JPetSigCh::Leading, 3.0);
//   JPetSigCh sigChB4(JPetSigCh::Leading, 4.0);
//   JPetSigCh sigChB5(JPetSigCh::Trailing, 5.0);
//   JPetSigCh sigChB6(JPetSigCh::Trailing, 6.0);
//   JPetSigCh sigChB7(JPetSigCh::Trailing, 7.0);
//   sigChB1.setThresholdNumber(1);
//   sigChB2.setThresholdNumber(2);
//   sigChB3.setThresholdNumber(3);
//   sigChB4.setThresholdNumber(4);
//   sigChB5.setThresholdNumber(3);
//   sigChB6.setThresholdNumber(2);
//   sigChB7.setThresholdNumber(1);
//   sigChB1.setRecoFlag(JPetSigCh::Good);
//   sigChB2.setRecoFlag(JPetSigCh::Good);
//   sigChB3.setRecoFlag(JPetSigCh::Good);
//   sigChB4.setRecoFlag(JPetSigCh::Good);
//   sigChB5.setRecoFlag(JPetSigCh::Good);
//   sigChB6.setRecoFlag(JPetSigCh::Good);
//   sigChB7.setRecoFlag(JPetSigCh::Good);
//   JPetRawSignal rawSignalB;
//   rawSignalB.addPoint(sigChB1);
//   rawSignalB.addPoint(sigChB2);
//   rawSignalB.addPoint(sigChB3);
//   rawSignalB.addPoint(sigChB4);
//   rawSignalB.addPoint(sigChB5);
//   rawSignalB.addPoint(sigChB6);
//   rawSignalB.addPoint(sigChB7);
//   BOOST_REQUIRE(!SignalFinderTools::validateSignal(rawSignalB, 4));
//
//   JPetSigCh sigChC1(JPetSigCh::Leading, 1.0);
//   JPetSigCh sigChC2(JPetSigCh::Leading, 2.0);
//   JPetSigCh sigChC3(JPetSigCh::Trailing, 3.0);
//   JPetSigCh sigChC4(JPetSigCh::Trailing, 4.0);
//   sigChC1.setThresholdNumber(1);
//   sigChC2.setThresholdNumber(2);
//   sigChC3.setThresholdNumber(2);
//   sigChC4.setThresholdNumber(1);
//   sigChC1.setRecoFlag(JPetSigCh::Good);
//   sigChC2.setRecoFlag(JPetSigCh::Good);
//   sigChC3.setRecoFlag(JPetSigCh::Corrupted);
//   sigChC4.setRecoFlag(JPetSigCh::Good);
//   JPetRawSignal rawSignalC;
//   rawSignalC.addPoint(sigChC1);
//   rawSignalC.addPoint(sigChC2);
//   rawSignalC.addPoint(sigChC3);
//   rawSignalC.addPoint(sigChC4);
//   BOOST_REQUIRE(!SignalFinderTools::validateSignal(rawSignalC, 4));
// }

// BOOST_AUTO_TEST_CASE(buildRawSignals_test_leads)
// {
//   JPetSigCh sigCh1(JPetSigCh::Leading, 1.0);
//   JPetSigCh sigCh2(JPetSigCh::Leading, 3.0);
//   JPetSigCh sigCh3(JPetSigCh::Leading, 4.0);
//   JPetSigCh sigCh4(JPetSigCh::Leading, 5.5);
//   JPetSigCh sigCh5(JPetSigCh::Leading, 101.0);
//   JPetSigCh sigCh6(JPetSigCh::Leading, 102.0);
//   JPetSigCh sigCh7(JPetSigCh::Leading, 103.0);
//   JPetSigCh sigCh8(JPetSigCh::Leading, 150.0);
//
//   sigCh1.setThresholdNumber(1);
//   sigCh2.setThresholdNumber(2);
//   sigCh3.setThresholdNumber(3);
//   sigCh4.setThresholdNumber(4);
//   sigCh5.setThresholdNumber(1);
//   sigCh6.setThresholdNumber(2);
//   sigCh7.setThresholdNumber(3);
//   sigCh8.setThresholdNumber(4);
//
//   std::vector<JPetSigCh> vec;
//   vec.push_back(sigCh1);
//   vec.push_back(sigCh2);
//   vec.push_back(sigCh3);
//   vec.push_back(sigCh4);
//   vec.push_back(sigCh5);
//   vec.push_back(sigCh6);
//   vec.push_back(sigCh7);
//   vec.push_back(sigCh8);
//
//   JPetStatistics stats;
//   auto results = SignalFinderTools::buildRawSignals(vec, 4, 5.0, 8.0, stats, false);
//   BOOST_REQUIRE_EQUAL(results.size(), 3);
//   BOOST_REQUIRE_EQUAL(results.at(0).getPoints(JPetSigCh::Leading).size(), 4);
//   BOOST_REQUIRE_EQUAL(results.at(1).getPoints(JPetSigCh::Leading).size(), 3);
//   BOOST_REQUIRE_EQUAL(results.at(2).getPoints(JPetSigCh::Leading).size(), 1);
// }

// BOOST_AUTO_TEST_CASE(buildRawSignals_test_trails)
// {
//   JPetSigCh sigChA1(JPetSigCh::Leading, 1.0);
//   JPetSigCh sigChA2(JPetSigCh::Leading, 2.0);
//   JPetSigCh sigChA3(JPetSigCh::Trailing, 8.0);
//   JPetSigCh sigChA4(JPetSigCh::Trailing, 9.0);
//   sigChA1.setThresholdNumber(1);
//   sigChA2.setThresholdNumber(2);
//   sigChA3.setThresholdNumber(2);
//   sigChA4.setThresholdNumber(1);
//
//   JPetSigCh sigChB1(JPetSigCh::Leading, 11.0);
//   JPetSigCh sigChB2(JPetSigCh::Leading, 12.0);
//   JPetSigCh sigChB3(JPetSigCh::Leading, 13.0);
//   JPetSigCh sigChB4(JPetSigCh::Leading, 14.0);
//   JPetSigCh sigChB5(JPetSigCh::Trailing, 18.0);
//   JPetSigCh sigChB6(JPetSigCh::Trailing, 19.0);
//   JPetSigCh sigChB7(JPetSigCh::Trailing, 20.0);
//   JPetSigCh sigChB8(JPetSigCh::Trailing, 29.0);
//   sigChB1.setThresholdNumber(1);
//   sigChB2.setThresholdNumber(2);
//   sigChB3.setThresholdNumber(3);
//   sigChB4.setThresholdNumber(4);
//   sigChB5.setThresholdNumber(4);
//   sigChB6.setThresholdNumber(3);
//   sigChB7.setThresholdNumber(2);
//   sigChB8.setThresholdNumber(1);
//
//   JPetSigCh sigChC1(JPetSigCh::Leading, 41.0);
//   JPetSigCh sigChC2(JPetSigCh::Leading, 42.0);
//   JPetSigCh sigChC3(JPetSigCh::Leading, 43.0);
//   JPetSigCh sigChC4(JPetSigCh::Trailing, 46.0);
//   JPetSigCh sigChC5(JPetSigCh::Trailing, 65.0);
//   JPetSigCh sigChC6(JPetSigCh::Trailing, 67.0);
//   sigChC1.setThresholdNumber(1);
//   sigChC2.setThresholdNumber(2);
//   sigChC3.setThresholdNumber(3);
//   sigChC4.setThresholdNumber(3);
//   sigChC5.setThresholdNumber(2);
//   sigChC6.setThresholdNumber(1);
//
//   std::vector<JPetSigCh> vec;
//   vec.push_back(sigChA1);
//   vec.push_back(sigChA2);
//   vec.push_back(sigChA3);
//   vec.push_back(sigChA4);
//   vec.push_back(sigChB1);
//   vec.push_back(sigChB2);
//   vec.push_back(sigChB3);
//   vec.push_back(sigChB4);
//   vec.push_back(sigChB5);
//   vec.push_back(sigChB6);
//   vec.push_back(sigChB7);
//   vec.push_back(sigChB8);
//   vec.push_back(sigChC1);
//   vec.push_back(sigChC2);
//   vec.push_back(sigChC3);
//   vec.push_back(sigChC4);
//   vec.push_back(sigChC5);
//   vec.push_back(sigChC6);
//
//   JPetStatistics stats;
//   auto results = SignalFinderTools::buildRawSignals(vec, 4, 5.0, 12.0, stats, false);
//   BOOST_REQUIRE_EQUAL(results.size(), 3);
//   BOOST_REQUIRE_EQUAL(results.at(0).getPoints(JPetSigCh::Leading).size(), 2);
//   BOOST_REQUIRE_EQUAL(results.at(0).getPoints(JPetSigCh::Trailing).size(), 2);
//   BOOST_REQUIRE_EQUAL(results.at(1).getPoints(JPetSigCh::Leading).size(), 4);
//   BOOST_REQUIRE_EQUAL(results.at(1).getPoints(JPetSigCh::Trailing).size(), 3);
//   BOOST_REQUIRE_EQUAL(results.at(2).getPoints(JPetSigCh::Leading).size(), 3);
//   BOOST_REQUIRE_EQUAL(results.at(2).getPoints(JPetSigCh::Trailing).size(), 1);
// }

BOOST_AUTO_TEST_SUITE_END()
