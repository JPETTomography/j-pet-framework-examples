/**
 *  @copyright Copyright 2017 The J-PET Framework Authors. All rights reserved.
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
 *  @file SinogramCreatorMC.cpp
 */

#include "SinogramCreatorMC.h"
#include <TH2I.h>
#include <TH2F.h>
#include <TH3F.h>
#include <TH1I.h>
using namespace jpet_options_tools;

SinogramCreatorMC::SinogramCreatorMC(const char* name) : JPetUserTask(name) {}

SinogramCreatorMC::~SinogramCreatorMC() {}

bool SinogramCreatorMC::init()
{
  setUpOptions();
  fOutputEvents = new JPetTimeWindow("JPetEvent");

  getStatistics().createHistogram(new TH2I("reconstuction_histogram_monte",
                                  "reconstuction_histogram_monte",
                                  std::ceil(fReconstructionLayerRadius * 2 * (1.f / fReconstructionDistanceAccuracy)) + 1, -fReconstructionLayerRadius, fReconstructionLayerRadius,
                                  std::ceil((fReconstructionEndAngle - fReconstructionStartAngle) / fReconstructionAngleStep), fReconstructionStartAngle, fReconstructionEndAngle));
  getStatistics().createHistogram(new TH1F("rej_r_1", "Position of rejected data", fReconstructionLayerRadius * 2 * 10 * 5, -fReconstructionLayerRadius, fReconstructionLayerRadius));

  getStatistics().createHistogram(new TH1F("pos_r_1_monte", "Position r monte data", (fReconstructionLayerRadius + 15.) * 2 * 10 * 5, -fReconstructionLayerRadius, fReconstructionLayerRadius)); //42.5cm * 2 sides * 10mm * 5, 0.2mm acc;
  getStatistics().createHistogram(new TH1F("pos_r_2_monte", "Position r monte data", fReconstructionLayerRadius * 2 * 10 * 5, -fReconstructionLayerRadius, fReconstructionLayerRadius));
  getStatistics().createHistogram(new TH1F("pos_r_3_monte", "Position r monte data", fReconstructionLayerRadius * 2 * 10 * 5, -fReconstructionLayerRadius, fReconstructionLayerRadius));

  getStatistics().getObject<TH2I>("reconstuction_histogram_monte")->SetBit(TH2::kCanRebin);

  getStatistics().getObject<TH1F>("rej_r_1")->SetBit(TH1::kCanRebin);

  getStatistics().getObject<TH1F>("pos_r_1_monte")->SetBit(TH1::kCanRebin);
  getStatistics().getObject<TH1F>("pos_r_2_monte")->SetBit(TH1::kCanRebin);
  getStatistics().getObject<TH1F>("pos_r_3_monte")->SetBit(TH1::kCanRebin);

  std::ifstream in("sinogram_data.txt");

  float x1, y1, x2, y2;
  while (in.peek() != EOF) {
    in >> x1 >> y1 >> x2 >> y2;
    hitsVector.push_back(std::make_pair(std::make_pair(x1, y1), std::make_pair(x2, y2)));
  }

  unsigned int currentValueInSinogram = 0;                                                       // holds current bin value of sinogram
  float reconstructionAngleDiff = std::abs(fReconstructionEndAngle - fReconstructionStartAngle); // should be always positive

  unsigned int maxThetaNumber = std::ceil(reconstructionAngleDiff / fReconstructionAngleStep);
  unsigned int maxDistanceNumber = std::ceil(fReconstructionLayerRadius * 2 * (1.f / fReconstructionDistanceAccuracy)) + 1;
  if (fSinogram == nullptr) {
    fSinogram = new SinogramResultType(maxDistanceNumber, (std::vector<unsigned int>(maxThetaNumber)));
  }
  for (unsigned int i = 0; i < hitsVector.size(); i++) {
    for (float theta = fReconstructionStartAngle; theta < fReconstructionEndAngle; theta += fReconstructionAngleStep) {
      float x = fReconstructionLayerRadius * std::cos(theta * (M_PI / 180.f)); // calculate x,y positon of line with theta angle from line (0,0) = theta
      float y = fReconstructionLayerRadius * std::sin(theta * (M_PI / 180.f));

      std::pair<float, float> intersectionPointMonteCarlo = SinogramCreatorTools::lineIntersection(std::make_pair(-x, -y), std::make_pair(x, y), hitsVector[i].first, hitsVector[i].second); //find intersection point
      if (intersectionPointMonteCarlo.first != std::numeric_limits<float>::max() && intersectionPointMonteCarlo.second != std::numeric_limits<float>::max()) {
        // check is there is intersection point
        float distance = SinogramCreatorTools::length2D(intersectionPointMonteCarlo.first, intersectionPointMonteCarlo.second);
        getStatistics().getObject<TH1F>("pos_r_1_monte")->Fill(distance);
        if (distance >= fReconstructionLayerRadius) {
          // if distance is greather then our max reconstuction layer radius, it cant be placed in sinogram
          getStatistics().getObject<TH1F>("rej_r_1")->Fill(distance);
          continue;
        }
        getStatistics().getObject<TH1F>("pos_r_2_monte")->Fill(distance);
        if (intersectionPointMonteCarlo.first < 0.f)
          distance = -distance;
        getStatistics().getObject<TH1F>("pos_r_3_monte")->Fill(distance);
        int distanceRound = SinogramCreatorTools::roundToNearesMultiplicity(distance, fReconstructionDistanceAccuracy, fReconstructionLayerRadius);
        int thetaNumber = std::round(theta / fReconstructionAngleStep); // round because of floating point
        currentValueInSinogram = ++fSinogram->at(distanceRound).at(thetaNumber);
        if (currentValueInSinogram >= fMaxValueInSinogram)
          fMaxValueInSinogram = currentValueInSinogram;                                          // save max value of sinogram
        getStatistics().getObject<TH2I>("reconstuction_histogram_monte")->Fill(distance, theta); //add to histogram
      }
    }
  }

  return true;
}

