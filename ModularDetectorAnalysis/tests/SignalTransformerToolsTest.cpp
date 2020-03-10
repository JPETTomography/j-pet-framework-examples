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
 *  @file SignalTransformerToolsTest.cpp
 */

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE SignalTransformerToolsTest

#include "../SignalTransformerTools.h"
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(SignalTransformerToolsTestSuite)

BOOST_AUTO_TEST_CASE(getRawSigMtxMap_test_empty)
{
  JPetStatistics stats;
  auto results = SignalTransformerTools::getRawSigMtxMap(nullptr);
  BOOST_REQUIRE(results.empty());
}

BOOST_AUTO_TEST_CASE(getRawSigMtxMap_test)
{
  JPetScin scin1(1, 50.0, 19.0, 3.0, -1.0, 0.0, 0.0);

  JPetPM pm1A1(1, JPetPM::SideA, "1A1", 1);
  JPetPM pm1B1(2, JPetPM::SideB, "1B1", 1);
  pm1A1.setScin(scin1);
  pm1B1.setScin(scin1);

  JPetRawSignal sig11, sig12, sig13, sig14, sig15;
  sig11.setPM(pm1A1);
  sig12.setPM(pm1A1);
  sig13.setPM(pm1B1);
  sig14.setPM(pm1B1);
  sig15.setPM(pm1B1);

  JPetScin scin2(2, 50.0, 19.0, 3.0, -0.0, 0.0, 0.0);

  JPetPM pm2A1(3, JPetPM::SideA, "2A1", 1);
  JPetPM pm2B1(4, JPetPM::SideB, "2B1", 1);
  JPetPM pm2A2(5, JPetPM::SideA, "2A2", 2);
  JPetPM pm2B2(6, JPetPM::SideB, "2B2", 2);
  pm2A1.setScin(scin2);
  pm2B1.setScin(scin2);
  pm2A2.setScin(scin2);
  pm2B2.setScin(scin2);

  JPetRawSignal sig21, sig22, sig23, sig24, sig25, sig26, sig27;
  sig21.setPM(pm2A1);
  sig22.setPM(pm2A1);
  sig23.setPM(pm2B1);
  sig24.setPM(pm2B1);
  sig25.setPM(pm2B1);
  sig26.setPM(pm2A2);
  sig27.setPM(pm2A2);

  JPetScin scin3(3, 50.0, 19.0, 3.0, 1.0, 0.0, 0.0);

  JPetPM pm3A1(7, JPetPM::SideA, "3A1", 1);
  JPetPM pm3B1(8, JPetPM::SideB, "3B1", 1);
  JPetPM pm3A2(9, JPetPM::SideA, "3A2", 2);
  JPetPM pm3B2(10, JPetPM::SideB, "3B2", 2);
  JPetPM pm3A3(11, JPetPM::SideA, "3A3", 3);
  JPetPM pm3B3(12, JPetPM::SideB, "3B3", 3);
  pm3A1.setScin(scin3);
  pm3B1.setScin(scin3);
  pm3A2.setScin(scin3);
  pm3B2.setScin(scin3);
  pm3A3.setScin(scin3);
  pm3B3.setScin(scin3);

  JPetRawSignal sig31, sig32, sig33, sig34, sig35, sig36, sig37, sig38, sig39;
  sig31.setPM(pm3A1);
  sig32.setPM(pm3A1);
  sig33.setPM(pm3A1);
  sig34.setPM(pm3B1);
  sig35.setPM(pm3B1);
  sig36.setPM(pm3B2);
  sig37.setPM(pm3A3);
  sig38.setPM(pm3B3);
  sig39.setPM(pm3B3);

  JPetTimeWindow slot("JPetRawSignal");
  slot.add<JPetRawSignal>(sig11);
  slot.add<JPetRawSignal>(sig12);
  slot.add<JPetRawSignal>(sig13);
  slot.add<JPetRawSignal>(sig14);
  slot.add<JPetRawSignal>(sig15);
  slot.add<JPetRawSignal>(sig21);
  slot.add<JPetRawSignal>(sig22);
  slot.add<JPetRawSignal>(sig23);
  slot.add<JPetRawSignal>(sig24);
  slot.add<JPetRawSignal>(sig25);
  slot.add<JPetRawSignal>(sig26);
  slot.add<JPetRawSignal>(sig27);
  slot.add<JPetRawSignal>(sig31);
  slot.add<JPetRawSignal>(sig32);
  slot.add<JPetRawSignal>(sig33);
  slot.add<JPetRawSignal>(sig34);
  slot.add<JPetRawSignal>(sig35);
  slot.add<JPetRawSignal>(sig36);
  slot.add<JPetRawSignal>(sig37);
  slot.add<JPetRawSignal>(sig38);
  slot.add<JPetRawSignal>(sig39);

  JPetStatistics stats;
  auto results = SignalTransformerTools::getRawSigMtxMap(&slot);

  BOOST_REQUIRE_EQUAL(results.size(), 3);

  BOOST_REQUIRE_EQUAL(results[1].size(), 2);
  BOOST_REQUIRE_EQUAL(results[1].at(0).size(), 2);
  BOOST_REQUIRE_EQUAL(results[1].at(1).size(), 3);

  BOOST_REQUIRE_EQUAL(results[2].size(), 2);
  BOOST_REQUIRE_EQUAL(results[2].at(0).size(), 4);
  BOOST_REQUIRE_EQUAL(results[2].at(1).size(), 3);

  BOOST_REQUIRE_EQUAL(results[3].size(), 2);
  BOOST_REQUIRE_EQUAL(results[3].at(0).size(), 4);
  BOOST_REQUIRE_EQUAL(results[3].at(1).size(), 5);
}

