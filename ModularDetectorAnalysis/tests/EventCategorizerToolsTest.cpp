/**
 *  @copyright Copyright 2024 The J-PET Framework Authors. All rights reserved.
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
}

BOOST_AUTO_TEST_CASE(checkRelativeAngle_test)
{
  TVector3 pos1(5.0, 0.0, 0.0);
  TVector3 pos2(-5.0, 0.1, 0.0);
  TVector3 pos3(-5.0, 1.0, 0.0);

  BOOST_REQUIRE(EventCategorizerTools::checkRelativeAngles(pos1, pos2, 5.0));
  BOOST_REQUIRE(EventCategorizerTools::checkRelativeAngles(pos1, pos3, 20.0));
  BOOST_REQUIRE(!EventCategorizerTools::checkRelativeAngles(pos1, pos3, 1.0));
}

BOOST_AUTO_TEST_CASE(calculateDistance_test)
{
  JPetBaseHit hit1;
  JPetBaseHit hit2;
  hit1.setPos(1.0, 1.0, 1.0);
  hit2.setPos(-1.0, -1.0, -1.0);

  BOOST_REQUIRE_CLOSE(EventCategorizerTools::calculateDistance(&hit1, &hit2), sqrt(12.0), epsilon);
}

// TODO make more tests for annihilation point methods

BOOST_AUTO_TEST_SUITE_END()