bool SinogramCreatorMC::exec()
{
  return true;
}

bool SinogramCreatorMC::terminate()
{
  std::ofstream res(fOutFileName);
  res << "P2" << std::endl;
  res << (*fSinogram)[0].size() << " " << fSinogram->size() << std::endl;
  res << fMaxValueInSinogram << std::endl;
  for (unsigned int i = 0; i < fSinogram->size(); i++) {
    for (unsigned int j = 0; j < (*fSinogram)[0].size(); j++) {
      res << (*fSinogram)[i][j] << " ";
    }
    res << std::endl;
  }
  res.close();
  return true;
}

void SinogramCreatorMC::setUpOptions()
{
  auto opts = getOptions();
  if (isOptionSet(opts, kOutFileNameKey)) {
    fOutFileName = getOptionAsString(opts, kOutFileNameKey);
  }

  if (isOptionSet(opts, kReconstructionLayerRadiusKey)) {
    fReconstructionLayerRadius = getOptionAsFloat(opts, kReconstructionLayerRadiusKey);
  }

  if (isOptionSet(opts, kReconstructionStartAngle)) {
    fReconstructionStartAngle = getOptionAsFloat(opts, kReconstructionStartAngle);
  }

  if (isOptionSet(opts, kReconstructionEndAngle)) {
    fReconstructionEndAngle = getOptionAsFloat(opts, kReconstructionEndAngle);
  }

  if (isOptionSet(opts, kReconstructionDistanceAccuracy)) {
    fReconstructionDistanceAccuracy = getOptionAsFloat(opts, kReconstructionDistanceAccuracy);
  }

  if (isOptionSet(opts, kReconstructionAngleStep)) {
    fReconstructionAngleStep = getOptionAsFloat(opts, kReconstructionAngleStep);
  }
}
