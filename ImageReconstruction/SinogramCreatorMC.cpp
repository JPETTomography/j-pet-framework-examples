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
                                  kReconstructionMaxAngle, 0, kReconstructionMaxAngle));

  getStatistics().createHistogram(new TH1F("pos_dis", "Position distance monte data", (fReconstructionLayerRadius) * 2 * 10 * 5, -fReconstructionLayerRadius, fReconstructionLayerRadius));
  getStatistics().createHistogram(new TH1F("angle", "Position angle monte data", kReconstructionMaxAngle, 0, kReconstructionMaxAngle));

  getStatistics().getObject<TH2I>("reconstuction_histogram_monte")->SetBit(TH2::kCanRebin);

  getStatistics().getObject<TH1F>("angle")->SetBit(TH1::kCanRebin);
  getStatistics().getObject<TH1F>("pos_dis")->SetBit(TH1::kCanRebin);

  std::ifstream in("sinogram_data.txt");

  float x1, y1, x2, y2;
  while (in.peek() != EOF) {
    in >> x1 >> y1 >> x2 >> y2;
    hitsVector.push_back(std::make_pair(std::make_pair(x1, y1), std::make_pair(x2, y2)));
  }

  unsigned int currentValueInSinogram = 0; // holds current bin value of sinogram

  unsigned int maxDistanceNumber = std::ceil(fReconstructionLayerRadius * 2 * (1.f / fReconstructionDistanceAccuracy)) + 1;
  if (fSinogram == nullptr) {
    fSinogram = new SinogramResultType(maxDistanceNumber, (std::vector<unsigned int>(kReconstructionMaxAngle)));
  }
  for (unsigned int i = 0; i < hitsVector.size(); i++) {
    float distance = hitsVector[i].second.first * hitsVector[i].first.second - hitsVector[i].second.second * hitsVector[i].first.first;
    float norm = std::sqrt((hitsVector[i].second.second - hitsVector[i].first.second) * (hitsVector[i].second.second - hitsVector[i].first.second)
                           + (hitsVector[i].second.first - hitsVector[i].first.first) * (hitsVector[i].second.first - hitsVector[i].first.first));
    if (norm > 0) {
      distance /= norm;
      getStatistics().getObject<TH1F>("pos_dis")->Fill(distance);
      float angle = 0.;
      if ((hitsVector[i].second.first - hitsVector[i].first.first) != 0)
        angle = std::atan((hitsVector[i].first.second - hitsVector[i].second.second) / (hitsVector[i].second.first - hitsVector[i].first.first));

      if (distance > 0)
        angle = angle + M_PI / 2.;
      else
        angle = angle + 3. * M_PI / 2.;

      if (angle > M_PI) {
        angle = angle - M_PI;
        distance = -distance;
      }
      angle *= 180. / M_PI;
      getStatistics().getObject<TH1F>("angle")->Fill(angle);
      if (std::abs(distance) > EPSILON && std::abs(angle) > EPSILON)
      {
        int distanceRound = SinogramCreatorTools::roundToNearesMultiplicity(distance, fReconstructionDistanceAccuracy, fReconstructionLayerRadius);
        int thetaNumber = std::round(angle);
        currentValueInSinogram = ++fSinogram->at(distanceRound).at(thetaNumber);
        if (currentValueInSinogram >= fMaxValueInSinogram)
          fMaxValueInSinogram = currentValueInSinogram;                                          // save max value of sinogram
        getStatistics().getObject<TH2I>("reconstuction_histogram_monte")->Fill(distance, angle); //add to histogram
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
