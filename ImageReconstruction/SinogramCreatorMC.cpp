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

  getStatistics().createHistogram(new TH2I("reconstuction_histogram",
                                  "reconstuction histogram",
                                  std::ceil(fReconstructionLayerRadius * (1.f / fReconstructionDistanceAccuracy)) + 1, 0.f, fReconstructionLayerRadius,
                                  kReconstructionMaxAngle, 0, kReconstructionMaxAngle));

  getStatistics().createHistogram(new TH1F("pos_dis", "Position distance monte data", (fReconstructionLayerRadius) * 10 * 5, 0.f, fReconstructionLayerRadius));
  getStatistics().createHistogram(new TH1F("angle", "Position angle monte data", kReconstructionMaxAngle, 0, kReconstructionMaxAngle));

  getStatistics().getObject<TH2I>("reconstuction_histogram")->SetBit(TH2::kCanRebin);

  getStatistics().getObject<TH1F>("angle")->SetBit(TH1::kCanRebin);
  getStatistics().getObject<TH1F>("pos_dis")->SetBit(TH1::kCanRebin);

  std::ifstream in("sinogram_data.txt");

  float x1, y1, x2, y2, tmp1, tmp2;
  while (in.peek() != EOF) {
    in >> x1 >> y1 >> tmp1 >> x2 >> y2 >> tmp2;
    if (std::abs(tmp1) > 0.0005f || std::abs(tmp2) > 0.0005f)
      continue;
    hitsVector.push_back(std::make_pair(std::make_pair(x1, y1), std::make_pair(x2, y2)));
  }

  unsigned int currentValueInSinogram = 0; // holds current bin value of sinogram

  unsigned int maxDistanceNumber = std::ceil(fReconstructionLayerRadius * (1.f / fReconstructionDistanceAccuracy)) + 1;
  if (fSinogram == nullptr) {
    fSinogram = new SinogramResultType(maxDistanceNumber, (std::vector<unsigned int>(kReconstructionMaxAngle)));
  }
  for (unsigned int i = 0; i < hitsVector.size(); i++) {

    float firstX = hitsVector[i].first.first;
    float firstY = hitsVector[i].first.second;
    float secondX = hitsVector[i].second.first;
    float secondY = hitsVector[i].second.second; // copy positions

    float distance = (secondX * firstY) - (secondY * firstX);

    float norm = std::sqrt(std::pow((secondY - firstY), 2) + std::pow((secondX - firstX), 2));
    if (norm > 0.f) {
      distance /= norm;
      getStatistics().getObject<TH1F>("pos_dis")->Fill(distance);
      float angle = 0.f;
      if (std::abs(secondX - firstX) > EPSILON)
        angle = std::atan((firstY - secondY) / (secondX - firstX));

      if (distance > 0.f)
        angle = angle + M_PI / 2.f;
      else
        angle = angle + 3.f * M_PI / 2.f;

      if (angle > M_PI) {
        angle = angle - M_PI;
        distance = -distance;
      }
      angle *= 180.f / M_PI;
      getStatistics().getObject<TH1F>("angle")->Fill(angle);
      if (std::abs(distance) > EPSILON && std::abs(angle) > EPSILON) {
        int distanceRound = SinogramCreatorTools::roundToNearesMultiplicity(distance + ( fReconstructionLayerRadius / 2.), fReconstructionDistanceAccuracy); //displacement is calculated from center
        int thetaNumber = std::round(angle);
        if (distanceRound >= maxDistanceNumber) {
          std::cout << "Distance round: " << distanceRound << " distance: " << distance << " norm: " << norm << " x1: " << firstX << " y1: " << firstY << " x2: " << secondX << " y2: " << secondY << std::endl;
        }
        currentValueInSinogram = ++fSinogram->at(distanceRound).at(thetaNumber);
        if (currentValueInSinogram >= fMaxValueInSinogram)
          fMaxValueInSinogram = currentValueInSinogram;                                    // save max value of sinogram
        getStatistics().getObject<TH2I>("reconstuction_histogram")->Fill(distance + (fReconstructionLayerRadius / 2.), angle); //add to histogram
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

  if (isOptionSet(opts, kReconstructionDistanceAccuracy)) {
    fReconstructionDistanceAccuracy = getOptionAsFloat(opts, kReconstructionDistanceAccuracy);
  }
}
