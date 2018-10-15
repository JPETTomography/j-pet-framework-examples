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
 *  @file HitFinderToolsTest.cpp
 */

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE HitFinderToolsTest

#include <JPetPhysSignal/JPetPhysSignal.h>
#include <JPetRecoSignal/JPetRecoSignal.h>
#include <JPetRawSignal/JPetRawSignal.h>
#include <boost/test/unit_test.hpp>
#include <JPetSigCh/JPetSigCh.h>
#include "JPetLoggerInclude.h"
#include "HitFinderTools.h"

BOOST_AUTO_TEST_SUITE(HitFinderTestSuite)

BOOST_AUTO_TEST_CASE(sortByTime_test)
{
  JPetPhysSignal sig1;
  JPetPhysSignal sig2;
  JPetPhysSignal sig3;
  JPetPhysSignal sig4;
  JPetPhysSignal sig5;
  sig1.setTime(7.0);
  sig2.setTime(4.0);
  sig3.setTime(16.0);
  sig4.setTime(1.0);
  sig5.setTime(9.0);
  std::vector<JPetPhysSignal> sigVec;
  sigVec.push_back(sig1);
  sigVec.push_back(sig2);
  sigVec.push_back(sig3);
  sigVec.push_back(sig4);
  sigVec.push_back(sig5);
  auto result = HitFinderTools::sortByTime(sigVec);
  auto epsilon = 0.001;
  BOOST_REQUIRE_CLOSE(result.at(0).getTime(), 1.0, epsilon);
  BOOST_REQUIRE_CLOSE(result.at(0).getTime(), 4.0, epsilon);
  BOOST_REQUIRE_CLOSE(result.at(0).getTime(), 7.0, epsilon);
  BOOST_REQUIRE_CLOSE(result.at(0).getTime(), 9.0, epsilon);
  BOOST_REQUIRE_CLOSE(result.at(0).getTime(), 16.0, epsilon);
}

BOOST_AUTO_TEST_CASE(getSignalsBySlot_test_empty)
{
  JPetParamBank paramBank;
  auto results =  HitFinderTools::getSignalsBySlot(nullptr, paramBank, false);
  BOOST_REQUIRE(results.empty());
}

BOOST_AUTO_TEST_CASE(getSignalsBySlot_test)
{
  JPetScin scin1(1);
  JPetScin scin2(2);
  JPetScin scin3(3);

  JPetParamBank paramBank;
  paramBank.addScintillator(scin1);
  paramBank.addScintillator(scin2);
  paramBank.addScintillator(scin3);

  JPetPM pm1(1, "first");
  JPetPM pm2(2, "second");
  JPetPM pm3(3, "third");
  JPetPM pm4(4, "fourth");
  JPetPM pm5(5, "fifth");
  JPetPM pm6(6, "sixth");

  pm1.setScin(scin1);
  pm2.setScin(scin1);
  pm3.setScin(scin2);
  pm4.setScin(scin2);
  pm5.setScin(scin3);
  pm6.setScin(scin3);

  JPetPhysSignal physSig1, physSig2, physSig3, physSig4;
  JPetPhysSignal physSig5, physSig6, physSig7, physSig8;
  physSig1.setPM(pm1);
  physSig2.setPM(pm1);
  physSig3.setPM(pm2);
  physSig4.setPM(pm2);
  physSig5.setPM(pm2);
  physSig6.setPM(pm3);
  physSig7.setPM(pm4);
  physSig8.setPM(pm5);

  physSig1.setRecoFlag(JPetBaseSignal::Good);
  physSig2.setRecoFlag(JPetBaseSignal::Good);
  physSig3.setRecoFlag(JPetBaseSignal::Corrupted);
  physSig4.setRecoFlag(JPetBaseSignal::Good);
  physSig5.setRecoFlag(JPetBaseSignal::Good);
  physSig6.setRecoFlag(JPetBaseSignal::Good);
  physSig7.setRecoFlag(JPetBaseSignal::Good);
  physSig8.setRecoFlag(JPetBaseSignal::Corrupted);

  JPetTimeWindow slot("JPetPhysSignal");
  slot.add<JPetPhysSignal>(physSig1);
  slot.add<JPetPhysSignal>(physSig2);
  slot.add<JPetPhysSignal>(physSig3);
  slot.add<JPetPhysSignal>(physSig4);
  slot.add<JPetPhysSignal>(physSig5);
  slot.add<JPetPhysSignal>(physSig6);
  slot.add<JPetPhysSignal>(physSig7);
  slot.add<JPetPhysSignal>(physSig8);

  auto results1 = HitFinderTools::getSignalsBySlot(
    &slot, paramBank, true
  );

  auto results2 = HitFinderTools::getSignalsBySlot(
    &slot, paramBank, false
  );

  BOOST_REQUIRE_EQUAL(results1.size(), 3);
  BOOST_REQUIRE_EQUAL(results2.size(), 3);
  BOOST_REQUIRE_EQUAL(results1.at(0).size(), 5);
  BOOST_REQUIRE_EQUAL(results1.at(1).size(), 2);
  BOOST_REQUIRE_EQUAL(results1.at(2).size(), 1);
  BOOST_REQUIRE_EQUAL(results2.at(0).size(), 4);
  BOOST_REQUIRE_EQUAL(results2.at(1).size(), 2);
  BOOST_REQUIRE_EQUAL(results2.at(2).size(), 0);
}

