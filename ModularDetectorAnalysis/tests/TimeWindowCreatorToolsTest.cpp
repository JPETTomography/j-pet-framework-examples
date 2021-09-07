/**
 *  @copyright Copyright 2021 The J-PET Framework Authors. All rights reserved.
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

#include "../TimeWindowCreatorTools.h"
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(TimeWindowCreatorToolsTestSuite)

BOOST_AUTO_TEST_CASE(sortByTime_test)
{
  JPetPM pm1(1, "first", 1);
  JPetChannel channel1(1, 1, 50.0);
  channel1.setPM(pm1);

  JPetChannelSignal chSig1(JPetChannelSignal::Leading, 1.0);
  JPetChannelSignal chSig2(JPetChannelSignal::Trailing, 4.0);
  JPetChannelSignal chSig3(JPetChannelSignal::Leading, 2.0);
  JPetChannelSignal chSig4(JPetChannelSignal::Leading, 6.0);
  JPetChannelSignal chSig5(JPetChannelSignal::Trailing, 5.0);
  JPetChannelSignal chSig6(JPetChannelSignal::Leading, 3.0);

  chSig1.setChannel(channel1);
  chSig2.setChannel(channel1);
  chSig3.setChannel(channel1);
  chSig4.setChannel(channel1);
  chSig5.setChannel(channel1);
  chSig6.setChannel(channel1);

  std::vector<JPetChannelSignal> chSigs;
  chSigs.push_back(chSig1);
  chSigs.push_back(chSig2);
  chSigs.push_back(chSig3);
  chSigs.push_back(chSig4);
  chSigs.push_back(chSig5);
  chSigs.push_back(chSig6);

  BOOST_REQUIRE_EQUAL(chSigs.at(0).getTime(), 1.0);
  BOOST_REQUIRE_EQUAL(chSigs.at(1).getTime(), 4.0);
  BOOST_REQUIRE_EQUAL(chSigs.at(2).getTime(), 2.0);
  BOOST_REQUIRE_EQUAL(chSigs.at(3).getTime(), 6.0);
  BOOST_REQUIRE_EQUAL(chSigs.at(4).getTime(), 5.0);
  BOOST_REQUIRE_EQUAL(chSigs.at(5).getTime(), 3.0);

  TimeWindowCreatorTools::sortByTime(chSigs);
  BOOST_REQUIRE_EQUAL(chSigs.at(0).getTime(), 1.0);
  BOOST_REQUIRE_EQUAL(chSigs.at(1).getTime(), 2.0);
  BOOST_REQUIRE_EQUAL(chSigs.at(2).getTime(), 3.0);
  BOOST_REQUIRE_EQUAL(chSigs.at(3).getTime(), 4.0);
  BOOST_REQUIRE_EQUAL(chSigs.at(4).getTime(), 5.0);
  BOOST_REQUIRE_EQUAL(chSigs.at(5).getTime(), 6.0);
}

BOOST_AUTO_TEST_CASE(generateChannelSignal_test)
{
  JPetChannel channel1(23, 1, 34.5);
  auto chSig = TimeWindowCreatorTools::generateChannelSignal(11.0, channel1, JPetChannelSignal::Trailing, 1000.0);
  auto epsilon = 0.0001;
  BOOST_REQUIRE_EQUAL(chSig.getEdgeType(), JPetChannelSignal::Trailing);
  BOOST_REQUIRE_EQUAL(chSig.getChannel().getID(), 23);
  BOOST_REQUIRE_EQUAL(chSig.getChannel().getThresholdNumber(), 1);
  BOOST_REQUIRE_CLOSE(chSig.getChannel().getThresholdValue(), 34.5, epsilon);
  BOOST_REQUIRE_CLOSE(chSig.getTime(), 1000.0 * 11.0 - 1000.0, epsilon);
}

BOOST_AUTO_TEST_CASE(flagChannelSignals_test)
{
  JPetChannelSignal chSig00(JPetChannelSignal::Leading, 10.0);
  JPetChannelSignal chSig01(JPetChannelSignal::Trailing, 11.0);

  JPetChannelSignal chSig02(JPetChannelSignal::Leading, 12.0);
  JPetChannelSignal chSig03(JPetChannelSignal::Leading, 13.0);
  JPetChannelSignal chSig04(JPetChannelSignal::Trailing, 14.0);

  JPetChannelSignal chSig05(JPetChannelSignal::Leading, 15.0);
  JPetChannelSignal chSig06(JPetChannelSignal::Trailing, 16.0);

  JPetChannelSignal chSig07(JPetChannelSignal::Leading, 17.0);
  JPetChannelSignal chSig08(JPetChannelSignal::Trailing, 18.0);

  JPetChannelSignal chSig09(JPetChannelSignal::Leading, 19.0);
  JPetChannelSignal chSig10(JPetChannelSignal::Trailing, 20.0);
  JPetChannelSignal chSig11(JPetChannelSignal::Trailing, 21.0);

  JPetChannelSignal chSig12(JPetChannelSignal::Leading, 22.0);
  JPetChannelSignal chSig13(JPetChannelSignal::Trailing, 23.0);

  JPetChannelSignal chSig14(JPetChannelSignal::Leading, 24.0);
  JPetChannelSignal chSig15(JPetChannelSignal::Leading, 25.0);
  JPetChannelSignal chSig16(JPetChannelSignal::Leading, 26.0);
  JPetChannelSignal chSig17(JPetChannelSignal::Trailing, 27.0);

  JPetChannelSignal chSig18(JPetChannelSignal::Leading, 28.0);
  JPetChannelSignal chSig19(JPetChannelSignal::Trailing, 29.0);

  JPetChannelSignal chSig20(JPetChannelSignal::Leading, 30.0);
  JPetChannelSignal chSig21(JPetChannelSignal::Trailing, 31.0);
  JPetChannelSignal chSig22(JPetChannelSignal::Trailing, 32.0);
  JPetChannelSignal chSig23(JPetChannelSignal::Trailing, 33.0);
  JPetChannelSignal chSig24(JPetChannelSignal::Trailing, 33.0);

  JPetChannelSignal chSig25(JPetChannelSignal::Leading, 34.0);
  JPetChannelSignal chSig26(JPetChannelSignal::Trailing, 35.0);

  JPetChannelSignal chSig27(JPetChannelSignal::Leading, 36.0);

  JPetPM pm1(1, "first", 1);
  JPetChannel channel1(1, 1, 50.0);
  channel1.setPM(pm1);

  chSig00.setChannel(channel1);
  chSig01.setChannel(channel1);
  chSig02.setChannel(channel1);
  chSig03.setChannel(channel1);
  chSig04.setChannel(channel1);
  chSig05.setChannel(channel1);
  chSig06.setChannel(channel1);
  chSig07.setChannel(channel1);
  chSig08.setChannel(channel1);
  chSig09.setChannel(channel1);
  chSig10.setChannel(channel1);
  chSig11.setChannel(channel1);
  chSig12.setChannel(channel1);
  chSig13.setChannel(channel1);
  chSig14.setChannel(channel1);
  chSig15.setChannel(channel1);
  chSig16.setChannel(channel1);
  chSig17.setChannel(channel1);
  chSig18.setChannel(channel1);
  chSig19.setChannel(channel1);
  chSig20.setChannel(channel1);
  chSig21.setChannel(channel1);
  chSig22.setChannel(channel1);
  chSig23.setChannel(channel1);
  chSig24.setChannel(channel1);
  chSig25.setChannel(channel1);
  chSig26.setChannel(channel1);
  chSig27.setChannel(channel1);

  std::vector<JPetChannelSignal> thrChSig;
  thrChSig.push_back(chSig00);
  thrChSig.push_back(chSig01);
  thrChSig.push_back(chSig02);
  thrChSig.push_back(chSig03);
  thrChSig.push_back(chSig04);
  thrChSig.push_back(chSig05);
  thrChSig.push_back(chSig06);
  thrChSig.push_back(chSig07);
  thrChSig.push_back(chSig08);
  thrChSig.push_back(chSig09);
  thrChSig.push_back(chSig10);
  thrChSig.push_back(chSig11);
  thrChSig.push_back(chSig12);
  thrChSig.push_back(chSig13);
  thrChSig.push_back(chSig14);
  thrChSig.push_back(chSig15);
  thrChSig.push_back(chSig16);
  thrChSig.push_back(chSig17);
  thrChSig.push_back(chSig18);
  thrChSig.push_back(chSig19);
  thrChSig.push_back(chSig20);
  thrChSig.push_back(chSig21);
  thrChSig.push_back(chSig22);
  thrChSig.push_back(chSig23);
  thrChSig.push_back(chSig24);
  thrChSig.push_back(chSig25);
  thrChSig.push_back(chSig26);
  thrChSig.push_back(chSig27);

  JPetStatistics stats;
  TimeWindowCreatorTools::flagChannelSignals(thrChSig, stats, false);
  BOOST_REQUIRE_EQUAL(thrChSig.at(0).getRecoFlag(), JPetRecoSignal::Good);
  BOOST_REQUIRE_EQUAL(thrChSig.at(1).getRecoFlag(), JPetRecoSignal::Good);

  BOOST_REQUIRE_EQUAL(thrChSig.at(2).getRecoFlag(), JPetRecoSignal::Corrupted);
  BOOST_REQUIRE_EQUAL(thrChSig.at(3).getRecoFlag(), JPetRecoSignal::Good);
  BOOST_REQUIRE_EQUAL(thrChSig.at(4).getRecoFlag(), JPetRecoSignal::Good);

  BOOST_REQUIRE_EQUAL(thrChSig.at(5).getRecoFlag(), JPetRecoSignal::Good);
  BOOST_REQUIRE_EQUAL(thrChSig.at(6).getRecoFlag(), JPetRecoSignal::Good);

  BOOST_REQUIRE_EQUAL(thrChSig.at(7).getRecoFlag(), JPetRecoSignal::Good);
  BOOST_REQUIRE_EQUAL(thrChSig.at(8).getRecoFlag(), JPetRecoSignal::Good);

  BOOST_REQUIRE_EQUAL(thrChSig.at(9).getRecoFlag(), JPetRecoSignal::Good);
  BOOST_REQUIRE_EQUAL(thrChSig.at(10).getRecoFlag(), JPetRecoSignal::Good);
  BOOST_REQUIRE_EQUAL(thrChSig.at(11).getRecoFlag(), JPetRecoSignal::Corrupted);

  BOOST_REQUIRE_EQUAL(thrChSig.at(12).getRecoFlag(), JPetRecoSignal::Good);
  BOOST_REQUIRE_EQUAL(thrChSig.at(13).getRecoFlag(), JPetRecoSignal::Good);

  BOOST_REQUIRE_EQUAL(thrChSig.at(14).getRecoFlag(), JPetRecoSignal::Corrupted);
  BOOST_REQUIRE_EQUAL(thrChSig.at(15).getRecoFlag(), JPetRecoSignal::Corrupted);
  BOOST_REQUIRE_EQUAL(thrChSig.at(16).getRecoFlag(), JPetRecoSignal::Good);
  BOOST_REQUIRE_EQUAL(thrChSig.at(17).getRecoFlag(), JPetRecoSignal::Good);

  BOOST_REQUIRE_EQUAL(thrChSig.at(18).getRecoFlag(), JPetRecoSignal::Good);
  BOOST_REQUIRE_EQUAL(thrChSig.at(19).getRecoFlag(), JPetRecoSignal::Good);

  BOOST_REQUIRE_EQUAL(thrChSig.at(20).getRecoFlag(), JPetRecoSignal::Good);
  BOOST_REQUIRE_EQUAL(thrChSig.at(21).getRecoFlag(), JPetRecoSignal::Good);
  BOOST_REQUIRE_EQUAL(thrChSig.at(22).getRecoFlag(), JPetRecoSignal::Corrupted);
  BOOST_REQUIRE_EQUAL(thrChSig.at(23).getRecoFlag(), JPetRecoSignal::Corrupted);
  BOOST_REQUIRE_EQUAL(thrChSig.at(24).getRecoFlag(), JPetRecoSignal::Corrupted);

  BOOST_REQUIRE_EQUAL(thrChSig.at(25).getRecoFlag(), JPetRecoSignal::Good);
  BOOST_REQUIRE_EQUAL(thrChSig.at(26).getRecoFlag(), JPetRecoSignal::Good);

  BOOST_REQUIRE_EQUAL(thrChSig.at(27).getRecoFlag(), JPetRecoSignal::Good);
}

BOOST_AUTO_TEST_SUITE_END()
