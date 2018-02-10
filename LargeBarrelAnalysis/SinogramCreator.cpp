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
    int maxThetaNumber = (reconstructionAngleDiff / reconstructionAngleStep) + 1;
    int maxDistanceNumber = std::floor(kReconstructionLayerRadius * (1.f / kReconstructionDistanceAccuracy)) + 1;
    fSinogram = new SinogramResultType(maxDistanceNumber, (std::vector<int>(maxThetaNumber)));
  }
  if (const auto& timeWindow = dynamic_cast<const JPetTimeWindow* const>(fEvent)) {
    unsigned int numberOfEventsInTimeWindow = timeWindow->getNumberOfEvents();
    for (unsigned int i = 0; i < numberOfEventsInTimeWindow; i++) {
      auto event = dynamic_cast<const JPetEvent&>(timeWindow->operator[](static_cast<int>(i)));
      auto hits = event.getHits();
      if (hits.size() == 2) {
        const auto& firstHit = hits[0];
        const auto& secondHit = hits[1];
        if (checkLayer(firstHit) && checkLayer(secondHit)) {
          for (float theta = kReconstructionStartAngle; theta < kReconstructionEndAngle; theta += reconstructionAngleStep) {
            float x = kReconstructionLayerRadius * std::cos(theta * (M_PI / 180));
            float y = kReconstructionLayerRadius * std::sin(theta * (M_PI / 180));
            std::pair<float, float> intersectionPoint = SinogramCreatorTools::lineIntersection(std::make_pair(0.f, 0.f), std::make_pair(x, y),
                std::make_pair(firstHit.getPosX(), firstHit.getPosY), std::make_pair(secondHit.getPosX(), secondHit.getPosY()));
            if (intersectionPoint.first != std::numeric_limits<float>::max() && intersectionPoint.second != std::numeric_limits<float>::max()) {
              float distance = SinogramCreatorTools::length2D(intersectionPoint.first, intersectionPoint.second);
              int distanceRound = std::floor((distance / kReconstructionDistanceAccuracy) + kReconstructionDistanceAccuracy);
              int thetaNumber = theta / reconstructionAngleStep;
              fSinogram->at(distanceRound).at(thetaNumber)++;
            }
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
