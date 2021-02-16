/**
 *  @copyright Copyright 2018 The J-PET Framework Authors. All rights reserved.
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
#define BOOST_TEST_MODULE EventCategorizerToolsTests
#include "../calibrationProgram/CalibrationTools.h"
#include <boost/test/unit_test.hpp>
#include <cstdlib> 
#include <ctime>

/// Accuracy for BOOST_REQUIRE_CLOSE comparisons
const double kEpsilon = 0.01;
const double kEpsilonIterator = 2;

BOOST_AUTO_TEST_SUITE(CalibrationProgramCheckSuite)

BOOST_AUTO_TEST_CASE(checkEstimateExtremumBin) {
  CalibrationTools calibTools;
  std::vector<double> values;
  unsigned numberOfPoints = 100;
  double mean1 = -2000, mean2 = 2000;
  double sigma = 200;
  double stepSize = 100;
  double tempValue;
  for (unsigned i=0; i<numberOfPoints; i++) {
    tempValue = exp(pow(-stepSize*numberOfPoints/2 + i*stepSize - mean1,2)/(2*pow(sigma,2)));
    tempValue -= exp(pow(-stepSize*numberOfPoints/2 + i*stepSize - mean2,2)/(2*pow(sigma,2)));
    values.push_back(tempValue);
  }
  unsigned extremumLeft = calibTools.EstimateExtremumBin(values, 0, 0, Side::Left);
  unsigned extremumRight = calibTools.EstimateExtremumBin(values, 0, 0, Side::Right);
  BOOST_REQUIRE_CLOSE((float)extremumLeft, 30, kEpsilonIterator);
  BOOST_REQUIRE_CLOSE((float)extremumRight, 70, kEpsilonIterator);
}

BOOST_AUTO_TEST_CASE(checkFindingPeak) {
  CalibrationTools calibTools;
  std::vector<double> values, arguments;
  values.push_back(-2);
  values.push_back(-1);
  values.push_back(0);
  values.push_back(1);
  values.push_back(2);
  arguments.push_back(0);
  arguments.push_back(1);
  arguments.push_back(2);
  arguments.push_back(3);
  arguments.push_back(4);
  Parameter result = calibTools.FindPeak(arguments, values, 0, 4);
  BOOST_REQUIRE_CLOSE(result.Value, 2, kEpsilon);
}

BOOST_AUTO_TEST_CASE(checkFindingMiddleAB) {
  CalibrationTools calibTools;
  TH1D* histo = new TH1D("TestDistributionAB", "Test distribution of AB", 500, -24750, 24250);
  unsigned numberOfEvents = 10000000;
  double uniformDistributionWidth = 5000; // 5 ns
  double resolutionSigma = 150; // 150 ps
  srand((unsigned)time(NULL));
  double tdiffAB, smearing, tempNumber2, tempNumber3;
  for (unsigned i=0; i<numberOfEvents; i++) {
    tdiffAB = uniformDistributionWidth*rand()/RAND_MAX;
    tempNumber2 = rand()/RAND_MAX;
    tempNumber3 = rand()/RAND_MAX;
    smearing = sqrt(-2*log(tempNumber2))*cos(2*M_PI*tempNumber3);
//Box-muller transformation
    histo -> Fill(tdiffAB + smearing*resolutionSigma);
  }
  double meanTemp = histo->GetMean(1);
  double rangeParameter = 4*histo->GetStdDev();
    
  Parameter resultLeft = calibTools.FindMiddle(histo, meanTemp - rangeParameter, meanTemp, Side::Left, "");
  Parameter resultRight = calibTools.FindMiddle(histo, meanTemp, meanTemp + rangeParameter, Side::Right, "");
  BOOST_REQUIRE_CLOSE(resultLeft.Value, -uniformDistributionWidth/2, kEpsilon*resolutionSigma);
  BOOST_REQUIRE_CLOSE(resultRight.Value, uniformDistributionWidth/2, kEpsilon*resolutionSigma);
  
  delete histo;
}

BOOST_AUTO_TEST_CASE(checkFindingMiddlePALS) {
  CalibrationTools calibTools;
  TH1D* histo = new TH1D("TestDistributionPALS", "Test PALS distribution", 500, -24750, 24250);
  unsigned numberOfEvents = 10000000;
  double exponentialMean = 2000;
  double resolutionSigma = 150; // 150 ps
  srand((unsigned)time(NULL));
  double tdiff, smearing, tempNumber2, tempNumber3;
  for (unsigned i=0; i<numberOfEvents; i++) {
    tdiff = -log(rand()/RAND_MAX)/exponentialMean;
    tempNumber2 = rand()/RAND_MAX;
    tempNumber3 = rand()/RAND_MAX;
    smearing = sqrt(-2*log(tempNumber2))*cos(2*M_PI*tempNumber3);
//Box-muller transformation
    histo -> Fill(tdiff + smearing*resolutionSigma);
  }
  double meanTemp = histo->GetMean(1);
  double rangeParameter = 4*histo->GetStdDev();
    
  Parameter results = calibTools.FindMiddle(histo, meanTemp - rangeParameter, meanTemp + rangeParameter, Side::MaxAnni, "");
  BOOST_REQUIRE_CLOSE(results.Value, 0., kEpsilon*resolutionSigma);
  
  delete histo;
}

BOOST_AUTO_TEST_SUITE_END()
