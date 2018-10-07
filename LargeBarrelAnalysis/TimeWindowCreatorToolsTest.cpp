/**
 *  @copyright Copyright 2018 The J-PET Framework Authors. All rights reserved.
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may find a copy of the License in the LICENCE file.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 *  @file TimeWindowCreatorToolsTest.cpp
 */

 #include "TimeWindowCreatorTools.h"

BOOST_AUTO_TEST_SUITE(TimeWindowCreatorToolsTestSuite)

BOOST_AUTO_TEST_CASE(generateSigCh_test)
{
  JPetFEB feb(1, true, "just great", "very nice front-end board", 1, 1, 4, 4);
  JPetTRB trb(2, 555, 333);
  std::pair<float, float> hvGains(23.4, 43.2);
  JPetPM pm(JPetPM::SideA, 23, 123, 321, hvGains, "average pm");

  JPetTOMBChannel channel(123);
  channel.setFEB(feb);
  channel.setTRB(trb);
  channel.setPM(pm);
  channel.setThreshold(34.5);
  channel.setLocalChannelNumber(1);

  std::map<unsigned int, std::vector<double>> timeCalibrationMap;
  std::vector<double> niceVector(22.0, 33.0, 44.0);
  timeCalibrationMap[123] = niceVector;

  auto sigCh = TimeWindowCreatorTools::generateSigCh(
    50.0, channel, JPetSigCh::Trailing, timeCalibrationMap, NULL, true
  );

  auto epsilon = 0.0001;
  BOOST_REQUIRE_EQUAL(sigCh.getType(), JPetSigCh::Trailing);
  BOOST_REQUIRE_EQUAL(sigCh.getPM().getID(), 23);
  BOOST_REQUIRE_EQUAL(sigCh.getFEB().getID(), 1);
  BOOST_REQUIRE_EQUAL(sigCh.getTRB().getID(), 2);
  BOOST_REQUIRE_EQUAL(sigCh.getDAQch(), 123);
  BOOST_REQUIRE_EQUAL(sigCh.getThresholdNumber(), 1);
  BOOST_REQUIRE_CLOSE(sigCh.getThreshold(), 34.5, epsilon);
  BOOST_REQUIRE_CLOSE(sigCh.getValue(), 1000.0*(50.0+22.0), epsilon);
}

BOOST_AUTO_TEST_CASE(sigChByPMTHR_test)
{
  JPetParamBank paramBank;

  JPetPM pm1(1, "first");
  paramBank.addPM(pm1);
  auto sigChA1 = JPetSigCh(JPetSigCh::Leading, 10.0);
  auto sigChA2 = JPetSigCh(JPetSigCh::Leading, 11.0);
  auto sigChA3 = JPetSigCh(JPetSigCh::Leading, 12.5);
  auto sigChA4 = JPetSigCh(JPetSigCh::Leading, 13.5);
  auto sigChA5 = JPetSigCh(JPetSigCh::Leading, 14.5);
  auto sigChA6 = JPetSigCh(JPetSigCh::Leading, 15.0);
  sigChA1.setThresholdNumber(1);
  sigChA2.setThresholdNumber(2);
  sigChA3.setThresholdNumber(3);
  sigChA4.setThresholdNumber(4);
  sigChA5.setThresholdNumber(2);
  sigChA6.setThresholdNumber(3);
  sigChA1.setPM(pm1);
  sigChA2.setPM(pm1);
  sigChA3.setPM(pm1);
  sigChA4.setPM(pm1);
  sigChA5.setPM(pm1);
  sigChA6.setPM(pm1);

  JPetPM pm2(2, "second");
  paramBank->addPM(pm2);
  auto sigChB1 = JPetSigCh(JPetSigCh::Leading, 1.0);
  auto sigChB2 = JPetSigCh(JPetSigCh::Leading, 2.0);
  auto sigChB3 = JPetSigCh(JPetSigCh::Leading, 3.5);
  sigChB1.setThresholdNumber(1);
  sigChB2.setThresholdNumber(1);
  sigChB3.setThresholdNumber(2);
  sigChB1.setPM(pm2);
  sigChB2.setPM(pm2);
  sigChB3.setPM(pm2);

  JPetPM pm3(3, "third");
  paramBank->addPM(pm3);
  auto sigChC1 = JPetSigCh(JPetSigCh::Leading, 5.0);
  auto sigChC2 = JPetSigCh(JPetSigCh::Leading, 6.0);
  auto sigChC3 = JPetSigCh(JPetSigCh::Leading, 7.0);
  sigChC1.setThresholdNumber(3);
  sigChC2.setThresholdNumber(3);
  sigChC3.setThresholdNumber(3);
  sigChC1.setPM(pm3);
  sigChC2.setPM(pm3);
  sigChC3.setPM(pm3);

  vector<JPetSigCh> inputSigChs;
  inputSigChs.push_back(sigChA1);
  inputSigChs.push_back(sigChA2);
  inputSigChs.push_back(sigChA3);
  inputSigChs.push_back(sigChA4);
  inputSigChs.push_back(sigChA5);
  inputSigChs.push_back(sigChA6);
  inputSigChs.push_back(sigChB1);
  inputSigChs.push_back(sigChB2);
  inputSigChs.push_back(sigChB3);
  inputSigChs.push_back(sigChC1);
  inputSigChs.push_back(sigChC2);
  inputSigChs.push_back(sigChC3);

  auto results = TimeWindowCreatorTools::sigChByPMTHR(inputSigChs, paramBank, 4);

  BOOST_REQUIRE_EQUAL(results.size(), 3);
  BOOST_REQUIRE_EQUAL(results.at(0).size(), 4);
  BOOST_REQUIRE_EQUAL(results.at(1).size(), 4);
  BOOST_REQUIRE_EQUAL(results.at(2).size(), 4);
  BOOST_REQUIRE_EQUAL(results.at(0).at(0).size(), 1);
  BOOST_REQUIRE_EQUAL(results.at(0).at(1).size(), 2);
  BOOST_REQUIRE_EQUAL(results.at(0).at(2).size(), 2);
  BOOST_REQUIRE_EQUAL(results.at(0).at(3).size(), 1);
  BOOST_REQUIRE_EQUAL(results.at(1).at(0).size(), 2);
  BOOST_REQUIRE_EQUAL(results.at(1).at(1).size(), 1);
  BOOST_REQUIRE_EQUAL(results.at(1).at(2).size(), 0);
  BOOST_REQUIRE_EQUAL(results.at(1).at(3).size(), 0);
  BOOST_REQUIRE_EQUAL(results.at(2).at(0).size(), 0);
  BOOST_REQUIRE_EQUAL(results.at(2).at(1).size(), 0);
  BOOST_REQUIRE_EQUAL(results.at(2).at(2).size(), 3);
  BOOST_REQUIRE_EQUAL(results.at(2).at(3).size(), 0);
}

