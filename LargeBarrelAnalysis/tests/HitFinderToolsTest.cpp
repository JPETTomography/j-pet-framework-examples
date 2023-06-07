/**
 *  @copyright Copyright 2023 The J-PET Framework Authors. All rights reserved.
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

#include <JPetCachedFunction/JPetCachedFunction.h>
#include <JPetLoggerInclude.h>
#include <JPetPhysSignal/JPetPhysSignal.h>
#include <JPetRawSignal/JPetRawSignal.h>
#include <JPetRecoSignal/JPetRecoSignal.h>
#include <JPetSigCh/JPetSigCh.h>

#include "../HitFinderTools.h"
#include "../ToTEnergyConverter.h"
#include "../ToTEnergyConverterFactory.h"

#include <boost/test/unit_test.hpp>

using namespace tot_energy_converter;
using namespace jpet_common_tools;

const double kEpsilon = 0.01;

using FunctionFormula = std::string;
using FunctionParams = std::vector<double>;
using FunctionLimits = std::pair<double, double>;
using FuncParamsAndLimits = std::pair<FunctionFormula, std::pair<FunctionParams, FunctionLimits>>;

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
  auto results = HitFinderTools::getSignalsBySlot(nullptr, false);
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

  // JPetCachedFunctionParams params("pol1", {0.0, 10.0});
  // ToTEnergyConverter conv(params, Range(10000, 0., 100.));

  FunctionFormula formula1 = "pol1";
  FunctionParams params1 = {-91958, 19341};
  FunctionLimits limits1 = {0, 100};

  FunctionFormula formula2 = "[0] + [1] * TMath::Log(x)";
  FunctionParams params2 = {1, -2};
  FunctionLimits limits2 = {2, 100};

  FuncParamsAndLimits e2tot = {formula1, {params1, limits1}};
  FuncParamsAndLimits tot2e = {formula2, {params2, limits2}};

  ToTEnergyConverterFactory fact;
  fact.setEnergyConverterOptions(e2tot);
  fact.setToTConverterOptions(tot2e);

  auto result1 = HitFinderTools::matchAllSignals(allSignals, velocitiesMap, 5.0, 193, fact, stats, false);

  auto result2 = HitFinderTools::matchAllSignals(allSignals, velocitiesMap, 5.0, 1, fact, stats, false);

  BOOST_REQUIRE_EQUAL(result1.size(), 2);
  BOOST_REQUIRE_EQUAL(result2.size(), 1);
}

BOOST_AUTO_TEST_CASE(createHit_test)
{
  // JPetLayer layer1(1, true, "layer1", 10.0);
  // JPetBarrelSlot slot1(2, true, "barel1", 30.0, 23);
  // slot1.setLayer(layer1);
  //
  // JPetScin scin1(1);
  // scin1.setBarrelSlot(slot1);
  //
  // JPetPM pmA(11, "first"), pmB(22, "second");
  // pmA.setScin(scin1);
  // pmB.setScin(scin1);
  // pmA.setBarrelSlot(slot1);
  // pmB.setBarrelSlot(slot1);
  // pmA.setSide(JPetPM::SideA);
  // pmB.setSide(JPetPM::SideB);
  //
  // JPetTOMBChannel channel1(66), channel2(88);
  // JPetSigCh sigCh1L(JPetSigCh::Leading, 10.0);
  // JPetSigCh sigCh1T(JPetSigCh::Trailing, 12.0);
  // JPetSigCh sigCh2L(JPetSigCh::Leading, 11.0);
  // JPetSigCh sigCh2T(JPetSigCh::Trailing, 13.0);
  // sigCh1L.setTOMBChannel(channel1);
  // sigCh1T.setTOMBChannel(channel1);
  // sigCh2L.setTOMBChannel(channel2);
  // sigCh2T.setTOMBChannel(channel2);
  // sigCh1L.setThresholdNumber(1);
  // sigCh1T.setThresholdNumber(1);
  // sigCh2L.setThresholdNumber(1);
  // sigCh2T.setThresholdNumber(1);
  // sigCh1L.setPM(pmA);
  // sigCh1T.setPM(pmA);
  // sigCh2L.setPM(pmB);
  // sigCh2T.setPM(pmB);
  //
  // JPetRawSignal raw1, raw2;
  // raw1.addPoint(sigCh1L);
  // raw1.addPoint(sigCh1T);
  // raw2.addPoint(sigCh2L);
  // raw2.addPoint(sigCh2T);
  // raw1.setBarrelSlot(slot1);
  // raw2.setBarrelSlot(slot1);
  // raw1.setPM(pmA);
  // raw2.setPM(pmB);
  //
  // JPetRecoSignal reco1, reco2;
  // reco1.setRawSignal(raw1);
  // reco2.setRawSignal(raw2);
  // reco1.setBarrelSlot(slot1);
  // reco2.setBarrelSlot(slot1);
  // reco1.setPM(pmA);
  // reco2.setPM(pmB);
  //
  // JPetPhysSignal physSigA, physSigB;
  // physSigA.setTime(10.0);
  // physSigB.setTime(12.0);
  // physSigA.setRecoSignal(reco1);
  // physSigB.setRecoSignal(reco2);
  //
  // std::map<unsigned int, std::vector<double>> velocitiesMap;
  // std::vector<double> velVec = {2.0, 3.4, 4.5, 5.6};
  // velocitiesMap.insert(std::make_pair(66, velVec));
  // velocitiesMap.insert(std::make_pair(88, velVec));
  //
  // JPetStatistics stats;
  //
  // JPetCachedFunctionParams params1("pol1", {0.0, 10.0});
  // ToTEnergyConverter conv1(params1, Range(10000, 0., 100.));
  //
  // auto hit1 = HitFinderTools::createHit(physSigA, physSigB, velocitiesMap, true, conv1, stats, false);
  //
  // auto epsilon = 0.0001;
  //
  // BOOST_REQUIRE_CLOSE(hit1.getTime(), 11.0, epsilon);
  // BOOST_REQUIRE_CLOSE(hit1.getQualityOfTime(), -1.0, epsilon);
  // BOOST_REQUIRE_CLOSE(hit1.getTimeDiff(), 2.0, epsilon);
  // BOOST_REQUIRE_CLOSE(hit1.getQualityOfTimeDiff(), -1.0, epsilon);
  // BOOST_REQUIRE_EQUAL(hit1.getScintillator().getID(), 1);
  // BOOST_REQUIRE_EQUAL(hit1.getBarrelSlot().getID(), 2);
  // BOOST_REQUIRE_CLOSE(hit1.getPosX(), 10.0 * cos(TMath::DegToRad() * 30.0), epsilon);
  // BOOST_REQUIRE_CLOSE(hit1.getPosY(), 10.0 * sin(TMath::DegToRad() * 30.0), epsilon);
  // BOOST_REQUIRE_CLOSE(hit1.getPosZ(), 4.0 * 1.0 / 2000.0, epsilon);
  // BOOST_REQUIRE_CLOSE(hit1.getEnergy(), 40.0, epsilon);
  // BOOST_REQUIRE_CLOSE(hit1.getQualityOfEnergy(), -1.0, epsilon);
  //
  // // Case: resultcan be caluculated but ToT is outside range of function
  // JPetCachedFunctionParams params2("pol1", {0.0, 10.0});
  // ToTEnergyConverter conv2(params2, Range(10000, 0., 1.));
  //
  // auto hit2 = HitFinderTools::createHit(physSigA, physSigB, velocitiesMap, true, conv2, stats, false);
  // BOOST_REQUIRE_CLOSE(hit2.getEnergy(), -1.0, epsilon);
  //
  // // Case with different function
  // JPetCachedFunctionParams params3("sqrt([0]*x)", {1.0});
  // ToTEnergyConverter conv3(params3, Range(10000, 0., 100.));
  //
  // auto hit3 = HitFinderTools::createHit(physSigA, physSigB, velocitiesMap, true, conv3, stats, false);
  // BOOST_REQUIRE_CLOSE(hit3.getEnergy(), 2, epsilon);
  //
  // // Case: result is -nan, energy set to -1.0
  // JPetCachedFunctionParams params4("sqrt([0]*x)", {-1.0});
  // ToTEnergyConverter conv4(params4, Range(10000, -100.0, 100.));
  //
  // auto hit4 = HitFinderTools::createHit(physSigA, physSigB, velocitiesMap, true, conv4, stats, false);
  // BOOST_REQUIRE_CLOSE(hit4.getEnergy(), -1.0, epsilon);
}

BOOST_AUTO_TEST_CASE(matchSignals_test_sameSide)
{
  // JPetBarrelSlot slot1(1, true, "one", 15.0, 1);
  // JPetScin scin1(1);
  // JPetPM pm1(11, "first");
  // pm1.setBarrelSlot(slot1);
  // pm1.setScin(scin1);
  // pm1.setSide(JPetPM::SideA);
  // JPetPhysSignal physSig1, physSig2, physSig3;
  // physSig1.setBarrelSlot(slot1);
  // physSig2.setBarrelSlot(slot1);
  // physSig3.setBarrelSlot(slot1);
  // physSig1.setPM(pm1);
  // physSig2.setPM(pm1);
  // physSig3.setPM(pm1);
  // physSig1.setTime(1.0);
  // physSig2.setTime(2.0);
  // physSig3.setTime(3.0);
  // std::vector<JPetPhysSignal> slotSignals;
  // slotSignals.push_back(physSig1);
  // slotSignals.push_back(physSig2);
  // slotSignals.push_back(physSig3);
  //
  // JPetStatistics stats;
  // std::map<unsigned int, std::vector<double>> velocitiesMap;
  // JPetCachedFunctionParams params("pol1", {0.0, 10.0});
  // ToTEnergyConverter conv(params, Range(10000, 0., 100.));
  //
  // auto result = HitFinderTools::matchSignals(slotSignals, velocitiesMap, 5.0, false, conv, stats, false);
  // BOOST_REQUIRE(result.empty());
}

BOOST_AUTO_TEST_CASE(matchSignals_test)
{
  // JPetLayer layer1(1, true, "layer1", 10.0);
  // JPetBarrelSlot slot1(23, true, "barel1", 30.0, 23);
  // slot1.setLayer(layer1);
  //
  // JPetScin scin1(23);
  // scin1.setBarrelSlot(slot1);
  // JPetPM pm1A(31, "1A");
  // JPetPM pm1B(75, "1B");
  // pm1A.setScin(scin1);
  // pm1B.setScin(scin1);
  // pm1A.setBarrelSlot(slot1);
  // pm1B.setBarrelSlot(slot1);
  // pm1A.setSide(JPetPM::SideA);
  // pm1B.setSide(JPetPM::SideB);
  //
  // JPetTOMBChannel channel1(66);
  // JPetTOMBChannel channel2(88);
  // JPetSigCh sigCh1(JPetSigCh::Leading, 12.3);
  // JPetSigCh sigCh2(JPetSigCh::Leading, 13.4);
  // sigCh1.setTOMBChannel(channel1);
  // sigCh2.setTOMBChannel(channel2);
  // sigCh1.setThresholdNumber(1);
  // sigCh2.setThresholdNumber(1);
  // sigCh1.setPM(pm1A);
  // sigCh2.setPM(pm1B);
  //
  // JPetRawSignal raw1;
  // JPetRawSignal raw2;
  // raw1.addPoint(sigCh1);
  // raw2.addPoint(sigCh2);
  // raw1.setBarrelSlot(slot1);
  // raw2.setBarrelSlot(slot1);
  //
  // JPetRecoSignal reco1;
  // JPetRecoSignal reco2;
  // reco1.setRawSignal(raw1);
  // reco2.setRawSignal(raw2);
  // reco1.setBarrelSlot(slot1);
  // reco2.setBarrelSlot(slot1);
  //
  // JPetPhysSignal physSig1A, physSig1B;
  // JPetPhysSignal physSig2A, physSig2B;
  // JPetPhysSignal physSig3A, physSig3B;
  // JPetPhysSignal physSigA;
  // physSigA.setBarrelSlot(slot1);
  // physSig1A.setBarrelSlot(slot1);
  // physSig1B.setBarrelSlot(slot1);
  // physSig2A.setBarrelSlot(slot1);
  // physSig2B.setBarrelSlot(slot1);
  // physSig3A.setBarrelSlot(slot1);
  // physSig3B.setBarrelSlot(slot1);
  // physSigA.setRecoSignal(reco1);
  // physSig1A.setRecoSignal(reco1);
  // physSig2A.setRecoSignal(reco1);
  // physSig3A.setRecoSignal(reco1);
  // physSig1B.setRecoSignal(reco2);
  // physSig2B.setRecoSignal(reco2);
  // physSig3B.setRecoSignal(reco2);
  // physSigA.setPM(pm1A);
  // physSig1A.setPM(pm1A);
  // physSig1B.setPM(pm1B);
  // physSig2A.setPM(pm1A);
  // physSig2B.setPM(pm1B);
  // physSig3A.setPM(pm1A);
  // physSig3B.setPM(pm1B);
  // physSig1A.setTime(1.0);
  // physSig1B.setTime(1.5);
  // physSigA.setTime(2.2);
  // physSig2A.setTime(4.0);
  // physSig2B.setTime(4.8);
  // physSig3A.setTime(7.2);
  // physSig3B.setTime(6.5);
  // physSigA.setRecoFlag(JPetBaseSignal::Good);
  // physSig1A.setRecoFlag(JPetBaseSignal::Good);
  // physSig1B.setRecoFlag(JPetBaseSignal::Good);
  // physSig2A.setRecoFlag(JPetBaseSignal::Good);
  // physSig2B.setRecoFlag(JPetBaseSignal::Corrupted);
  // physSig3A.setRecoFlag(JPetBaseSignal::Good);
  // physSig3B.setRecoFlag(JPetBaseSignal::Good);
  // std::vector<JPetPhysSignal> slotSignals;
  // slotSignals.push_back(physSigA);
  // slotSignals.push_back(physSig1A);
  // slotSignals.push_back(physSig1B);
  // slotSignals.push_back(physSig2A);
  // slotSignals.push_back(physSig2B);
  // slotSignals.push_back(physSig3A);
  // slotSignals.push_back(physSig3B);
  //
  // JPetStatistics stats;
  // std::map<unsigned int, std::vector<double>> velocitiesMap;
  // std::vector<double> velVec = {2.0, 3.4, 4.5, 5.6};
  // velocitiesMap.insert(std::make_pair(66, velVec));
  // velocitiesMap.insert(std::make_pair(88, velVec));
  //
  // JPetCachedFunctionParams params("pol1", {0.0, 10.0});
  // ToTEnergyConverter conv(params, Range(10000, 0., 100.));
  //
  // auto result = HitFinderTools::matchSignals(slotSignals, velocitiesMap, 1.0, false, conv, stats, false);
  // auto epsilon = 0.0001;
  //
  // BOOST_REQUIRE_EQUAL(result.size(), 3);
  //
  // BOOST_REQUIRE_EQUAL(result.at(0).getSignalA().getPM().getID(), 31);
  // BOOST_REQUIRE_EQUAL(result.at(0).getSignalB().getPM().getID(), 75);
  // BOOST_REQUIRE_EQUAL(result.at(1).getSignalA().getPM().getID(), 31);
  // BOOST_REQUIRE_EQUAL(result.at(1).getSignalB().getPM().getID(), 75);
  // BOOST_REQUIRE_EQUAL(result.at(2).getSignalA().getPM().getID(), 31);
  // BOOST_REQUIRE_EQUAL(result.at(2).getSignalB().getPM().getID(), 75);
  //
  // BOOST_REQUIRE_EQUAL(result.at(0).getSignalA().getRecoFlag(), JPetBaseSignal::Good);
  // BOOST_REQUIRE_EQUAL(result.at(0).getSignalB().getRecoFlag(), JPetBaseSignal::Good);
  // BOOST_REQUIRE_EQUAL(result.at(1).getSignalA().getRecoFlag(), JPetBaseSignal::Good);
  // BOOST_REQUIRE_EQUAL(result.at(1).getSignalB().getRecoFlag(), JPetBaseSignal::Corrupted);
  // BOOST_REQUIRE_EQUAL(result.at(2).getSignalA().getRecoFlag(), JPetBaseSignal::Good);
  // BOOST_REQUIRE_EQUAL(result.at(2).getSignalB().getRecoFlag(), JPetBaseSignal::Good);
  //
  // BOOST_REQUIRE_EQUAL(result.at(0).getSignalA().getPM().getScin().getID(), 23);
  // BOOST_REQUIRE_EQUAL(result.at(0).getSignalB().getPM().getScin().getID(), 23);
  // BOOST_REQUIRE_EQUAL(result.at(1).getSignalA().getPM().getScin().getID(), 23);
  // BOOST_REQUIRE_EQUAL(result.at(1).getSignalB().getPM().getScin().getID(), 23);
  // BOOST_REQUIRE_EQUAL(result.at(2).getSignalA().getPM().getScin().getID(), 23);
  // BOOST_REQUIRE_EQUAL(result.at(2).getSignalB().getPM().getScin().getID(), 23);
  //
  // BOOST_REQUIRE_CLOSE(result.at(0).getTime(), (1.0 + 1.5) / 2, epsilon);
  // BOOST_REQUIRE_CLOSE(result.at(1).getTime(), (4.0 + 4.8) / 2, epsilon);
  // BOOST_REQUIRE_CLOSE(result.at(2).getTime(), (6.5 + 7.2) / 2, epsilon);
  //
  // BOOST_REQUIRE_CLOSE(result.at(0).getTimeDiff(), 1.5 - 1.0, epsilon);
  // BOOST_REQUIRE_CLOSE(result.at(1).getTimeDiff(), 4.8 - 4.0, epsilon);
  // BOOST_REQUIRE_CLOSE(result.at(2).getTimeDiff(), 6.5 - 7.2, epsilon);
  //
  // BOOST_REQUIRE_CLOSE(result.at(0).getPosX(), 8.660254038, epsilon);
  // BOOST_REQUIRE_CLOSE(result.at(1).getPosX(), 8.660254038, epsilon);
  // BOOST_REQUIRE_CLOSE(result.at(2).getPosX(), 8.660254038, epsilon);
  //
  // BOOST_REQUIRE_CLOSE(result.at(0).getPosY(), 5.0, epsilon);
  // BOOST_REQUIRE_CLOSE(result.at(1).getPosY(), 5.0, epsilon);
  // BOOST_REQUIRE_CLOSE(result.at(2).getPosY(), 5.0, epsilon);
  //
  // BOOST_REQUIRE_CLOSE(result.at(0).getPosZ(), 2.0 * result.at(0).getTimeDiff() / 2000.0, epsilon);
  // BOOST_REQUIRE_CLOSE(result.at(1).getPosZ(), 2.0 * result.at(1).getTimeDiff() / 2000.0, epsilon);
  // BOOST_REQUIRE_CLOSE(result.at(2).getPosZ(), 2.0 * result.at(2).getTimeDiff() / 2000.0, epsilon);
  //
  // BOOST_REQUIRE(result.at(0).getPosZ() > 0.0);
  // BOOST_REQUIRE(result.at(1).getPosZ() > 0.0);
  // BOOST_REQUIRE(result.at(2).getPosZ() < 0.0);
  //
  // BOOST_REQUIRE_EQUAL(result.at(0).getRecoFlag(), JPetHit::Good);
  // BOOST_REQUIRE_EQUAL(result.at(1).getRecoFlag(), JPetHit::Corrupted);
  // BOOST_REQUIRE_EQUAL(result.at(2).getRecoFlag(), JPetHit::Good);
}

BOOST_AUTO_TEST_CASE(checkForPromptTest_checkTOTCalc)
{
  JPetBarrelSlot barrelSlot(666, true, "Some Slot", 66.0, 666);
  JPetPM pmA(1, "A");
  JPetPM pmB(2, "B");
  pmA.setSide(JPetPM::SideA);
  pmB.setSide(JPetPM::SideB);
  pmA.setBarrelSlot(barrelSlot);
  pmB.setBarrelSlot(barrelSlot);

  JPetSigCh sigCh1;
  JPetSigCh sigCh2;
  JPetSigCh sigCh3;
  JPetSigCh sigCh4;
  JPetSigCh sigCh5;
  JPetSigCh sigCh6;
  JPetSigCh sigCh7;
  JPetSigCh sigCh8;

  sigCh1.setType(JPetSigCh::Leading);
  sigCh2.setType(JPetSigCh::Leading);
  sigCh3.setType(JPetSigCh::Leading);
  sigCh4.setType(JPetSigCh::Leading);
  sigCh5.setType(JPetSigCh::Trailing);
  sigCh6.setType(JPetSigCh::Trailing);
  sigCh7.setType(JPetSigCh::Trailing);
  sigCh8.setType(JPetSigCh::Trailing);

  sigCh1.setThresholdNumber(1);
  sigCh2.setThresholdNumber(2);
  sigCh3.setThresholdNumber(3);
  sigCh4.setThresholdNumber(4);
  sigCh5.setThresholdNumber(1);
  sigCh6.setThresholdNumber(2);
  sigCh7.setThresholdNumber(3);
  sigCh8.setThresholdNumber(4);

  sigCh1.setThreshold(80);
  sigCh2.setThreshold(160);
  sigCh3.setThreshold(240);
  sigCh4.setThreshold(320);
  sigCh5.setThreshold(80);
  sigCh6.setThreshold(160);
  sigCh7.setThreshold(240);
  sigCh8.setThreshold(320);

  sigCh1.setValue(10.0);
  sigCh2.setValue(12.0);
  sigCh3.setValue(14.0);
  sigCh4.setValue(16.0);
  sigCh5.setValue(23.0);
  sigCh6.setValue(21.0);
  sigCh7.setValue(19.0);
  sigCh8.setValue(17.0);

  JPetRawSignal rawSig1A;
  JPetRawSignal rawSig1B;
  JPetRawSignal rawSig2A;
  JPetRawSignal rawSig2B;
  JPetRawSignal rawSig3A;
  JPetRawSignal rawSig3B;

  rawSig1A.setPM(pmA);
  rawSig1B.setPM(pmB);
  rawSig2A.setPM(pmA);
  rawSig2B.setPM(pmB);
  rawSig3A.setPM(pmA);
  rawSig3B.setPM(pmB);

  rawSig1A.addPoint(sigCh1);
  rawSig1B.addPoint(sigCh1);

  rawSig2A.addPoint(sigCh1);
  rawSig2A.addPoint(sigCh2);
  rawSig2A.addPoint(sigCh3);
  rawSig2A.addPoint(sigCh4);
  rawSig2A.addPoint(sigCh5);
  rawSig2A.addPoint(sigCh6);
  rawSig2A.addPoint(sigCh7);
  rawSig2A.addPoint(sigCh8);

  rawSig2B.addPoint(sigCh1);
  rawSig2B.addPoint(sigCh2);
  rawSig2B.addPoint(sigCh3);
  rawSig2B.addPoint(sigCh4);
  rawSig2B.addPoint(sigCh5);
  rawSig2B.addPoint(sigCh6);
  rawSig2B.addPoint(sigCh7);
  rawSig2B.addPoint(sigCh8);

  sigCh1.setValue(100.0);
  sigCh2.setValue(120.0);
  sigCh3.setValue(140.0);
  sigCh4.setValue(160.0);
  sigCh5.setValue(230.0);
  sigCh6.setValue(210.0);
  sigCh7.setValue(190.0);
  sigCh8.setValue(170.0);

  rawSig3A.addPoint(sigCh1);
  rawSig3A.addPoint(sigCh2);
  rawSig3A.addPoint(sigCh3);
  rawSig3A.addPoint(sigCh4);
  rawSig3A.addPoint(sigCh5);
  rawSig3A.addPoint(sigCh6);
  rawSig3A.addPoint(sigCh7);
  rawSig3A.addPoint(sigCh8);

  rawSig3B.addPoint(sigCh1);
  rawSig3B.addPoint(sigCh2);
  rawSig3B.addPoint(sigCh3);
  rawSig3B.addPoint(sigCh4);
  rawSig3B.addPoint(sigCh5);
  rawSig3B.addPoint(sigCh6);
  rawSig3B.addPoint(sigCh7);
  rawSig3B.addPoint(sigCh8);

  JPetRecoSignal recoSignal1A;
  JPetRecoSignal recoSignal1B;
  JPetRecoSignal recoSignal2A;
  JPetRecoSignal recoSignal2B;
  JPetRecoSignal recoSignal3A;
  JPetRecoSignal recoSignal3B;

  recoSignal1A.setRawSignal(rawSig1A);
  recoSignal1B.setRawSignal(rawSig1B);
  recoSignal2A.setRawSignal(rawSig2A);
  recoSignal2B.setRawSignal(rawSig2B);
  recoSignal3A.setRawSignal(rawSig3A);
  recoSignal3B.setRawSignal(rawSig3B);

  JPetPhysSignal physSignal1A;
  JPetPhysSignal physSignal1B;
  JPetPhysSignal physSignal2A;
  JPetPhysSignal physSignal2B;
  JPetPhysSignal physSignal3A;
  JPetPhysSignal physSignal3B;

  physSignal1A.setRecoSignal(recoSignal1A);
  physSignal1B.setRecoSignal(recoSignal1B);
  physSignal2A.setRecoSignal(recoSignal2A);
  physSignal2B.setRecoSignal(recoSignal2B);
  physSignal3A.setRecoSignal(recoSignal3A);
  physSignal3B.setRecoSignal(recoSignal3B);

  JPetHit hit1;
  JPetHit hit2;
  JPetHit hit3;

  hit1.setSignals(physSignal1A, physSignal1B);
  hit2.setSignals(physSignal2A, physSignal2B);
  hit3.setSignals(physSignal3A, physSignal3B);

  std::string TOTCalculationType = "standard";
  BOOST_REQUIRE_CLOSE(HitFinderTools::calculateTOT(hit1, HitFinderTools::getTOTCalculationType(TOTCalculationType)), 0.0, kEpsilon);
  BOOST_REQUIRE_CLOSE(HitFinderTools::calculateTOT(hit2, HitFinderTools::getTOTCalculationType(TOTCalculationType)), 56.0, kEpsilon);
  BOOST_REQUIRE_CLOSE(HitFinderTools::calculateTOT(hit3, HitFinderTools::getTOTCalculationType(TOTCalculationType)), 560.0, kEpsilon);
}

BOOST_AUTO_TEST_SUITE_END()
