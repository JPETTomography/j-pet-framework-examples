/**
 *  @copyright Copyright 2021 The J-PET Framework Authors. All rights reserved.
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
#define BOOST_TEST_MODULE HitFinderToolsTestSuite

#include "../HitFinderTools.h"
#include <JPetStatistics/JPetStatistics.h>
#include <boost/test/unit_test.hpp>

auto epsilon = 0.0001;

BOOST_AUTO_TEST_SUITE(HitFinderToolsTestSuite)

BOOST_AUTO_TEST_CASE(sortByTime_signals_test)
{
  JPetMatrixSignal mtxSig1, mtxSig2, mtxSig3, mtxSig4, mtxSig5, mtxSig6;
  mtxSig1.setTime(2.2);
  mtxSig2.setTime(4.4);
  mtxSig3.setTime(1.1);
  mtxSig4.setTime(6.6);
  mtxSig5.setTime(5.5);
  mtxSig6.setTime(3.3);

  std::vector<JPetMatrixSignal> mtxSignals;
  mtxSignals.push_back(mtxSig1);
  mtxSignals.push_back(mtxSig2);
  mtxSignals.push_back(mtxSig3);
  mtxSignals.push_back(mtxSig4);
  mtxSignals.push_back(mtxSig5);
  mtxSignals.push_back(mtxSig6);

  BOOST_REQUIRE_CLOSE(mtxSignals.at(0).getTime(), 2.2, epsilon);
  BOOST_REQUIRE_CLOSE(mtxSignals.at(1).getTime(), 4.4, epsilon);
  BOOST_REQUIRE_CLOSE(mtxSignals.at(2).getTime(), 1.1, epsilon);
  BOOST_REQUIRE_CLOSE(mtxSignals.at(3).getTime(), 6.6, epsilon);
  BOOST_REQUIRE_CLOSE(mtxSignals.at(4).getTime(), 5.5, epsilon);
  BOOST_REQUIRE_CLOSE(mtxSignals.at(5).getTime(), 3.3, epsilon);

  HitFinderTools::sortByTime(mtxSignals);

  BOOST_REQUIRE_CLOSE(mtxSignals.at(0).getTime(), 1.1, epsilon);
  BOOST_REQUIRE_CLOSE(mtxSignals.at(1).getTime(), 2.2, epsilon);
  BOOST_REQUIRE_CLOSE(mtxSignals.at(2).getTime(), 3.3, epsilon);
  BOOST_REQUIRE_CLOSE(mtxSignals.at(3).getTime(), 4.4, epsilon);
  BOOST_REQUIRE_CLOSE(mtxSignals.at(4).getTime(), 5.5, epsilon);
  BOOST_REQUIRE_CLOSE(mtxSignals.at(5).getTime(), 6.6, epsilon);
}

BOOST_AUTO_TEST_CASE(sortByTime_hits_test)
{
  JPetPhysRecoHit hit1, hit2, hit3, hit4, hit5, hit6;
  hit1.setTime(2.2);
  hit2.setTime(4.4);
  hit3.setTime(1.1);
  hit4.setTime(6.6);
  hit5.setTime(5.5);
  hit6.setTime(3.3);

  std::vector<JPetPhysRecoHit> hits;
  hits.push_back(hit1);
  hits.push_back(hit2);
  hits.push_back(hit3);
  hits.push_back(hit4);
  hits.push_back(hit5);
  hits.push_back(hit6);

  BOOST_REQUIRE_CLOSE(hits.at(0).getTime(), 2.2, epsilon);
  BOOST_REQUIRE_CLOSE(hits.at(1).getTime(), 4.4, epsilon);
  BOOST_REQUIRE_CLOSE(hits.at(2).getTime(), 1.1, epsilon);
  BOOST_REQUIRE_CLOSE(hits.at(3).getTime(), 6.6, epsilon);
  BOOST_REQUIRE_CLOSE(hits.at(4).getTime(), 5.5, epsilon);
  BOOST_REQUIRE_CLOSE(hits.at(5).getTime(), 3.3, epsilon);

  HitFinderTools::sortByTime(hits);

  BOOST_REQUIRE_CLOSE(hits.at(0).getTime(), 1.1, epsilon);
  BOOST_REQUIRE_CLOSE(hits.at(1).getTime(), 2.2, epsilon);
  BOOST_REQUIRE_CLOSE(hits.at(2).getTime(), 3.3, epsilon);
  BOOST_REQUIRE_CLOSE(hits.at(3).getTime(), 4.4, epsilon);
  BOOST_REQUIRE_CLOSE(hits.at(4).getTime(), 5.5, epsilon);
  BOOST_REQUIRE_CLOSE(hits.at(5).getTime(), 6.6, epsilon);
}

BOOST_AUTO_TEST_CASE(getSignalsByScin_test_empty)
{
  auto results = HitFinderTools::getSignalsByScin(nullptr);
  BOOST_REQUIRE(results.empty());
}

BOOST_AUTO_TEST_CASE(getSignalsByScin_test)
{
  JPetScin scin1(1, 50.0, 19.0, 3.0, -1.0, 0.0, 0.0, 45.0, 45.0, 45.0);
  JPetScin scin2(2, 50.0, 19.0, 3.0, -1.0, 0.0, 0.0, 45.0, 45.0, 45.0);
  JPetScin scin3(3, 50.0, 19.0, 3.0, -1.0, 0.0, 0.0, 45.0, 45.0, 45.0);
  JPetMatrix mtx1A(1, JPetMatrix::SideA);
  JPetMatrix mtx1B(2, JPetMatrix::SideB);
  JPetMatrix mtx2A(3, JPetMatrix::SideA);
  JPetMatrix mtx2B(4, JPetMatrix::SideB);
  JPetMatrix mtx3A(5, JPetMatrix::SideA);
  JPetMatrix mtx3B(6, JPetMatrix::SideB);
  mtx1A.setScin(scin1);
  mtx1B.setScin(scin1);
  mtx2A.setScin(scin2);
  mtx2B.setScin(scin2);
  mtx3A.setScin(scin3);
  mtx3B.setScin(scin3);

  JPetMatrixSignal mtxSig1, mtxSig2, mtxSig3, mtxSig4, mtxSig5, mtxSig6, mtxSig7, mtxSig8, mtxSig9;
  mtxSig1.setMatrix(mtx1A);
  mtxSig2.setMatrix(mtx1B);
  mtxSig3.setMatrix(mtx2A);
  mtxSig4.setMatrix(mtx2A);
  mtxSig5.setMatrix(mtx2A);
  mtxSig6.setMatrix(mtx2B);
  mtxSig7.setMatrix(mtx3B);
  mtxSig8.setMatrix(mtx3B);
  mtxSig9.setMatrix(mtx3B);

  JPetTimeWindow timeSlot("JPetMatrixSignal");
  timeSlot.add<JPetMatrixSignal>(mtxSig1);
  timeSlot.add<JPetMatrixSignal>(mtxSig2);
  timeSlot.add<JPetMatrixSignal>(mtxSig3);
  timeSlot.add<JPetMatrixSignal>(mtxSig4);
  timeSlot.add<JPetMatrixSignal>(mtxSig5);
  timeSlot.add<JPetMatrixSignal>(mtxSig6);
  timeSlot.add<JPetMatrixSignal>(mtxSig7);
  timeSlot.add<JPetMatrixSignal>(mtxSig8);
  timeSlot.add<JPetMatrixSignal>(mtxSig9);

  auto result = HitFinderTools::getSignalsByScin(&timeSlot);
  BOOST_REQUIRE_EQUAL(result.size(), 3);
  BOOST_REQUIRE_EQUAL(result.at(1).size(), 2);
  BOOST_REQUIRE_EQUAL(result.at(2).size(), 4);
  BOOST_REQUIRE_EQUAL(result.at(3).size(), 3);
}

BOOST_AUTO_TEST_CASE(matchSignals_test)
{
  JPetScin scin1(1, 50.0, 19.0, 3.0, -1.0, 0.0, 0.0, 45.0, 45.0, 45.0);
  JPetScin scin2(2, 50.0, 19.0, 3.0, -1.0, 0.0, 0.0, 45.0, 45.0, 45.0);
  JPetScin scin3(3, 50.0, 19.0, 3.0, -1.0, 0.0, 0.0, 45.0, 45.0, 45.0);
  JPetMatrix mtx1A(1, JPetMatrix::SideA);
  JPetMatrix mtx1B(2, JPetMatrix::SideB);
  JPetMatrix mtx2A(3, JPetMatrix::SideA);
  JPetMatrix mtx2B(4, JPetMatrix::SideB);
  JPetMatrix mtx3A(5, JPetMatrix::SideA);
  JPetMatrix mtx3B(6, JPetMatrix::SideB);
  mtx1A.setScin(scin1);
  mtx1B.setScin(scin1);
  mtx2A.setScin(scin2);
  mtx2B.setScin(scin2);
  mtx3A.setScin(scin3);
  mtx3B.setScin(scin3);

  JPetMatrixSignal mtxSig1, mtxSig2, mtxSig3, mtxSig4, mtxSig5, mtxSig6, mtxSig7, mtxSig8, mtxSig9;
  JPetMatrixSignal mtxSig10, mtxSig11, mtxSig12, mtxSig13, mtxSig14, mtxSig15, mtxSig16;
  mtxSig1.setMatrix(mtx1A);
  mtxSig2.setMatrix(mtx1B);
  mtxSig1.setTime(10.0);
  mtxSig2.setTime(12.0);

  mtxSig3.setMatrix(mtx2A);
  mtxSig4.setMatrix(mtx2A);
  mtxSig5.setMatrix(mtx2A);
  mtxSig6.setMatrix(mtx2B);
  mtxSig3.setTime(20.0);
  mtxSig4.setTime(21.0);
  mtxSig5.setTime(25.0);
  mtxSig6.setTime(29.0);

  mtxSig7.setMatrix(mtx3B);
  mtxSig8.setMatrix(mtx3B);
  mtxSig9.setMatrix(mtx3B);
  mtxSig7.setTime(30.0);
  mtxSig8.setTime(31.0);
  mtxSig9.setTime(32.0);

  mtxSig10.setMatrix(mtx1A);
  mtxSig11.setMatrix(mtx1A);
  mtxSig12.setMatrix(mtx1B);
  mtxSig10.setTime(40.0);
  mtxSig11.setTime(41.0);
  mtxSig12.setTime(42.0);

  mtxSig13.setMatrix(mtx1A);
  mtxSig14.setMatrix(mtx1B);
  mtxSig13.setTime(50.0);
  mtxSig14.setTime(54.0);

  mtxSig15.setMatrix(mtx1A);
  mtxSig16.setMatrix(mtx1B);
  mtxSig15.setTime(60.0);
  mtxSig16.setTime(69.0);

  std::vector<JPetMatrixSignal> signals1;
  signals1.push_back(mtxSig1);
  signals1.push_back(mtxSig2);
  signals1.push_back(mtxSig3);
  signals1.push_back(mtxSig4);
  signals1.push_back(mtxSig5);
  signals1.push_back(mtxSig6);
  signals1.push_back(mtxSig7);
  signals1.push_back(mtxSig8);
  signals1.push_back(mtxSig9);
  signals1.push_back(mtxSig10);
  signals1.push_back(mtxSig11);
  signals1.push_back(mtxSig12);
  signals1.push_back(mtxSig13);
  signals1.push_back(mtxSig14);
  signals1.push_back(mtxSig15);
  signals1.push_back(mtxSig16);

  auto signals2 = signals1;

  JPetStatistics stats;
  boost::property_tree::ptree empty;
  auto result1 = HitFinderTools::matchSignals(signals1, 5.0, empty, stats, false);
  auto result2 = HitFinderTools::matchSignals(signals2, 3.0, empty, stats, false);

  BOOST_REQUIRE_EQUAL(result1.size(), 4);
  BOOST_REQUIRE_CLOSE(result1.at(0).getTime(), 11.0, epsilon);
  BOOST_REQUIRE_CLOSE(result1.at(1).getTime(), 27.0, epsilon);
  BOOST_REQUIRE_CLOSE(result1.at(2).getTime(), 41.0, epsilon);
  BOOST_REQUIRE_CLOSE(result1.at(3).getTime(), 52.0, epsilon);

  BOOST_REQUIRE_EQUAL(result2.size(), 2);
  BOOST_REQUIRE_CLOSE(result2.at(0).getTime(), 11.0, epsilon);
  BOOST_REQUIRE_CLOSE(result2.at(1).getTime(), 41.0, epsilon);
}

BOOST_AUTO_TEST_CASE(createHit_test)
{
  JPetScin scin(123, 50.0, 19.0, 3.0, 10.0, 10.0, 10.0, 0.0, 90.0, 0.0);
  JPetMatrix mtxA(1, JPetMatrix::SideA);
  JPetMatrix mtxB(2, JPetMatrix::SideB);
  mtxA.setScin(scin);
  mtxB.setScin(scin);

  JPetMatrixSignal mtxSig1, mtxSig2;
  mtxSig1.setMatrix(mtxA);
  mtxSig2.setMatrix(mtxB);
  mtxSig1.setTime(2.0);
  mtxSig2.setTime(6.0);

  JPetPM pm1A(1, "1A", 1);
  JPetPM pm2A(2, "2A", 2);
  JPetPM pm1B(3, "1B", 1);
  JPetPM pm2B(4, "2B", 2);
  pm1A.setMatrix(mtxA);
  pm2A.setMatrix(mtxA);
  pm1B.setMatrix(mtxB);
  pm2B.setMatrix(mtxB);

  JPetPMSignal pmSig1, pmSig2, pmSig3, pmSig4;
  pmSig1.setPM(pm1A);
  pmSig2.setPM(pm2A);
  pmSig1.setTime(1.0);
  pmSig2.setTime(3.0);
  pmSig1.setToT(10.0);
  pmSig2.setToT(14.0);
  pmSig3.setPM(pm1B);
  pmSig4.setPM(pm2B);
  pmSig3.setTime(5.0);
  pmSig4.setTime(7.0);
  pmSig3.setToT(12.0);
  pmSig4.setToT(16.0);

  mtxSig1.addPMSignal(pmSig1);
  mtxSig1.addPMSignal(pmSig2);
  mtxSig2.addPMSignal(pmSig3);
  mtxSig2.addPMSignal(pmSig4);

  boost::property_tree::ptree calibTree;
  calibTree.put("scin." + std::to_string(123) + ".tof_correction", 1.5);
  calibTree.put("scin." + std::to_string(123) + ".eff_velocity", 11.0);

  auto hit = HitFinderTools::createHit(mtxSig1, mtxSig2, calibTree);

  BOOST_REQUIRE_EQUAL(hit.getScin().getID(), 123);
  BOOST_REQUIRE_CLOSE(hit.getSignalA().getTime(), 2.0, epsilon);
  BOOST_REQUIRE_CLOSE(hit.getSignalB().getTime(), 6.0, epsilon);
  BOOST_REQUIRE_CLOSE(hit.getTime(), 4.0 - 1.5, epsilon);
  BOOST_REQUIRE_CLOSE(hit.getTimeDiff(), 4.0, epsilon);
  BOOST_REQUIRE_CLOSE(hit.getToT(), 13.0, epsilon);
  BOOST_REQUIRE_CLOSE(hit.getEnergy(), 13.0, epsilon);
  // Before rotation (10.0, 10.0, 4*11/2 = 22.0)
  // Rotation around Y axis by 90 degrees: x,y,z -> z,y,-x
  BOOST_REQUIRE_CLOSE(hit.getPosX(), 22.0, epsilon);
  BOOST_REQUIRE_CLOSE(hit.getPosY(), 10.0, epsilon);
  BOOST_REQUIRE_CLOSE(hit.getPosZ(), -10.0, epsilon);

  BOOST_REQUIRE_CLOSE(hit.getQualityOfTime(), -1.0, epsilon);
  BOOST_REQUIRE_CLOSE(hit.getQualityOfTimeDiff(), -1.0, epsilon);
  BOOST_REQUIRE_CLOSE(hit.getQualityOfEnergy(), -1.0, epsilon);
  BOOST_REQUIRE_CLOSE(hit.getQualityOfToT(), -1.0, epsilon);
}

BOOST_AUTO_TEST_CASE(createDummyHit_test)
{
  JPetScin scin(123, 50.0, 19.0, 3.0, 10.0, 10.0, 10.0, 0.0, 90.0, 0.0);
  JPetMatrix mtxA(1, JPetMatrix::SideA);
  mtxA.setScin(scin);

  JPetMatrixSignal mtxSig;
  mtxSig.setMatrix(mtxA);
  mtxSig.setTime(2.0);

  JPetPM pm1A(1, "1A", 1);
  JPetPM pm2A(2, "2A", 2);
  JPetPM pm3A(3, "3A", 3);
  JPetPM pm4A(4, "4A", 4);
  pm1A.setMatrix(mtxA);
  pm2A.setMatrix(mtxA);
  pm3A.setMatrix(mtxA);
  pm4A.setMatrix(mtxA);

  JPetPMSignal pmSig1, pmSig2, pmSig3, pmSig4;
  pmSig1.setPM(pm1A);
  pmSig2.setPM(pm2A);
  pmSig3.setPM(pm3A);
  pmSig4.setPM(pm4A);
  pmSig1.setTime(1.0);
  pmSig2.setTime(3.0);
  pmSig3.setTime(5.0);
  pmSig4.setTime(7.0);
  pmSig1.setToT(10.0);
  pmSig2.setToT(14.0);
  pmSig3.setToT(12.0);
  pmSig4.setToT(16.0);

  mtxSig.addPMSignal(pmSig1);
  mtxSig.addPMSignal(pmSig2);
  mtxSig.addPMSignal(pmSig3);
  mtxSig.addPMSignal(pmSig4);

  auto hit = HitFinderTools::createDummyHit(mtxSig);
  BOOST_REQUIRE_EQUAL(hit.getScin().getID(), 123);
  BOOST_REQUIRE_CLOSE(hit.getSignalA().getTime(), 0.0, epsilon);
  BOOST_REQUIRE_CLOSE(hit.getSignalB().getTime(), 2.0, epsilon);
  BOOST_REQUIRE_CLOSE(hit.getTime(), 2.0, epsilon);
  BOOST_REQUIRE_CLOSE(hit.getTimeDiff(), 0.0, epsilon);
  BOOST_REQUIRE_CLOSE(hit.getToT(), 13.0, epsilon);
  BOOST_REQUIRE_CLOSE(hit.getEnergy(), 0.0, epsilon);
  BOOST_REQUIRE_CLOSE(hit.getPosX(), 10.0, epsilon);
  BOOST_REQUIRE_CLOSE(hit.getPosY(), 10.0, epsilon);
  BOOST_REQUIRE_CLOSE(hit.getPosZ(), 10.0, epsilon);
  BOOST_REQUIRE_CLOSE(hit.getQualityOfTime(), 0.0, epsilon);
  BOOST_REQUIRE_CLOSE(hit.getQualityOfTimeDiff(), 0.0, epsilon);
  BOOST_REQUIRE_CLOSE(hit.getQualityOfEnergy(), 0.0, epsilon);
  BOOST_REQUIRE_CLOSE(hit.getQualityOfToT(), 0.0, epsilon);
}

BOOST_AUTO_TEST_SUITE_END()