BOOST_AUTO_TEST_CASE(mergeRawSignalsOnSide_test)
{
  JPetScin scin1(1, 50.0, 19.0, 3.0, -1.0, 0.0, 0.0);
  JPetScin scin2(2, 50.0, 19.0, 3.0, 0.0, 0.0, 0.0);
  JPetScin scin3(3, 50.0, 19.0, 3.0, 1.0, 0.0, 0.0);
  JPetScin scin4(4, 50.0, 19.0, 3.0, 2.0, 0.0, 0.0);

  JPetPM pm1(1, JPetPM::SideA, "1", 1);
  JPetPM pm2(2, JPetPM::SideA, "2", 2);
  JPetPM pm3(3, JPetPM::SideA, "3", 3);
  JPetPM pm4(4, JPetPM::SideA, "4", 4);

  JPetChannel channel1(1, 1, 50.0);
  JPetChannel channel2(2, 1, 50.0);
  JPetChannel channel3(3, 1, 50.0);
  JPetChannel channel4(4, 1, 50.0);
  channel1.setPM(pm1);
  channel2.setPM(pm2);
  channel3.setPM(pm3);
  channel4.setPM(pm4);

  JPetSigCh sigChA1(JPetSigCh::Leading, 10.0);
  JPetSigCh sigChA2(JPetSigCh::Leading, 11.0);
  JPetSigCh sigChA3(JPetSigCh::Leading, 12.0);
  JPetSigCh sigChA4(JPetSigCh::Leading, 13.0);

  JPetSigCh sigChB1(JPetSigCh::Leading, 20.0);
  JPetSigCh sigChB2(JPetSigCh::Leading, 21.0);

  JPetSigCh sigChC1(JPetSigCh::Leading, 30.0);
  JPetSigCh sigChC2(JPetSigCh::Leading, 31.0);
  JPetSigCh sigChC3(JPetSigCh::Leading, 32.0);

  JPetSigCh sigChD1(JPetSigCh::Leading, 31.5);

  JPetSigCh sigChE1(JPetSigCh::Leading, 40.5);
  JPetSigCh sigChE2(JPetSigCh::Leading, 40.9);
  JPetSigCh sigChE3(JPetSigCh::Leading, 41.2);
  JPetSigCh sigChE4(JPetSigCh::Leading, 41.8);
  JPetSigCh sigChE5(JPetSigCh::Leading, 42.5);
  JPetSigCh sigChE6(JPetSigCh::Leading, 42.9);

  sigChA1.setChannel(channel1);
  sigChA2.setChannel(channel2);
  sigChA3.setChannel(channel3);
  sigChA4.setChannel(channel4);

  sigChB1.setChannel(channel2);
  sigChB2.setChannel(channel3);

  sigChC1.setChannel(channel4);
  sigChC2.setChannel(channel2);
  sigChC3.setChannel(channel3);

  sigChD1.setChannel(channel2);

  sigChE1.setChannel(channel1);
  sigChE2.setChannel(channel2);
  sigChE3.setChannel(channel3);
  sigChE4.setChannel(channel4);
  sigChE5.setChannel(channel2);
  sigChE6.setChannel(channel3);

  JPetRawSignal sigA1, sigA2, sigA3, sigA4;
  JPetRawSignal sigB1, sigB2;
  JPetRawSignal sigC1, sigC2, sigC3;
  JPetRawSignal sigD1;
  JPetRawSignal sigE1, sigE2, sigE3, sigE4, sigE5, sigE6;

  sigA1.setPM(sigChA1.getChannel().getPM());
  sigA1.addPoint(sigChA1);
  sigA2.setPM(sigChA2.getChannel().getPM());
  sigA2.addPoint(sigChA2);
  sigA3.setPM(sigChA3.getChannel().getPM());
  sigA3.addPoint(sigChA3);
  sigA4.setPM(sigChA4.getChannel().getPM());
  sigA4.addPoint(sigChA4);

  sigB1.setPM(sigChB1.getChannel().getPM());
  sigB1.addPoint(sigChB1);
  sigB2.setPM(sigChB2.getChannel().getPM());
  sigB2.addPoint(sigChB2);

  sigC1.setPM(sigChC1.getChannel().getPM());
  sigC1.addPoint(sigChC1);
  sigC2.setPM(sigChC2.getChannel().getPM());
  sigC2.addPoint(sigChC2);
  sigC3.setPM(sigChC3.getChannel().getPM());
  sigC3.addPoint(sigChC3);

  sigD1.setPM(sigChD1.getChannel().getPM());
  sigD1.addPoint(sigChD1);

  sigE1.setPM(sigChE1.getChannel().getPM());
  sigE1.addPoint(sigChE1);
  sigE2.setPM(sigChE2.getChannel().getPM());
  sigE2.addPoint(sigChE2);
  sigE3.setPM(sigChE3.getChannel().getPM());
  sigE3.addPoint(sigChE3);
  sigE4.setPM(sigChE4.getChannel().getPM());
  sigE4.addPoint(sigChE4);
  sigE5.setPM(sigChE5.getChannel().getPM());
  sigE5.addPoint(sigChE5);
  sigE6.setPM(sigChE6.getChannel().getPM());
  sigE6.addPoint(sigChE6);

  std::vector<JPetRawSignal> rawSigVec;
  rawSigVec.push_back(sigA1);
  rawSigVec.push_back(sigA2);
  rawSigVec.push_back(sigA3);
  rawSigVec.push_back(sigA4);

  rawSigVec.push_back(sigB1);
  rawSigVec.push_back(sigB2);

  rawSigVec.push_back(sigC1);
  rawSigVec.push_back(sigC2);
  rawSigVec.push_back(sigC3);

  rawSigVec.push_back(sigD1);

  rawSigVec.push_back(sigE1);
  rawSigVec.push_back(sigE2);
  rawSigVec.push_back(sigE3);
  rawSigVec.push_back(sigE4);
  rawSigVec.push_back(sigE5);
  rawSigVec.push_back(sigE6);


  JPetStatistics stats;
  auto result = SignalTransformerTools::mergeRawSignalsOnSide(
    rawSigVec, 5.0, stats, false
  );

  double epsilon = 0.00001;
  BOOST_REQUIRE_EQUAL(result.size(), 6);
  BOOST_REQUIRE_EQUAL(result.at(0).getRawSignals().size(), 4);
  BOOST_REQUIRE_EQUAL(result.at(1).getRawSignals().size(), 2);
  BOOST_REQUIRE_EQUAL(result.at(2).getRawSignals().size(), 3);
  BOOST_REQUIRE_EQUAL(result.at(3).getRawSignals().size(), 1);
  BOOST_REQUIRE_EQUAL(result.at(4).getRawSignals().size(), 4);
  BOOST_REQUIRE_EQUAL(result.at(5).getRawSignals().size(), 2);

  auto timeA1 = SignalTransformerTools::getRawSigBaseTime(result.at(0).getRawSignals().at(1));
  auto timeA2 = SignalTransformerTools::getRawSigBaseTime(result.at(0).getRawSignals().at(2));
  auto timeA3 = SignalTransformerTools::getRawSigBaseTime(result.at(0).getRawSignals().at(3));
  auto timeA4 = SignalTransformerTools::getRawSigBaseTime(result.at(0).getRawSignals().at(4));

  auto timeB1 = SignalTransformerTools::getRawSigBaseTime(result.at(1).getRawSignals().at(2));
  auto timeB2 = SignalTransformerTools::getRawSigBaseTime(result.at(1).getRawSignals().at(3));

  auto timeC1 = SignalTransformerTools::getRawSigBaseTime(result.at(2).getRawSignals().at(4));
  auto timeC2 = SignalTransformerTools::getRawSigBaseTime(result.at(2).getRawSignals().at(2));
  auto timeC3 = SignalTransformerTools::getRawSigBaseTime(result.at(2).getRawSignals().at(3));

  auto timeD1 = SignalTransformerTools::getRawSigBaseTime(result.at(3).getRawSignals().at(2));

  auto timeE1 = SignalTransformerTools::getRawSigBaseTime(result.at(4).getRawSignals().at(1));
  auto timeE2 = SignalTransformerTools::getRawSigBaseTime(result.at(4).getRawSignals().at(2));
  auto timeE3 = SignalTransformerTools::getRawSigBaseTime(result.at(4).getRawSignals().at(3));
  auto timeE4 = SignalTransformerTools::getRawSigBaseTime(result.at(4).getRawSignals().at(4));
  auto timeE5 = SignalTransformerTools::getRawSigBaseTime(result.at(5).getRawSignals().at(2));
  auto timeE6 = SignalTransformerTools::getRawSigBaseTime(result.at(5).getRawSignals().at(3));

  BOOST_REQUIRE_CLOSE(timeA1, 10.0, epsilon);
  BOOST_REQUIRE_CLOSE(timeA2, 11.0, epsilon);
  BOOST_REQUIRE_CLOSE(timeA3, 12.0, epsilon);
  BOOST_REQUIRE_CLOSE(timeA4, 13.0, epsilon);

  BOOST_REQUIRE_CLOSE(timeB1, 20.0, epsilon);
  BOOST_REQUIRE_CLOSE(timeB2, 21.0, epsilon);

  BOOST_REQUIRE_CLOSE(timeC1, 30.0, epsilon);
  BOOST_REQUIRE_CLOSE(timeC2, 31.0, epsilon);
  BOOST_REQUIRE_CLOSE(timeC3, 32.0, epsilon);

  BOOST_REQUIRE_CLOSE(timeD1, 31.5, epsilon);

  BOOST_REQUIRE_CLOSE(timeE1, 40.5, epsilon);
  BOOST_REQUIRE_CLOSE(timeE2, 40.9, epsilon);
  BOOST_REQUIRE_CLOSE(timeE3, 41.2, epsilon);
  BOOST_REQUIRE_CLOSE(timeE4, 41.8, epsilon);
  BOOST_REQUIRE_CLOSE(timeE5, 42.5, epsilon);
  BOOST_REQUIRE_CLOSE(timeE6, 42.9, epsilon);
}

BOOST_AUTO_TEST_SUITE_END()