BOOST_AUTO_TEST_CASE(matchSignals_test_sameSide)
{
  JPetScin scin1(1);
  JPetPM pm1(11, "first");
  pm1.setScin(scin1);
  pm1.setSide(JPetPM::SideA);
  JPetPhysSignal physSig1, physSig2, physSig3;
  physSig1.setPM(pm1);
  physSig2.setPM(pm1);
  physSig3.setPM(pm1);
  physSig1.setTime(1.0);
  physSig2.setTime(2.0);
  physSig3.setTime(3.0);
  std::vector<JPetPhysSignal> slotSignals;
  slotSignals.push_back(physSig1);
  slotSignals.push_back(physSig2);
  slotSignals.push_back(physSig3);
  JPetStatistics stats;
  std::map<unsigned int, std::vector<double>> velocitiesMap;
  auto result = HitFinderTools::matchSignals(slotSignals, velocitiesMap, 5.0, -1, stats, false);
  BOOST_REQUIRE(result.empty());
}

BOOST_AUTO_TEST_CASE(matchSignals_test_timeDiff)
{
  JPetScin scin1(1);
  JPetPM pm1(11, "first");
  JPetPM pm2(22, "second");
  pm1.setScin(scin1);
  pm2.setScin(scin1);
  pm1.setSide(JPetPM::SideA);
  pm2.setSide(JPetPM::SideB);
  JPetPhysSignal physSig1, physSig2, physSig3, physSig4, physSig5, physSig6;
  physSig1.setPM(pm1);
  physSig2.setPM(pm2);
  physSig3.setPM(pm1);
  physSig4.setPM(pm2);
  physSig5.setPM(pm1);
  physSig6.setPM(pm2);
  physSig1.setTime(10.0);
  physSig2.setTime(20.0);
  physSig3.setTime(30.0);
  physSig4.setTime(40.0);
  physSig5.setTime(50.0);
  physSig6.setTime(52.0);
  std::vector<JPetPhysSignal> slotSignals;
  slotSignals.push_back(physSig1);
  slotSignals.push_back(physSig2);
  slotSignals.push_back(physSig3);
  slotSignals.push_back(physSig4);
  slotSignals.push_back(physSig5);
  slotSignals.push_back(physSig6);
  JPetStatistics stats;
  std::map<unsigned int, std::vector<double>> velocitiesMap;
  auto result = HitFinderTools::matchSignals(slotSignals, velocitiesMap, 5.0, -1, stats, false);
  BOOST_REQUIRE_EQUAL(result.size(), 1);
}

