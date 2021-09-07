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
 *  @file SignalFinderToolsTest.cpp
 */

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE SignalFinderToolsTest

#include "../SignalFinderTools.h"
#include <JPetParamBank/JPetParamBank.h>
#include <boost/test/unit_test.hpp>

auto epsilon = 0.0001;

BOOST_AUTO_TEST_SUITE(SignalFinderTestSuite)

BOOST_AUTO_TEST_CASE(getChannelSignalsByPM_nullPointer_test)
{
  auto results = SignalFinderTools::getChannelSignalsByPM(nullptr, false);
  BOOST_REQUIRE(results.empty());
}

BOOST_AUTO_TEST_CASE(getChannelSignalsByPM_Test)
{
  JPetPM pm1(1, "first", 1);
  JPetChannel channel1(1, 1, 50.0);
  channel1.setPM(pm1);

  JPetChannelSignal chSigA1(JPetRecoSignal::Good, JPetChannelSignal::Leading, 10.0);
  JPetChannelSignal chSigA2(JPetRecoSignal::Good, JPetChannelSignal::Leading, 11.0);
  JPetChannelSignal chSigA3(JPetRecoSignal::Good, JPetChannelSignal::Leading, 12.5);
  chSigA1.setChannel(channel1);
  chSigA2.setChannel(channel1);
  chSigA3.setChannel(channel1);

  JPetPM pm2(2, "second", 2);
  JPetChannel channel2(2, 2, 100.0);
  channel2.setPM(pm2);

  JPetChannelSignal chSigB1(JPetRecoSignal::Good, JPetChannelSignal::Leading, 1.0);
  JPetChannelSignal chSigB2(JPetRecoSignal::Corrupted, JPetChannelSignal::Leading, 2.0);
  JPetChannelSignal chSigB3(JPetRecoSignal::Good, JPetChannelSignal::Leading, 3.5);
  chSigB1.setChannel(channel2);
  chSigB2.setChannel(channel2);
  chSigB3.setChannel(channel2);

  JPetPM pm3(3, "third", 3);
  JPetChannel channel3(3, 3, 456.0);
  channel3.setPM(pm3);

  JPetChannelSignal chSigC1(JPetRecoSignal::Corrupted, JPetChannelSignal::Leading, 5.0);
  JPetChannelSignal chSigC2(JPetRecoSignal::Good, JPetChannelSignal::Leading, 6.0);
  chSigC1.setChannel(channel3);
  chSigC2.setChannel(channel3);

  JPetTimeWindow slot("JPetChannelSignal");
  slot.add<JPetChannelSignal>(chSigA1);
  slot.add<JPetChannelSignal>(chSigA2);
  slot.add<JPetChannelSignal>(chSigA3);
  slot.add<JPetChannelSignal>(chSigB1);
  slot.add<JPetChannelSignal>(chSigB2);
  slot.add<JPetChannelSignal>(chSigB3);
  slot.add<JPetChannelSignal>(chSigC1);
  slot.add<JPetChannelSignal>(chSigC2);

  auto results1 = SignalFinderTools::getChannelSignalsByPM(&slot, true);

  BOOST_REQUIRE_EQUAL(results1.size(), 3);
  BOOST_REQUIRE_EQUAL(results1[1].size(), 3);
  BOOST_REQUIRE_EQUAL(results1[2].size(), 3);
  BOOST_REQUIRE_EQUAL(results1[3].size(), 2);

  auto results2 = SignalFinderTools::getChannelSignalsByPM(&slot, false);

  BOOST_REQUIRE_EQUAL(results2.size(), 3);
  BOOST_REQUIRE_EQUAL(results2[1].size(), 3);
  BOOST_REQUIRE_EQUAL(results2[2].size(), 2);
  BOOST_REQUIRE_EQUAL(results2[3].size(), 1);
}

