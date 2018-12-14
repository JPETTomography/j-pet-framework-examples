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
  JPetSigCh sigCh01(JPetSigCh::Leading, 10.0);
  JPetSigCh sigCh02(JPetSigCh::Trailing, 11.0);

  JPetSigCh sigCh03(JPetSigCh::Leading, 12.0);
  JPetSigCh sigCh04(JPetSigCh::Leading, 13.0);
  JPetSigCh sigCh05(JPetSigCh::Trailing, 14.0);

  JPetSigCh sigCh06(JPetSigCh::Leading, 15.0);
  JPetSigCh sigCh07(JPetSigCh::Trailing, 16.0);

  JPetSigCh sigCh08(JPetSigCh::Leading, 17.0);
  JPetSigCh sigCh09(JPetSigCh::Trailing, 18.0);

  JPetSigCh sigCh10(JPetSigCh::Leading, 19.0);
  JPetSigCh sigCh11(JPetSigCh::Trailing, 20.0);
  JPetSigCh sigCh12(JPetSigCh::Trailing, 21.0);

  JPetSigCh sigCh13(JPetSigCh::Leading, 22.0);
  JPetSigCh sigCh14(JPetSigCh::Trailing, 23.0);

  JPetSigCh sigCh15(JPetSigCh::Leading, 24.0);
  JPetSigCh sigCh16(JPetSigCh::Leading, 25.0);
  JPetSigCh sigCh17(JPetSigCh::Leading, 26.0);
  JPetSigCh sigCh18(JPetSigCh::Trailing, 27.0);

  JPetSigCh sigCh19(JPetSigCh::Leading, 28.0);
  JPetSigCh sigCh20(JPetSigCh::Trailing, 29.0);

  JPetSigCh sigCh21(JPetSigCh::Leading, 30.0);
  JPetSigCh sigCh22(JPetSigCh::Trailing, 31.0);
  JPetSigCh sigCh23(JPetSigCh::Trailing, 32.0);
  JPetSigCh sigCh24(JPetSigCh::Trailing, 33.0);
  JPetSigCh sigCh25(JPetSigCh::Trailing, 33.0);

  JPetSigCh sigCh26(JPetSigCh::Leading, 34.0);
  JPetSigCh sigCh27(JPetSigCh::Trailing, 35.0);

  JPetSigCh sigCh28(JPetSigCh::Leading, 34.0);

  JPetPM pm1(1, "first");
  sigCh01.setPM(pm1);
  sigCh02.setPM(pm1);
  sigCh03.setPM(pm1);
  sigCh04.setPM(pm1);
  sigCh05.setPM(pm1);
  sigCh06.setPM(pm1);
  sigCh07.setPM(pm1);
  sigCh08.setPM(pm1);
  sigCh09.setPM(pm1);
  sigCh10.setPM(pm1);
  sigCh11.setPM(pm1);
  sigCh12.setPM(pm1);
  sigCh13.setPM(pm1);
  sigCh14.setPM(pm1);
  sigCh15.setPM(pm1);
  sigCh16.setPM(pm1);
  sigCh17.setPM(pm1);
  sigCh18.setPM(pm1);
  sigCh19.setPM(pm1);
  sigCh20.setPM(pm1);
  sigCh21.setPM(pm1);
  sigCh22.setPM(pm1);
  sigCh23.setPM(pm1);
  sigCh24.setPM(pm1);
  sigCh25.setPM(pm1);
  sigCh26.setPM(pm1);
  sigCh27.setPM(pm1);
  sigCh28.setPM(pm1);

  std::vector<JPetSigCh> thrSigCh;
  thrSigCh.push_back(sigCh01);
  thrSigCh.push_back(sigCh02);
  thrSigCh.push_back(sigCh03);
  thrSigCh.push_back(sigCh04);
  thrSigCh.push_back(sigCh05);
  thrSigCh.push_back(sigCh06);
  thrSigCh.push_back(sigCh07);
  thrSigCh.push_back(sigCh08);
  thrSigCh.push_back(sigCh09);
  thrSigCh.push_back(sigCh10);
  thrSigCh.push_back(sigCh11);
  thrSigCh.push_back(sigCh12);
  thrSigCh.push_back(sigCh13);
  thrSigCh.push_back(sigCh14);
  thrSigCh.push_back(sigCh15);
  thrSigCh.push_back(sigCh16);
  thrSigCh.push_back(sigCh17);
  thrSigCh.push_back(sigCh18);
  thrSigCh.push_back(sigCh19);
  thrSigCh.push_back(sigCh20);
  thrSigCh.push_back(sigCh21);
  thrSigCh.push_back(sigCh22);
  thrSigCh.push_back(sigCh23);
  thrSigCh.push_back(sigCh24);
  thrSigCh.push_back(sigCh25);
  thrSigCh.push_back(sigCh26);
  thrSigCh.push_back(sigCh27);
  thrSigCh.push_back(sigCh28);

  JPetStatistics stats;
  TimeWindowCreatorTools::flagSigChs(thrSigCh, stats, false);
  BOOST_REQUIRE_EQUAL(thrSigCh.at(0).getRecoFlag(), JPetSigCh::Good);
  BOOST_REQUIRE_EQUAL(thrSigCh.at(1).getRecoFlag(), JPetSigCh::Good);

  BOOST_REQUIRE_EQUAL(thrSigCh.at(2).getRecoFlag(), JPetSigCh::Corrupted);
  BOOST_REQUIRE_EQUAL(thrSigCh.at(3).getRecoFlag(), JPetSigCh::Good);
  BOOST_REQUIRE_EQUAL(thrSigCh.at(4).getRecoFlag(), JPetSigCh::Good);

  BOOST_REQUIRE_EQUAL(thrSigCh.at(5).getRecoFlag(), JPetSigCh::Good);
  BOOST_REQUIRE_EQUAL(thrSigCh.at(6).getRecoFlag(), JPetSigCh::Good);

  BOOST_REQUIRE_EQUAL(thrSigCh.at(7).getRecoFlag(), JPetSigCh::Good);
  BOOST_REQUIRE_EQUAL(thrSigCh.at(8).getRecoFlag(), JPetSigCh::Good);

  BOOST_REQUIRE_EQUAL(thrSigCh.at(9).getRecoFlag(), JPetSigCh::Good);
  BOOST_REQUIRE_EQUAL(thrSigCh.at(10).getRecoFlag(), JPetSigCh::Good);
  BOOST_REQUIRE_EQUAL(thrSigCh.at(11).getRecoFlag(), JPetSigCh::Corrupted);

  BOOST_REQUIRE_EQUAL(thrSigCh.at(12).getRecoFlag(), JPetSigCh::Good);
  BOOST_REQUIRE_EQUAL(thrSigCh.at(13).getRecoFlag(), JPetSigCh::Good);

  BOOST_REQUIRE_EQUAL(thrSigCh.at(14).getRecoFlag(), JPetSigCh::Corrupted);
  BOOST_REQUIRE_EQUAL(thrSigCh.at(15).getRecoFlag(), JPetSigCh::Corrupted);
  BOOST_REQUIRE_EQUAL(thrSigCh.at(16).getRecoFlag(), JPetSigCh::Good);
  BOOST_REQUIRE_EQUAL(thrSigCh.at(17).getRecoFlag(), JPetSigCh::Good);

  BOOST_REQUIRE_EQUAL(thrSigCh.at(18).getRecoFlag(), JPetSigCh::Good);
  BOOST_REQUIRE_EQUAL(thrSigCh.at(19).getRecoFlag(), JPetSigCh::Good);

  BOOST_REQUIRE_EQUAL(thrSigCh.at(20).getRecoFlag(), JPetSigCh::Good);
  BOOST_REQUIRE_EQUAL(thrSigCh.at(21).getRecoFlag(), JPetSigCh::Good);
  BOOST_REQUIRE_EQUAL(thrSigCh.at(22).getRecoFlag(), JPetSigCh::Corrupted);
  BOOST_REQUIRE_EQUAL(thrSigCh.at(23).getRecoFlag(), JPetSigCh::Corrupted);
  BOOST_REQUIRE_EQUAL(thrSigCh.at(24).getRecoFlag(), JPetSigCh::Corrupted);

  BOOST_REQUIRE_EQUAL(thrSigCh.at(25).getRecoFlag(), JPetSigCh::Good);
  BOOST_REQUIRE_EQUAL(thrSigCh.at(26).getRecoFlag(), JPetSigCh::Good);

  // BOOST_REQUIRE_EQUAL(thrSigCh.at(27).getRecoFlag(), JPetSigCh::Good);
}

BOOST_AUTO_TEST_SUITE_END()
