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
 *  @file SignalTransformerToolsTest.cpp
 */

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE SignalTransformerToolsTest

#include "../SignalTransformerTools.h"
#include <boost/test/unit_test.hpp>

double epsilon = 0.00001;

BOOST_AUTO_TEST_SUITE(SignalTransformerToolsTestSuite)

BOOST_AUTO_TEST_CASE(getRawSigMtxMap_test_empty)
{
  auto results = SignalTransformerTools::getPMSigMtxMap(nullptr);
  BOOST_REQUIRE(results.empty());
}

BOOST_AUTO_TEST_CASE(getPMSigMtxMap)
{
  JPetScin scin1(1, 50.0, 19.0, 3.0, -1.0, 0.0, 0.0, 45.0, 45.0, 45.0);
  JPetMatrix mtx1A(1, JPetMatrix::SideA);
  JPetMatrix mtx1B(2, JPetMatrix::SideB);
  mtx1A.setScin(scin1);
  mtx1B.setScin(scin1);
  JPetPM pm1A1(1, "1A1", 1);
  JPetPM pm1B1(2, "1B1", 1);
  pm1A1.setMatrix(mtx1A);
  pm1B1.setMatrix(mtx1B);

  JPetPMSignal sig11, sig12, sig13, sig14, sig15;
  sig11.setPM(pm1A1);
  sig12.setPM(pm1A1);
  sig13.setPM(pm1B1);
  sig14.setPM(pm1B1);
  sig15.setPM(pm1B1);

  JPetScin scin2(2, 50.0, 19.0, 3.0, -1.0, 0.0, 0.0, 45.0, 45.0, 45.0);
  JPetMatrix mtx2A(3, JPetMatrix::SideA);
  JPetMatrix mtx2B(4, JPetMatrix::SideB);
  mtx2A.setScin(scin2);
  mtx2B.setScin(scin2);
  JPetPM pm2A1(3, "2A1", 1);
  JPetPM pm2B1(4, "2B1", 1);
  JPetPM pm2A2(5, "2A2", 2);
  JPetPM pm2B2(6, "2B2", 2);
  pm2A1.setMatrix(mtx2A);
  pm2B1.setMatrix(mtx2B);
  pm2A2.setMatrix(mtx2A);
  pm2B2.setMatrix(mtx2B);

  JPetPMSignal sig21, sig22, sig23, sig24, sig25, sig26, sig27;
  sig21.setPM(pm2A1);
  sig22.setPM(pm2A1);
  sig23.setPM(pm2B1);
  sig24.setPM(pm2B1);
  sig25.setPM(pm2B1);
  sig26.setPM(pm2A2);
  sig27.setPM(pm2A2);

  JPetScin scin3(3, 50.0, 19.0, 3.0, -1.0, 0.0, 0.0, 45.0, 45.0, 45.0);
  JPetMatrix mtx3A(5, JPetMatrix::SideA);
  JPetMatrix mtx3B(6, JPetMatrix::SideB);
  mtx3A.setScin(scin3);
  mtx3B.setScin(scin3);
  JPetPM pm3A1(7, "3A1", 1);
  JPetPM pm3B1(8, "3B1", 1);
  JPetPM pm3A2(9, "3A2", 2);
  JPetPM pm3B2(10, "3B2", 2);
  JPetPM pm3A3(11, "3A3", 3);
  JPetPM pm3B3(12, "3B3", 3);
  pm3A1.setMatrix(mtx3A);
  pm3B1.setMatrix(mtx3B);
  pm3A2.setMatrix(mtx3A);
  pm3B2.setMatrix(mtx3B);
  pm3A3.setMatrix(mtx3A);
  pm3B3.setMatrix(mtx3B);

  JPetPMSignal sig31, sig32, sig33, sig34, sig35, sig36, sig37, sig38, sig39;
  sig31.setPM(pm3A1);
  sig32.setPM(pm3A1);
  sig33.setPM(pm3A1);
  sig34.setPM(pm3B1);
  sig35.setPM(pm3B1);
  sig36.setPM(pm3B2);
  sig37.setPM(pm3A3);
  sig38.setPM(pm3B3);
  sig39.setPM(pm3B3);

  JPetTimeWindow slot("JPetPMSignal");
  slot.add<JPetPMSignal>(sig11);
  slot.add<JPetPMSignal>(sig12);
  slot.add<JPetPMSignal>(sig13);
  slot.add<JPetPMSignal>(sig14);
  slot.add<JPetPMSignal>(sig15);
  slot.add<JPetPMSignal>(sig21);
  slot.add<JPetPMSignal>(sig22);
  slot.add<JPetPMSignal>(sig23);
  slot.add<JPetPMSignal>(sig24);
  slot.add<JPetPMSignal>(sig25);
  slot.add<JPetPMSignal>(sig26);
  slot.add<JPetPMSignal>(sig27);
  slot.add<JPetPMSignal>(sig31);
  slot.add<JPetPMSignal>(sig32);
  slot.add<JPetPMSignal>(sig33);
  slot.add<JPetPMSignal>(sig34);
  slot.add<JPetPMSignal>(sig35);
  slot.add<JPetPMSignal>(sig36);
  slot.add<JPetPMSignal>(sig37);
  slot.add<JPetPMSignal>(sig38);
  slot.add<JPetPMSignal>(sig39);

  auto results = SignalTransformerTools::getPMSigMtxMap(&slot);

  BOOST_REQUIRE_EQUAL(results.size(), 2);
  BOOST_REQUIRE_EQUAL(results.at(JPetMatrix::SideA).size(), 3);
  BOOST_REQUIRE_EQUAL(results.at(JPetMatrix::SideB).size(), 3);

  BOOST_REQUIRE_EQUAL(results.at(JPetMatrix::SideA).at(1).size(), 2);
  BOOST_REQUIRE_EQUAL(results.at(JPetMatrix::SideA).at(2).size(), 4);
  BOOST_REQUIRE_EQUAL(results.at(JPetMatrix::SideA).at(3).size(), 4);
  BOOST_REQUIRE_EQUAL(results.at(JPetMatrix::SideB).at(1).size(), 3);
  BOOST_REQUIRE_EQUAL(results.at(JPetMatrix::SideB).at(2).size(), 3);
  BOOST_REQUIRE_EQUAL(results.at(JPetMatrix::SideB).at(3).size(), 5);
}