BOOST_AUTO_TEST_CASE(buildPMSignals_empty)
{
  std::vector<JPetChannelSignal> chSigByPM;
  JPetStatistics stats;
  boost::property_tree::ptree empty;

  auto results = SignalFinderTools::buildPMSignals(chSigByPM, 5.0, 5.0, 2, stats, false, SignalFinderTools::kThresholdTrapeze, empty);
  BOOST_REQUIRE(results.empty());
}

BOOST_AUTO_TEST_CASE(buildPMSignals_one_signal)
{
  JPetPM pm1(1, "first", 1);
  JPetChannel channel1(1, 1, 50.0);
  channel1.setPM(pm1);

  JPetChannelSignal chSig1(JPetChannelSignal::Leading, 10.0);
  JPetChannelSignal chSig2(JPetChannelSignal::Trailing, 15.0);
  chSig1.setChannel(channel1);
  chSig2.setChannel(channel1);

  std::vector<JPetChannelSignal> chSigVec;
  chSigVec.push_back(chSig1);
  chSigVec.push_back(chSig2);

  JPetStatistics stats;
  boost::property_tree::ptree empty;
  double chSigEdgeMaxTime = 5.0;
  double chSigLeadTrailMaxTime = 10.0;
  auto results =
      SignalFinderTools::buildPMSignals(chSigVec, chSigEdgeMaxTime, chSigLeadTrailMaxTime, 2, stats, false, SignalFinderTools::kSimplified, empty);

  BOOST_REQUIRE_EQUAL(results.size(), 1);
  BOOST_REQUIRE_CLOSE(results.at(0).getTime(), 10.0, epsilon);
  BOOST_REQUIRE_EQUAL(results.at(0).getLeadTrailPairs().size(), 1);
  BOOST_REQUIRE_CLOSE(results.at(0).getLeadTrailPairs().at(0).first.getTime(), 10.0, epsilon);
  BOOST_REQUIRE_CLOSE(results.at(0).getLeadTrailPairs().at(0).second.getTime(), 15.0, epsilon);
}

BOOST_AUTO_TEST_CASE(buildPMSignals_2)
{
  JPetPM pm1(1, "first", 1);
  JPetChannel channel1(1, 1, 50.0);
  JPetChannel channel2(2, 2, 150.0);
  channel1.setPM(pm1);
  channel2.setPM(pm1);

  JPetChannelSignal chSig1(JPetChannelSignal::Leading, 5.0);
  JPetChannelSignal chSig2(JPetChannelSignal::Leading, 7.0);
  JPetChannelSignal chSig3(JPetChannelSignal::Trailing, 10.0);
  JPetChannelSignal chSig4(JPetChannelSignal::Trailing, 8.0);

  chSig1.setChannel(channel1);
  chSig2.setChannel(channel2);
  chSig3.setChannel(channel1);
  chSig4.setChannel(channel2);

  std::vector<JPetChannelSignal> chSigFromSamePM;
  chSigFromSamePM.push_back(chSig1);
  chSigFromSamePM.push_back(chSig2);
  chSigFromSamePM.push_back(chSig3);
  chSigFromSamePM.push_back(chSig4);

  double chSigEdgeMaxTime = 5.;
  double chSigLeadTrailMaxTime = 10.;

  JPetStatistics stats;
  boost::property_tree::ptree empty;
  auto results = SignalFinderTools::buildPMSignals(chSigFromSamePM, chSigEdgeMaxTime, chSigLeadTrailMaxTime, 2, stats, false,
                                                   SignalFinderTools::kSimplified, empty);
  BOOST_REQUIRE_EQUAL(results.size(), 1);
  BOOST_REQUIRE_CLOSE(results.at(0).getTime(), 5.0, epsilon);
  BOOST_REQUIRE_EQUAL(results.at(0).getLeadTrailPairs().size(), 2);
  BOOST_REQUIRE_CLOSE(results.at(0).getLeadTrailPairs().at(0).first.getTime(), 5.0, epsilon);
  BOOST_REQUIRE_CLOSE(results.at(0).getLeadTrailPairs().at(0).second.getTime(), 10.0, epsilon);
  BOOST_REQUIRE_CLOSE(results.at(0).getLeadTrailPairs().at(1).first.getTime(), 7.0, epsilon);
  BOOST_REQUIRE_CLOSE(results.at(0).getLeadTrailPairs().at(1).second.getTime(), 8.0, epsilon);
}

