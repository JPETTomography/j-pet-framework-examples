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
 *  @file EventCategorizerToolsTest.cpp
 */

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE EventCategorizerToolsTools

#include "../EventCategorizerTools.h"
#include <boost/test/unit_test.hpp>

auto epsilon = 0.0001;

BOOST_AUTO_TEST_SUITE(EventCategorizerToolsTestSuite)

BOOST_AUTO_TEST_CASE(checkToT_test)
{

  JPetPhysRecoHit hit;
  hit.setToT(57.0);

  BOOST_REQUIRE(EventCategorizerTools::checkToT(&hit, 50.0, 60.0));
  BOOST_REQUIRE(!EventCategorizerTools::checkToT(&hit, 50.0, 55.0));
  BOOST_REQUIRE(!EventCategorizerTools::checkToT(&hit, 59.0, 60.0));
  // BOOST_REQUIRE_CLOSE(EventCategorizerTools::mtxSignals.at(0).getTime(), 1.1, epsilon);
}

BOOST_AUTO_TEST_CASE(checkRelativeAngle_test)
{
  JPetBaseHit hit1;
  JPetBaseHit hit2;
  JPetBaseHit hit3;
  hit1.setPos(1.0, 1.0, 1.0);
  hit2.setPos(-1.0, -1.0, -1.0);
  hit3.setPos(0.9, 0.9, 0.9);
}

BOOST_AUTO_TEST_SUITE_END()
