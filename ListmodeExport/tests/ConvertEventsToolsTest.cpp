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
 *  @file ConvertEventsToolsTest.cpp
 */

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE EventCategorizerToolsTools

#include "../ConvertEventsTools.h"
#include <boost/test/unit_test.hpp>

auto epsilon = 0.0001;

BOOST_AUTO_TEST_SUITE(EventCategorizerToolsTestSuite)

BOOST_AUTO_TEST_CASE(readLUTFile_test)
{
  // TODO put the LUT file in unit test files
  std::string lutFileName = "../scripts/test_conversion.lut";
  auto lutGeom = ConvertEventsTools::readLUTFile(lutFileName, 40);
  BOOST_REQUIRE_EQUAL(lutGeom.size(), 40);
}

BOOST_AUTO_TEST_CASE(getCrystalID_test)
{
  TVector3 crystal1(5.0, 5.0, 5.0);
  TVector3 crystal2(10.0, 10.0, 10.0);
  TVector3 crystal3(20.0, 20.0, 20.0);

  std::vector<TVector3> crystals;
  crystals.push_back(crystal1);
  crystals.push_back(crystal2);
  crystals.push_back(crystal3);

  TVector3 hitPos1(5.1, 5.1, 5.1);
  TVector3 hitPos2(5.5, 5.5, 5.5);
  TVector3 hitPos3(10.1, 10.1, 10.1);
  TVector3 hitPos4(22.2, 22.2, 22.2);

  BOOST_REQUIRE_EQUAL(ConvertEventsTools::getCrystalID(hitPos1, crystals, 1.0), 0);
  BOOST_REQUIRE_EQUAL(ConvertEventsTools::getCrystalID(hitPos2, crystals, 1.0), 0);
  BOOST_REQUIRE_EQUAL(ConvertEventsTools::getCrystalID(hitPos2, crystals, 0.1), -1);
  BOOST_REQUIRE_EQUAL(ConvertEventsTools::getCrystalID(hitPos3, crystals, 1.0), 1);
  BOOST_REQUIRE_EQUAL(ConvertEventsTools::getCrystalID(hitPos4, crystals, 5.0), 2);
  BOOST_REQUIRE_EQUAL(ConvertEventsTools::getCrystalID(hitPos4, crystals, 1.0), -1);
}

BOOST_AUTO_TEST_SUITE_END()