BOOST_AUTO_TEST_CASE(findChannelSignalOnNextThr_empty)
{
  std::vector<JPetChannelSignal> empty;
  BOOST_REQUIRE_EQUAL(SignalFinderTools::findChannelSignalOnNextThr(1.0, 10.0, empty), -1);
}

BOOST_AUTO_TEST_CASE(findChannelSignalOnNextThr)
{
  std::vector<JPetChannelSignal> chSig1 = {JPetChannelSignal(JPetChannelSignal::Leading, 2), JPetChannelSignal(JPetChannelSignal::Leading, 5)};

  BOOST_REQUIRE_EQUAL(SignalFinderTools::findChannelSignalOnNextThr(1.0, 10.0, chSig1), 0);

  std::vector<JPetChannelSignal> chSig2 = {JPetChannelSignal(JPetChannelSignal::Leading, 7), JPetChannelSignal(JPetChannelSignal::Leading, 3)};
  BOOST_REQUIRE_EQUAL(SignalFinderTools::findChannelSignalOnNextThr(1.0, 10.0, chSig2), 0);

  std::vector<JPetChannelSignal> chSig3 = {JPetChannelSignal(JPetChannelSignal::Leading, 15), JPetChannelSignal(JPetChannelSignal::Leading, 3)};
  BOOST_REQUIRE_EQUAL(SignalFinderTools::findChannelSignalOnNextThr(1.0, 10.0, chSig3), 1);

  std::vector<JPetChannelSignal> chSig4 = {JPetChannelSignal(JPetChannelSignal::Leading, 15), JPetChannelSignal(JPetChannelSignal::Leading, 20)};
  BOOST_REQUIRE_EQUAL(SignalFinderTools::findChannelSignalOnNextThr(1.0, 10.0, chSig4), -1);
}

BOOST_AUTO_TEST_CASE(findTrailingChannelSignal_empty)
{
  JPetChannelSignal lead(JPetChannelSignal::Leading, 1);
  std::vector<JPetChannelSignal> trailings;
  BOOST_REQUIRE_EQUAL(SignalFinderTools::findTrailingChannelSignal(lead, 10.0, trailings), -1);
}

BOOST_AUTO_TEST_CASE(findTrailingChannelSignal)
{
  JPetPM pm1(1, "first", 1);
  JPetChannel channel1(1, 1, 50.0);

  JPetChannelSignal lead(JPetChannelSignal::Leading, 1.0);
  lead.setChannel(channel1);

  JPetChannelSignal trail1(JPetChannelSignal::Trailing, 2.0);
  JPetChannelSignal trail2(JPetChannelSignal::Trailing, 5.0);

  trail1.setChannel(channel1);
  trail2.setChannel(channel1);

  std::vector<JPetChannelSignal> trailings;
  trailings.push_back(trail1);
  trailings.push_back(trail2);

  BOOST_REQUIRE_EQUAL(SignalFinderTools::findTrailingChannelSignal(lead, 10.0, trailings), 0);

  JPetChannelSignal trail3(JPetChannelSignal::Trailing, 12.0);
  JPetChannelSignal trail4(JPetChannelSignal::Trailing, 2.0);

  trail3.setChannel(channel1);
  trail4.setChannel(channel1);

  std::vector<JPetChannelSignal> trailings2;
  trailings2.push_back(trail3);
  trailings2.push_back(trail4);
  BOOST_REQUIRE_EQUAL(SignalFinderTools::findTrailingChannelSignal(lead, 10.0, trailings2), 1);

  JPetChannelSignal trail5(JPetChannelSignal::Trailing, 2.0);
  JPetChannelSignal trail6(JPetChannelSignal::Trailing, 4.0);
  trail5.setChannel(channel1);
  trail6.setChannel(channel1);

  std::vector<JPetChannelSignal> trailings3;
  trailings3.push_back(trail5);
  trailings3.push_back(trail6);
  BOOST_REQUIRE_EQUAL(SignalFinderTools::findTrailingChannelSignal(lead, 10.0, trailings3), 0);
}