BOOST_AUTO_TEST_CASE(flagTHRSigChs_test)
{
  JPetSigCh sigChA1(JPetSigCh::Leading, 10.0);
  JPetSigCh sigChA2(JPetSigCh::Trailing, 11.0);
  JPetSigCh sigChA3(JPetSigCh::Leading, 12.0);
  JPetSigCh sigChA4(JPetSigCh::Trailing, 13.0);

  JPetSigCh sigChB1(JPetSigCh::Leading, 20.0);
  JPetSigCh sigChB2(JPetSigCh::Trailing, 21.0);
  JPetSigCh sigChB3(JPetSigCh::Leading, 22.0);
  JPetSigCh sigChB4(JPetSigCh::Trailing, 23.0);

  JPetSigCh sigChC1(JPetSigCh::Leading, 30.0);
  JPetSigCh sigChC2(JPetSigCh::Trailing, 31.0);
  JPetSigCh sigChC3(JPetSigCh::Leading, 32.0);
  JPetSigCh sigChC4(JPetSigCh::Leading, 33.0);
  JPetSigCh sigChC5(JPetSigCh::Trailing, 34.0);
  JPetSigCh sigChC6(JPetSigCh::Leading, 35.0);
  JPetSigCh sigChC7(JPetSigCh::Leading, 36.0);
  JPetSigCh sigChC8(JPetSigCh::Leading, 37.0);
  JPetSigCh sigChC9(JPetSigCh::Trailing, 38.0);

  vector<JPetSigCh> thrSigCh;
  thrSigCh.push_back(sigChA1);
  thrSigCh.push_back(sigChA2);
  thrSigCh.push_back(sigChA3);
  thrSigCh.push_back(sigChA4);
  thrSigCh.push_back(sigChB1);
  thrSigCh.push_back(sigChB3);
  thrSigCh.push_back(sigChB4);
  thrSigCh.push_back(sigChB2);
  thrSigCh.push_back(sigChC5);
  thrSigCh.push_back(sigChC2);
  thrSigCh.push_back(sigChC9);
  thrSigCh.push_back(sigChC4);
  thrSigCh.push_back(sigChC8);
  thrSigCh.push_back(sigChC3);
  thrSigCh.push_back(sigChC6);
  thrSigCh.push_back(sigChC7);
  thrSigCh.push_back(sigChC1);

  JPetStatistics stats;
  auto result = TimeWindowCreatorTools::flagTHRSigChs(thrSigCh, stats, false);
  BOOST_REQUIRE_EQUAL(results.at(0), JPetSigCh::Good);
  BOOST_REQUIRE_EQUAL(results.at(1), JPetSigCh::Good);
  BOOST_REQUIRE_EQUAL(results.at(2), JPetSigCh::Good);
  BOOST_REQUIRE_EQUAL(results.at(3), JPetSigCh::Good);
  BOOST_REQUIRE_EQUAL(results.at(4), JPetSigCh::Good);
  BOOST_REQUIRE_EQUAL(results.at(5), JPetSigCh::Good);
  BOOST_REQUIRE_EQUAL(results.at(6), JPetSigCh::Good);
  BOOST_REQUIRE_EQUAL(results.at(7), JPetSigCh::Good);
  BOOST_REQUIRE_EQUAL(results.at(8), JPetSigCh::Good);
  BOOST_REQUIRE_EQUAL(results.at(9), JPetSigCh::Good);
  BOOST_REQUIRE_EQUAL(results.at(10), JPetSigCh::Corrupted);
  BOOST_REQUIRE_EQUAL(results.at(11), JPetSigCh::Good);
  BOOST_REQUIRE_EQUAL(results.at(12), JPetSigCh::Good);
  BOOST_REQUIRE_EQUAL(results.at(13), JPetSigCh::Corrupted);
  BOOST_REQUIRE_EQUAL(results.at(14), JPetSigCh::Corrupted);
  BOOST_REQUIRE_EQUAL(results.at(15), JPetSigCh::Good);
  BOOST_REQUIRE_EQUAL(results.at(16), JPetSigCh::Good);
}

BOOST_AUTO_TEST_SUITE_END()
