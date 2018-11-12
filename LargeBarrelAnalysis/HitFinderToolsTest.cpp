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
  auto epsilon = 0.001;
  BOOST_REQUIRE_CLOSE(sigVec.at(0).getTime(), 7.0, epsilon);
  BOOST_REQUIRE_CLOSE(sigVec.at(1).getTime(), 4.0, epsilon);
  BOOST_REQUIRE_CLOSE(sigVec.at(2).getTime(), 16.0, epsilon);
  BOOST_REQUIRE_CLOSE(sigVec.at(3).getTime(), 1.0, epsilon);
  BOOST_REQUIRE_CLOSE(sigVec.at(4).getTime(), 9.0, epsilon);
  HitFinderTools::sortByTime(sigVec);
  BOOST_REQUIRE_CLOSE(sigVec.at(0).getTime(), 1.0, epsilon);
  BOOST_REQUIRE_CLOSE(sigVec.at(1).getTime(), 4.0, epsilon);
  BOOST_REQUIRE_CLOSE(sigVec.at(2).getTime(), 7.0, epsilon);
  BOOST_REQUIRE_CLOSE(sigVec.at(3).getTime(), 9.0, epsilon);
  BOOST_REQUIRE_CLOSE(sigVec.at(4).getTime(), 16.0, epsilon);
}

BOOST_AUTO_TEST_CASE(getSignalsBySlot_test_empty)
{
  auto results =  HitFinderTools::getSignalsBySlot(nullptr, false);
  BOOST_REQUIRE(results.empty());
}

BOOST_AUTO_TEST_CASE(getSignalsBySlot_test)
{
  JPetBarrelSlot slot1(1, true, "one", 15.0, 1);
  JPetBarrelSlot slot2(2, true, "two", 25.0, 1);
  JPetBarrelSlot slot3(3, true, "three", 35.0, 1);

  JPetPhysSignal physSig1, physSig2, physSig3, physSig4;
  JPetPhysSignal physSig5, physSig6, physSig7, physSig8;
  physSig1.setBarrelSlot(slot1);
  physSig2.setBarrelSlot(slot1);
  physSig3.setBarrelSlot(slot2);
  physSig4.setBarrelSlot(slot2);
  physSig5.setBarrelSlot(slot3);
  physSig6.setBarrelSlot(slot3);
  physSig7.setBarrelSlot(slot3);
  physSig8.setBarrelSlot(slot3);
  physSig1.setRecoFlag(JPetBaseSignal::Good);
  physSig2.setRecoFlag(JPetBaseSignal::Good);
  physSig3.setRecoFlag(JPetBaseSignal::Corrupted);
  physSig4.setRecoFlag(JPetBaseSignal::Corrupted);
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

  auto results1 = HitFinderTools::getSignalsBySlot(&slot, true);
  auto results2 = HitFinderTools::getSignalsBySlot(&slot, false);

  BOOST_REQUIRE_EQUAL(results1.size(), 3);
  BOOST_REQUIRE_EQUAL(results2.size(), 2);
  BOOST_REQUIRE_EQUAL(results1[1].size(), 2);
  BOOST_REQUIRE_EQUAL(results1[2].size(), 2);
  BOOST_REQUIRE_EQUAL(results1[3].size(), 4);
  BOOST_REQUIRE_EQUAL(results2[1].size(), 2);
  BOOST_REQUIRE_EQUAL(results2[3].size(), 3);
}

BOOST_AUTO_TEST_CASE(matchAllSignals_test_refDetID)
{
  JPetBarrelSlot slot(1, true, "one", 15.0, 1);
  JPetBarrelSlot refSlot(193, true, "refDet", 15.0, 1);
  JPetPM pm(11, "first");
  JPetPM refPM(385, "reference");
  pm.setBarrelSlot(slot);
  refPM.setBarrelSlot(refSlot);
  JPetPhysSignal physSig1, physSig2, physSig3;
  physSig1.setBarrelSlot(slot);
  physSig2.setBarrelSlot(refSlot);
  physSig3.setBarrelSlot(refSlot);
  physSig1.setPM(pm);
  physSig2.setPM(refPM);
  physSig3.setPM(refPM);
  physSig1.setTime(1.0);
  physSig2.setTime(2.0);
  physSig3.setTime(3.0);
  std::vector<JPetPhysSignal> slotSignals;
  slotSignals.push_back(physSig1);
  std::vector<JPetPhysSignal> refSignals;
  refSignals.push_back(physSig2);
  refSignals.push_back(physSig3);
  std::map<int, std::vector<JPetPhysSignal>> allSignals;
  allSignals.insert(std::make_pair(1, slotSignals));
  allSignals.insert(std::make_pair(193, refSignals));
  JPetStatistics stats;
  std::map<unsigned int, std::vector<double>> velocitiesMap;

  auto result1 = HitFinderTools::matchAllSignals(
    allSignals, velocitiesMap, 5.0, 193, stats, false
  );

  auto result2 = HitFinderTools::matchAllSignals(
    allSignals, velocitiesMap, 5.0, 1, stats, false
  );

  BOOST_REQUIRE_EQUAL(result1.size(), 2);
  BOOST_REQUIRE_EQUAL(result2.size(), 1);
}

