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
 *  @file SinogramCreator.cpp
 */

#include "SinogramCreator.h"

SinogramCreator::SinogramCreator(const char* name) : JPetUserTask(name) {}

SinogramCreator::~SinogramCreator() {}

bool SinogramCreator::init()
{
  auto opts = getOptions();
  fOutputEvents = new JPetTimeWindow("JPetEvent");

  return true;
}

bool SinogramCreator::exec()
{
  assert(kNumberOfScintillatorsInReconstructionLayer % 2 == 0); // number of scintillators always should be parzysta

  int numberOfScintillatorsInHalf = kNumberOfScintillatorsInReconstructionLayer / 2;
  float reconstructionAngleDiff = kReconstructionEndAngle - kReconstructionStartAngle;
  float reconstructionAngleStep = (reconstructionAngleDiff) / static_cast<float>(numberOfScintillatorsInHalf);
  if (fSinogram == nullptr) {
    fSinogram = new SinogramResultType((reconstructionAngleDiff / reconstructionAngleStep) + 1, (std::vector<int>(std::floor(kReconstructionLayerRadius * (1.f / kReconstructionDistanceAccuracy)) + 1)));
    std::cout << "thetaMAx: " << (reconstructionAngleDiff / reconstructionAngleStep) + 1 << " distanceMax: " << std::floor(kReconstructionLayerRadius * (1.f / kReconstructionDistanceAccuracy)) + 1 << std::endl;
  }
  if (auto& timeWindow = dynamic_cast<const JPetTimeWindow* const>(fEvent)) {
    unsigned int numberOfEventsInTimeWindow = timeWindow->getNumberOfEvents();
    for (unsigned int i = 0; i < numberOfEventsInTimeWindow; i++) {
      auto event = dynamic_cast<const JPetEvent&>(timeWindow->operator[](static_cast<int>(i)));
      auto hits = event.getHits();
      if (hits.size() == 2) {
        const auto& firstHit = hits[0];
        const auto& secondHit = hits[1];
        if (checkLayer(firstHit) && checkLayer(secondHit)) {
          for (float theta = kReconstructionStartAngle; theta < kReconstructionEndAngle; theta += reconstructionAngleStep) {
            float x = kReconstructionLayerRadius * std::cos(theta);
            float y = kReconstructionLayerRadius * std::sin(theta);
            std::cout << "x: " << x << " y: " << y << std::endl;
            float distance = SinogramCreatorTools::calculateDistanceFromCenter(0.f, 0.f, x, y, firstHit.getPosX(), secondHit.getPosY());
            int distanceRound = std::floor((distance / kReconstructionDistanceAccuracy) + kReconstructionDistanceAccuracy);
            int thetaNumber = theta / reconstructionAngleStep;
            std::cout << "thetaNumber: " << thetaNumber << " distanceRound: " << distanceRound << std::endl;
            fSinogram->at(thetaNumber).at(distanceRound)++;
          }
        }
      }
    }
  } else {
    ERROR("Returned event is not TimeWindow");
    return false;
  }
  return true;
}

bool SinogramCreator::checkLayer(const JPetHit& hit)
{
  return hit.getBarrelSlot().getLayer().getID() == 1;
}

bool SinogramCreator::terminate()
{
  const auto outFile = "sinogram.ppm";
  std::ofstream res(outFile);
  res << "P2" << std::endl;
  res << (*fSinogram)[0].size() << " " << fSinogram->size() << std::endl;
  res << "255" << std::endl;
  for (unsigned int i = 0; i < fSinogram->size(); i++) {
    for (unsigned int j = 0; j < (*fSinogram)[0].size(); j++) {
      res << static_cast<int>((*fSinogram)[i][j]) << " ";
    }
    res << std::endl;
  }
  res.close();
  return true;
}
