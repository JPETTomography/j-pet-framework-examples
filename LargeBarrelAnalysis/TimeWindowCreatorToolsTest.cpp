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

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TimeWindowCreatorToolsTest

#include <boost/test/unit_test.hpp>
#include "TimeWindowCreatorTools.h"

BOOST_AUTO_TEST_SUITE(TimeWindowCreatorToolsTestSuite)

BOOST_AUTO_TEST_CASE(sortByValue_test)
{
  JPetPM pm1(1, "first");

  JPetSigCh sigCh1(JPetSigCh::Leading, 1.0);
  JPetSigCh sigCh2(JPetSigCh::Trailing, 4.0);
  JPetSigCh sigCh3(JPetSigCh::Leading, 2.0);
  JPetSigCh sigCh4(JPetSigCh::Leading, 6.0);
  JPetSigCh sigCh5(JPetSigCh::Trailing, 5.0);
  JPetSigCh sigCh6(JPetSigCh::Leading, 3.0);
  sigCh1.setPM(pm1);
  sigCh2.setPM(pm1);
  sigCh3.setPM(pm1);
  sigCh4.setPM(pm1);
  sigCh5.setPM(pm1);
  sigCh6.setPM(pm1);

  std::vector<JPetSigCh> sigChs;
  sigChs.push_back(sigCh1);
  sigChs.push_back(sigCh2);
  sigChs.push_back(sigCh3);
  sigChs.push_back(sigCh4);
  sigChs.push_back(sigCh5);
  sigChs.push_back(sigCh6);

  BOOST_REQUIRE_EQUAL(sigChs.at(0).getValue(), 1.0);
  BOOST_REQUIRE_EQUAL(sigChs.at(1).getValue(), 4.0);
  BOOST_REQUIRE_EQUAL(sigChs.at(2).getValue(), 2.0);
  BOOST_REQUIRE_EQUAL(sigChs.at(3).getValue(), 6.0);
  BOOST_REQUIRE_EQUAL(sigChs.at(4).getValue(), 5.0);
  BOOST_REQUIRE_EQUAL(sigChs.at(5).getValue(), 3.0);

  TimeWindowCreatorTools::sortByValue(sigChs);
  BOOST_REQUIRE_EQUAL(sigChs.at(0).getValue(), 1.0);
  BOOST_REQUIRE_EQUAL(sigChs.at(1).getValue(), 2.0);
  BOOST_REQUIRE_EQUAL(sigChs.at(2).getValue(), 3.0);
  BOOST_REQUIRE_EQUAL(sigChs.at(3).getValue(), 4.0);
  BOOST_REQUIRE_EQUAL(sigChs.at(4).getValue(), 5.0);
  BOOST_REQUIRE_EQUAL(sigChs.at(5).getValue(), 6.0);
}


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

  std::map<unsigned int, std::vector<double>> thresholdsMap;
  std::map<unsigned int, std::vector<double>> timeCalibrationMap;
  std::vector<double> calibVec;
  calibVec.push_back(22.0);
  calibVec.push_back(33.0);
  calibVec.push_back(44.0);
  timeCalibrationMap[123] = calibVec;

  auto sigCh = TimeWindowCreatorTools::generateSigCh(
    50.0, channel, timeCalibrationMap, thresholdsMap, JPetSigCh::Trailing, true
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

BOOST_AUTO_TEST_CASE(flagSigChs_test)
{
  JPetPM pm1(1, "first");

  JPetSigCh sigChA1(JPetSigCh::Leading, 10.0);
  JPetSigCh sigChA2(JPetSigCh::Trailing, 11.0);
  JPetSigCh sigChA3(JPetSigCh::Leading, 12.0);
  JPetSigCh sigChA4(JPetSigCh::Trailing, 13.0);
  sigChA1.setPM(pm1);
  sigChA2.setPM(pm1);
  sigChA3.setPM(pm1);
  sigChA4.setPM(pm1);

  JPetSigCh sigChB1(JPetSigCh::Leading, 20.0);
  JPetSigCh sigChB2(JPetSigCh::Trailing, 21.0);
  JPetSigCh sigChB3(JPetSigCh::Leading, 22.0);
  JPetSigCh sigChB4(JPetSigCh::Trailing, 23.0);
  sigChB1.setPM(pm1);
  sigChB2.setPM(pm1);
  sigChB3.setPM(pm1);
  sigChB4.setPM(pm1);

  JPetSigCh sigChC1(JPetSigCh::Leading, 30.0);
  JPetSigCh sigChC2(JPetSigCh::Trailing, 31.0);
  JPetSigCh sigChC3(JPetSigCh::Leading, 32.0);
  JPetSigCh sigChC4(JPetSigCh::Leading, 33.0);
  JPetSigCh sigChC5(JPetSigCh::Trailing, 34.0);
  JPetSigCh sigChC6(JPetSigCh::Leading, 35.0);
  JPetSigCh sigChC7(JPetSigCh::Leading, 36.0);
  JPetSigCh sigChC8(JPetSigCh::Leading, 37.0);
  JPetSigCh sigChC9(JPetSigCh::Trailing, 38.0);
  sigChC1.setPM(pm1);
  sigChC2.setPM(pm1);
  sigChC3.setPM(pm1);
  sigChC4.setPM(pm1);
  sigChC5.setPM(pm1);
  sigChC6.setPM(pm1);
  sigChC7.setPM(pm1);
  sigChC8.setPM(pm1);
  sigChC9.setPM(pm1);

  std::vector<JPetSigCh> thrSigCh;
  thrSigCh.push_back(sigChA1);
  thrSigCh.push_back(sigChA2);
  thrSigCh.push_back(sigChA3);
  thrSigCh.push_back(sigChA4);
  thrSigCh.push_back(sigChB1);
  thrSigCh.push_back(sigChB2);
  thrSigCh.push_back(sigChB3);
  thrSigCh.push_back(sigChB4);
  thrSigCh.push_back(sigChC1);
  thrSigCh.push_back(sigChC2);
  thrSigCh.push_back(sigChC3);
  thrSigCh.push_back(sigChC4);
  thrSigCh.push_back(sigChC5);
  thrSigCh.push_back(sigChC6);
  thrSigCh.push_back(sigChC7);
  thrSigCh.push_back(sigChC8);
  thrSigCh.push_back(sigChC9);

  JPetStatistics stats;
  TimeWindowCreatorTools::flagSigChs(thrSigCh, stats, false);
  BOOST_REQUIRE_EQUAL(thrSigCh.at(0).getRecoFlag(), JPetSigCh::Good);
  BOOST_REQUIRE_EQUAL(thrSigCh.at(1).getRecoFlag(), JPetSigCh::Good);
  BOOST_REQUIRE_EQUAL(thrSigCh.at(2).getRecoFlag(), JPetSigCh::Good);
  BOOST_REQUIRE_EQUAL(thrSigCh.at(3).getRecoFlag(), JPetSigCh::Good);
  BOOST_REQUIRE_EQUAL(thrSigCh.at(4).getRecoFlag(), JPetSigCh::Good);
  BOOST_REQUIRE_EQUAL(thrSigCh.at(5).getRecoFlag(), JPetSigCh::Good);
  BOOST_REQUIRE_EQUAL(thrSigCh.at(6).getRecoFlag(), JPetSigCh::Good);
  BOOST_REQUIRE_EQUAL(thrSigCh.at(7).getRecoFlag(), JPetSigCh::Good);
  BOOST_REQUIRE_EQUAL(thrSigCh.at(8).getRecoFlag(), JPetSigCh::Good);
  BOOST_REQUIRE_EQUAL(thrSigCh.at(9).getRecoFlag(), JPetSigCh::Good);
  BOOST_REQUIRE_EQUAL(thrSigCh.at(10).getRecoFlag(), JPetSigCh::Corrupted);
  BOOST_REQUIRE_EQUAL(thrSigCh.at(11).getRecoFlag(), JPetSigCh::Good);
  BOOST_REQUIRE_EQUAL(thrSigCh.at(12).getRecoFlag(), JPetSigCh::Good);
  BOOST_REQUIRE_EQUAL(thrSigCh.at(13).getRecoFlag(), JPetSigCh::Corrupted);
  BOOST_REQUIRE_EQUAL(thrSigCh.at(14).getRecoFlag(), JPetSigCh::Corrupted);
  BOOST_REQUIRE_EQUAL(thrSigCh.at(15).getRecoFlag(), JPetSigCh::Good);
  BOOST_REQUIRE_EQUAL(thrSigCh.at(16).getRecoFlag(), JPetSigCh::Good);
}

BOOST_AUTO_TEST_SUITE_END()