BOOST_AUTO_TEST_CASE(matchSignals_test_sameSide)
{
  JPetBarrelSlot slot1(1, true, "one", 15.0, 1);
  JPetScin scin1(1);
  JPetPM pm1(11, "first");
  pm1.setBarrelSlot(slot1);
  pm1.setScin(scin1);
  pm1.setSide(JPetPM::SideA);
  JPetPhysSignal physSig1, physSig2, physSig3;
  physSig1.setBarrelSlot(slot1);
  physSig2.setBarrelSlot(slot1);
  physSig3.setBarrelSlot(slot1);
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
  auto result = HitFinderTools::matchSignals(slotSignals, velocitiesMap, 5.0, stats, false);
  BOOST_REQUIRE(result.empty());
}

BOOST_AUTO_TEST_CASE(matchSignals_test)
{
  JPetLayer layer1(1, true, "layer1", 10.0);
  JPetBarrelSlot slot1(23, true, "barel1", 30.0, 23);
  slot1.setLayer(layer1);

  JPetScin scin1(23);
  scin1.setBarrelSlot(slot1);
  JPetPM pm1A(31, "1A");
  JPetPM pm1B(75, "1B");
  pm1A.setScin(scin1);
  pm1B.setScin(scin1);
  pm1A.setBarrelSlot(slot1);
  pm1B.setBarrelSlot(slot1);
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
  raw1.setBarrelSlot(slot1);
  raw2.setBarrelSlot(slot1);

  JPetRecoSignal reco1;
  JPetRecoSignal reco2;
  reco1.setRawSignal(raw1);
  reco2.setRawSignal(raw2);
  reco1.setBarrelSlot(slot1);
  reco2.setBarrelSlot(slot1);

  JPetPhysSignal physSig1A, physSig1B;
  JPetPhysSignal physSig2A, physSig2B;
  JPetPhysSignal physSig3A, physSig3B;
  JPetPhysSignal physSigA;
  physSigA.setBarrelSlot(slot1);
  physSig1A.setBarrelSlot(slot1);
  physSig1B.setBarrelSlot(slot1);
  physSig2A.setBarrelSlot(slot1);
  physSig2B.setBarrelSlot(slot1);
  physSig3A.setBarrelSlot(slot1);
  physSig3B.setBarrelSlot(slot1);
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
  physSigA.setRecoFlag(JPetBaseSignal::Good);
  physSig1A.setRecoFlag(JPetBaseSignal::Good);
  physSig1B.setRecoFlag(JPetBaseSignal::Good);
  physSig2A.setRecoFlag(JPetBaseSignal::Good);
  physSig2B.setRecoFlag(JPetBaseSignal::Corrupted);
  physSig3A.setRecoFlag(JPetBaseSignal::Good);
  physSig3B.setRecoFlag(JPetBaseSignal::Good);
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
  auto result = HitFinderTools::matchSignals(slotSignals, velocitiesMap, 1.0, stats, false);
  auto epsilon = 0.0001;

  BOOST_REQUIRE_EQUAL(result.size(), 3);

  BOOST_REQUIRE_EQUAL(result.at(0).getSignalA().getPM().getID(), 31);
  BOOST_REQUIRE_EQUAL(result.at(0).getSignalB().getPM().getID(), 75);
  BOOST_REQUIRE_EQUAL(result.at(1).getSignalA().getPM().getID(), 31);
  BOOST_REQUIRE_EQUAL(result.at(1).getSignalB().getPM().getID(), 75);
  BOOST_REQUIRE_EQUAL(result.at(2).getSignalA().getPM().getID(), 31);
  BOOST_REQUIRE_EQUAL(result.at(2).getSignalB().getPM().getID(), 75);

  BOOST_REQUIRE_EQUAL(result.at(0).getSignalA().getRecoFlag(), JPetBaseSignal::Good);
  BOOST_REQUIRE_EQUAL(result.at(0).getSignalB().getRecoFlag(), JPetBaseSignal::Good);
  BOOST_REQUIRE_EQUAL(result.at(1).getSignalA().getRecoFlag(), JPetBaseSignal::Good);
  BOOST_REQUIRE_EQUAL(result.at(1).getSignalB().getRecoFlag(), JPetBaseSignal::Corrupted);
  BOOST_REQUIRE_EQUAL(result.at(2).getSignalA().getRecoFlag(), JPetBaseSignal::Good);
  BOOST_REQUIRE_EQUAL(result.at(2).getSignalB().getRecoFlag(), JPetBaseSignal::Good);

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

  BOOST_REQUIRE_EQUAL(result.at(0).getRecoFlag(), JPetHit::Good);
  BOOST_REQUIRE_EQUAL(result.at(1).getRecoFlag(), JPetHit::Corrupted);
  BOOST_REQUIRE_EQUAL(result.at(2).getRecoFlag(), JPetHit::Good);
}


BOOST_AUTO_TEST_CASE(getTot_test)
{
  ///This test is stupid, but I don't know for a moment what values to test.
  BOOST_CHECK_CLOSE(HitFinderTools::getToT(0), 0, 0.1);
}

BOOST_AUTO_TEST_SUITE_END()