BOOST_AUTO_TEST_CASE(calculateToT_test)
{
  JPetPM pm1(1, "first", 1);

  double thr1 = 50.0, thr2 = 100.0;
  JPetChannel channel1(1, 1, 50.0);
  JPetChannel channel2(2, 2, 100.0);
  channel1.setPM(pm1);
  channel2.setPM(pm1);

  double t1 = 5.0, t2 = 25.0, t3 = 10.0, t4 = 20.0;
  JPetChannelSignal chSig1(JPetChannelSignal::Leading, t1);
  JPetChannelSignal chSig2(JPetChannelSignal::Trailing, t2);
  JPetChannelSignal chSig3(JPetChannelSignal::Leading, t3);
  JPetChannelSignal chSig4(JPetChannelSignal::Trailing, t4);
  chSig1.setChannel(channel1);
  chSig2.setChannel(channel1);
  chSig3.setChannel(channel2);
  chSig4.setChannel(channel2);

  JPetPMSignal pmSig;
  pmSig.setPM(pm1);
  pmSig.addLeadTrailPair(chSig1, chSig2);
  pmSig.addLeadTrailPair(chSig3, chSig4);

  boost::property_tree::ptree calibTree;
  boost::property_tree::ptree empty;

  // Testing calculation methods without calibration
  auto tot1 = SignalFinderTools::calculatePMSignalToT(pmSig, SignalFinderTools::kSimplified, empty);
  auto tot2 = SignalFinderTools::calculatePMSignalToT(pmSig, SignalFinderTools::kThresholdRectangular, empty);
  auto tot3 = SignalFinderTools::calculatePMSignalToT(pmSig, SignalFinderTools::kThresholdTrapeze, empty);

  double simple = (t2 - t1) + (t4 - t3);
  double rect = thr1 * (t2 - t1) + (thr2 - thr1) * (t4 - t3);
  double trapeze = thr1 * (t2 - t1) + 0.5 * (thr2 - thr1) * (t2 - t1 + t4 - t3);
  BOOST_REQUIRE_CLOSE(tot1, simple, epsilon);
  BOOST_REQUIRE_CLOSE(tot2, rect, epsilon);
  BOOST_REQUIRE_CLOSE(tot3, trapeze, epsilon);

  double factorA = 4.0, factorB = 28.0;
  calibTree.put("sipm." + std::to_string(1) + ".tot_factor_a", factorA);
  calibTree.put("sipm." + std::to_string(1) + ".tot_factor_b", factorB);

  // With calibrations
  auto tot4 = SignalFinderTools::calculatePMSignalToT(pmSig, SignalFinderTools::kSimplified, calibTree);
  auto tot5 = SignalFinderTools::calculatePMSignalToT(pmSig, SignalFinderTools::kThresholdRectangular, calibTree);
  auto tot6 = SignalFinderTools::calculatePMSignalToT(pmSig, SignalFinderTools::kThresholdTrapeze, calibTree);

  BOOST_REQUIRE_CLOSE(tot4, factorA * simple + factorB, epsilon);
  BOOST_REQUIRE_CLOSE(tot5, factorA * rect + factorB, epsilon);
  BOOST_REQUIRE_CLOSE(tot6, factorA * trapeze + factorB, epsilon);
}

BOOST_AUTO_TEST_SUITE_END()