BOOST_AUTO_TEST_CASE(calculateAverageTime_test)
{
  JPetScin scin(1, 50.0, 19.0, 3.0, -1.0, 0.0, 0.0, 45.0, 45.0, 45.0);
  JPetMatrix mtx(1, JPetMatrix::SideA);
  mtx.setScin(scin);
  JPetPM pm1(1, "1", 1);
  JPetPM pm2(2, "2", 2);
  JPetPM pm3(2, "3", 3);
  JPetPM pm4(2, "4", 4);
  pm1.setMatrix(mtx);
  pm2.setMatrix(mtx);
  pm3.setMatrix(mtx);
  pm4.setMatrix(mtx);

  JPetPMSignal pmSig1, pmSig2, pmSig3, pmSig4;
  pmSig1.setTime(1.0);
  pmSig2.setTime(2.0);
  pmSig3.setTime(3.0);
  pmSig4.setTime(4.0);
  pmSig1.setPM(pm1);
  pmSig2.setPM(pm2);
  pmSig3.setPM(pm3);
  pmSig4.setPM(pm4);

  JPetMatrixSignal mtxSig;
  mtxSig.addPMSignal(pmSig1);
  mtxSig.addPMSignal(pmSig2);
  mtxSig.addPMSignal(pmSig3);
  mtxSig.addPMSignal(pmSig4);

  boost::property_tree::ptree empty;
  boost::property_tree::ptree calibTree;
  auto result = SignalTransformerTools::calculateAverageTime(mtxSig, calibTree);
  BOOST_REQUIRE_CLOSE(result, 2.5, epsilon);
}