BOOST_AUTO_TEST_CASE(matchSignals_test_refDetSigs)
{
  JPetBarrelSlot refBarrelSlot(193, true, "refBS", 45.0, 193);
  JPetScin refScin(193);
  JPetPM refPm(11, "ref");
  refPm.setScin(refScin);
  refPm.setBarrelSlot(refBarrelSlot);

  JPetPhysSignal physSig1, physSig2, physSig3;
  physSig1.setPM(refPm);
  physSig2.setPM(refPm);
  physSig3.setPM(refPm);
  physSig1.setTime(1.0);
  physSig2.setTime(2.0);
  physSig3.setTime(3.0);
  std::vector<JPetPhysSignal> slotSignals;
  slotSignals.push_back(physSig1);
  slotSignals.push_back(physSig2);
  slotSignals.push_back(physSig3);

  JPetStatistics stats;
  std::map<unsigned int, std::vector<double>> velocitiesMap;
  auto result = HitFinderTools::matchSignals(slotSignals, velocitiesMap, 5.0, 193, stats, false);
  BOOST_REQUIRE_EQUAL(result.size(), 3);
}

BOOST_AUTO_TEST_CASE(matchSignals_test)
{
  JPetLayer layer1(1, true, "layer1", 10.0);
  JPetBarrelSlot barrelSlot1(23, true, "barel1", 30.0, 23);
  barrelSlot1.setLayer(layer1);

  JPetScin scin1(23);
  JPetPM pm1A(31, "1A");
  JPetPM pm1B(75, "1B");
  pm1A.setScin(scin1);
  pm1B.setScin(scin1);
  pm1A.setBarrelSlot(barrelSlot1);
  pm1B.setBarrelSlot(barrelSlot1);
  pm1A.setSide(JPetPM::SideA);
  pm1B.setSide(JPetPM::SideB);

  JPetTOMBChannel channel1(66);
  JPetTOMBChannel channel2(88);
  JPetSigCh sigCh1(JPetSigCh::Leading, 12.3);
  JPetSigCh sigCh2(JPetSigCh::Leading, 13.4);
  sigCh1.setTOMBChannel(channel1);
  sigCh2.setTOMBChannel(channel2);
  sigCh1.setThresholdNumber(1);
  sigCh2.setThresholdNumber(1);
  sigCh1.setPM(pm1A);
  sigCh2.setPM(pm1B);

  JPetRawSignal raw1;
  JPetRawSignal raw2;
  raw1.addPoint(sigCh1);
  raw2.addPoint(sigCh2);

  JPetRecoSignal reco1;
  JPetRecoSignal reco2;
  reco1.setRawSignal(raw1);
  reco2.setRawSignal(raw2);

  JPetPhysSignal physSig1A, physSig1B;
  JPetPhysSignal physSig2A, physSig2B;
  JPetPhysSignal physSig3A, physSig3B;
  JPetPhysSignal physSigA;
  physSigA.setRecoSignal(reco1);
  physSig1A.setRecoSignal(reco1);
  physSig2A.setRecoSignal(reco1);
  physSig3A.setRecoSignal(reco1);
  physSig1B.setRecoSignal(reco2);
  physSig2B.setRecoSignal(reco2);
  physSig3B.setRecoSignal(reco2);
  physSigA.setPM(pm1A);
  physSig1A.setPM(pm1A);
  physSig1B.setPM(pm1B);
  physSig2A.setPM(pm1A);
  physSig2B.setPM(pm1B);
  physSig3A.setPM(pm1A);
  physSig3B.setPM(pm1B);
  physSig1A.setTime(1.0);
  physSig1B.setTime(1.5);
  physSigA.setTime(2.2);
  physSig2A.setTime(4.0);
  physSig2B.setTime(4.8);
  physSig3A.setTime(7.2);
  physSig3B.setTime(6.5);
  std::vector<JPetPhysSignal> slotSignals;
  slotSignals.push_back(physSigA);
  slotSignals.push_back(physSig1A);
  slotSignals.push_back(physSig1B);
  slotSignals.push_back(physSig2A);
  slotSignals.push_back(physSig2B);
  slotSignals.push_back(physSig3A);
  slotSignals.push_back(physSig3B);

  JPetStatistics stats;
  std::map<unsigned int, std::vector<double>> velocitiesMap;
  std::vector<double> velVec = {2.0, 3.4, 4.5, 5.6};
  velocitiesMap.insert(std::make_pair(66, velVec));
  velocitiesMap.insert(std::make_pair(88, velVec));
  auto result = HitFinderTools::matchSignals(slotSignals, velocitiesMap, 1.0, -1, stats, false);
  auto epsilon = 0.0001;

  BOOST_REQUIRE_EQUAL(result.size(), 3);

  BOOST_REQUIRE_EQUAL(result.at(0).getSignalA().getPM().getID(), 31);
  BOOST_REQUIRE_EQUAL(result.at(0).getSignalB().getPM().getID(), 75);
  BOOST_REQUIRE_EQUAL(result.at(1).getSignalA().getPM().getID(), 31);
  BOOST_REQUIRE_EQUAL(result.at(1).getSignalB().getPM().getID(), 75);
  BOOST_REQUIRE_EQUAL(result.at(2).getSignalA().getPM().getID(), 31);
  BOOST_REQUIRE_EQUAL(result.at(2).getSignalB().getPM().getID(), 75);

  BOOST_REQUIRE_EQUAL(result.at(0).getSignalA().getPM().getScin().getID(), 23);
  BOOST_REQUIRE_EQUAL(result.at(0).getSignalB().getPM().getScin().getID(), 23);
  BOOST_REQUIRE_EQUAL(result.at(1).getSignalA().getPM().getScin().getID(), 23);
  BOOST_REQUIRE_EQUAL(result.at(1).getSignalB().getPM().getScin().getID(), 23);
  BOOST_REQUIRE_EQUAL(result.at(2).getSignalA().getPM().getScin().getID(), 23);
  BOOST_REQUIRE_EQUAL(result.at(2).getSignalB().getPM().getScin().getID(), 23);

  BOOST_REQUIRE_CLOSE(result.at(0).getTime(), (1.0+1.5)/2, epsilon);
  BOOST_REQUIRE_CLOSE(result.at(1).getTime(), (4.0+4.8)/2, epsilon);
  BOOST_REQUIRE_CLOSE(result.at(2).getTime(), (6.5+7.2)/2, epsilon);

  BOOST_REQUIRE_CLOSE(result.at(0).getTimeDiff(), 1.5-1.0, epsilon);
  BOOST_REQUIRE_CLOSE(result.at(1).getTimeDiff(), 4.8-4.0, epsilon);
  BOOST_REQUIRE_CLOSE(result.at(2).getTimeDiff(), 6.5-7.2, epsilon);

  BOOST_REQUIRE_CLOSE(result.at(0).getPosX(), 8.660254038, epsilon);
  BOOST_REQUIRE_CLOSE(result.at(1).getPosX(), 8.660254038, epsilon);
  BOOST_REQUIRE_CLOSE(result.at(2).getPosX(), 8.660254038, epsilon);

  BOOST_REQUIRE_CLOSE(result.at(0).getPosY(), 5.0, epsilon);
  BOOST_REQUIRE_CLOSE(result.at(1).getPosY(), 5.0, epsilon);
  BOOST_REQUIRE_CLOSE(result.at(2).getPosY(), 5.0, epsilon);

  BOOST_REQUIRE_CLOSE(result.at(0).getPosZ(), 2.0 * result.at(0).getTimeDiff() / 2000.0, epsilon);
  BOOST_REQUIRE_CLOSE(result.at(1).getPosZ(), 2.0 * result.at(1).getTimeDiff() / 2000.0, epsilon);
  BOOST_REQUIRE_CLOSE(result.at(2).getPosZ(), 2.0 * result.at(2).getTimeDiff() / 2000.0, epsilon);

  BOOST_REQUIRE(result.at(0).getPosZ()>0.0);
  BOOST_REQUIRE(result.at(1).getPosZ()>0.0);
  BOOST_REQUIRE(result.at(2).getPosZ()<0.0);
}

BOOST_AUTO_TEST_SUITE_END()