BOOST_AUTO_TEST_CASE(mergePMSignalsOnSide_test)
{
  JPetScin scin(1, 50.0, 19.0, 3.0, -1.0, 0.0, 0.0, 45.0, 45.0, 45.0);
  JPetMatrix mtx(1, JPetMatrix::SideA);
  mtx.setScin(scin);
  JPetPM pm1(1, "1", 1), pm2(2, "2", 2), pm3(3, "3", 3), pm4(4, "4", 4);
  pm1.setMatrix(mtx);
  pm2.setMatrix(mtx);
  pm3.setMatrix(mtx);
  pm4.setMatrix(mtx);

  JPetPMSignal sigA1, sigA2, sigA3, sigA4;
  JPetPMSignal sigB1, sigB2;
  JPetPMSignal sigC1, sigC2, sigC3;
  JPetPMSignal sigD1;
  JPetPMSignal sigE1, sigE2, sigE3, sigE4, sigE5, sigE6;

  sigA1.setPM(pm1);
  sigA2.setPM(pm2);
  sigA3.setPM(pm3);
  sigA4.setPM(pm4);

  sigB1.setPM(pm2);
  sigB2.setPM(pm3);

  sigC1.setPM(pm4);
  sigC2.setPM(pm2);
  sigC3.setPM(pm3);

  sigD1.setPM(pm2);

  sigE1.setPM(pm1);
  sigE2.setPM(pm2);
  sigE3.setPM(pm3);
  sigE4.setPM(pm4);
  sigE5.setPM(pm2);
  sigE6.setPM(pm3);

  sigA1.setTime(10.0);
  sigA2.setTime(11.0);
  sigA3.setTime(12.0);
  sigA4.setTime(13.0);

  sigB1.setTime(20.0);
  sigB2.setTime(21.0);

  sigC1.setTime(30.0);
  sigC2.setTime(31.0);
  sigC3.setTime(32.0);

  sigD1.setTime(31.5);

  sigE1.setTime(40.5);
  sigE2.setTime(40.9);
  sigE3.setTime(41.2);
  sigE4.setTime(41.8);
  sigE5.setTime(42.5);
  sigE6.setTime(42.9);

  std::vector<JPetPMSignal> pmSigVec;
  pmSigVec.push_back(sigA1);
  pmSigVec.push_back(sigA2);
  pmSigVec.push_back(sigA3);
  pmSigVec.push_back(sigA4);

  pmSigVec.push_back(sigB1);
  pmSigVec.push_back(sigB2);

  pmSigVec.push_back(sigC1);
  pmSigVec.push_back(sigC2);
  pmSigVec.push_back(sigC3);

  pmSigVec.push_back(sigD1);

  pmSigVec.push_back(sigE1);
  pmSigVec.push_back(sigE2);
  pmSigVec.push_back(sigE3);
  pmSigVec.push_back(sigE4);
  pmSigVec.push_back(sigE5);
  pmSigVec.push_back(sigE6);

  boost::property_tree::ptree calibTree;
  auto result = SignalTransformerTools::mergePMSignalsOnSide(pmSigVec, 5.0, calibTree);

  BOOST_REQUIRE_EQUAL(result.size(), 6);
  BOOST_REQUIRE_EQUAL(result.at(0).getPMSignals().size(), 4);
  BOOST_REQUIRE_EQUAL(result.at(1).getPMSignals().size(), 2);
  BOOST_REQUIRE_EQUAL(result.at(2).getPMSignals().size(), 3);
  BOOST_REQUIRE_EQUAL(result.at(3).getPMSignals().size(), 1);
  BOOST_REQUIRE_EQUAL(result.at(4).getPMSignals().size(), 4);
  BOOST_REQUIRE_EQUAL(result.at(5).getPMSignals().size(), 2);

  BOOST_REQUIRE_CLOSE(result.at(0).getPMSignals().at(1).getTime(), 10.0, epsilon);
  BOOST_REQUIRE_CLOSE(result.at(0).getPMSignals().at(2).getTime(), 11.0, epsilon);
  BOOST_REQUIRE_CLOSE(result.at(0).getPMSignals().at(3).getTime(), 12.0, epsilon);
  BOOST_REQUIRE_CLOSE(result.at(0).getPMSignals().at(4).getTime(), 13.0, epsilon);

  BOOST_REQUIRE_CLOSE(result.at(1).getPMSignals().at(2).getTime(), 20.0, epsilon);
  BOOST_REQUIRE_CLOSE(result.at(1).getPMSignals().at(3).getTime(), 21.0, epsilon);

  BOOST_REQUIRE_CLOSE(result.at(2).getPMSignals().at(4).getTime(), 30.0, epsilon);
  BOOST_REQUIRE_CLOSE(result.at(2).getPMSignals().at(2).getTime(), 31.0, epsilon);
  BOOST_REQUIRE_CLOSE(result.at(2).getPMSignals().at(3).getTime(), 32.0, epsilon);

  BOOST_REQUIRE_CLOSE(result.at(3).getPMSignals().at(2).getTime(), 31.5, epsilon);

  BOOST_REQUIRE_CLOSE(result.at(4).getPMSignals().at(1).getTime(), 40.5, epsilon);
  BOOST_REQUIRE_CLOSE(result.at(4).getPMSignals().at(2).getTime(), 40.9, epsilon);
  BOOST_REQUIRE_CLOSE(result.at(4).getPMSignals().at(3).getTime(), 41.2, epsilon);
  BOOST_REQUIRE_CLOSE(result.at(4).getPMSignals().at(4).getTime(), 41.8, epsilon);

  BOOST_REQUIRE_CLOSE(result.at(5).getPMSignals().at(2).getTime(), 42.5, epsilon);
  BOOST_REQUIRE_CLOSE(result.at(5).getPMSignals().at(3).getTime(), 42.9, epsilon);
}

BOOST_AUTO_TEST_SUITE_END()
