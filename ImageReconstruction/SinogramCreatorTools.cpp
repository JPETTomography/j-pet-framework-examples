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
 *  @file SinogramCreatorTools.cpp
 */

#include "SinogramCreatorTools.h"
#include <iostream>

unsigned int SinogramCreatorTools::roundToNearesMultiplicity(float numberToRound, float accuracy)
{
  return std::floor((numberToRound / accuracy) + (accuracy / 2));
}

float SinogramCreatorTools::calculateAngle(float firstX, float firstY, float secondX, float secondY)
{
  float dx = firstX - secondX;
  float dy = firstY - secondY;
  float angle = 0.f;
  angle = std::atan2(dy, dx) * 180.f / M_PI;

  if (angle < 0.f)
    angle = angle + 180.f;
  return angle;
}

float SinogramCreatorTools::calculateDistance(float firstX, float firstY, float secondX, float secondY)
{
  float norm = calculateNorm(firstX, firstY, secondX, secondY);
  if (std::abs(norm) < 0.00001f)
  {
    return 0;
  }
  return ((secondX * firstY) - (secondY * firstX)) / norm;
}

float SinogramCreatorTools::calculateNorm(float firstX, float firstY, float secondX, float secondY)
{
  return std::sqrt(std::pow((secondY - firstY), 2) + std::pow((secondX - firstX), 2));
}

void SinogramCreatorTools::swapIfNeeded(float &firstX, float &firstY, float &secondX, float &secondY)
{
  if (firstX > secondX)
  {
    std::swap(firstX, secondX);
    std::swap(firstY, secondY);
  }
}

std::pair<int, int> SinogramCreatorTools::getSinogramRepresentation(float firstX, float firstY, float secondX, float secondY, float fMaxReconstructionLayerRadius, float fReconstructionDistanceAccuracy, int maxDistanceNumber, int kReconstructionMaxAngle)
{
  swapIfNeeded(firstX, firstY, secondX, secondY);
  float distance = SinogramCreatorTools::calculateDistance(firstX, firstY, secondX, secondY);
  const float angle = SinogramCreatorTools::calculateAngle(firstX, firstY, secondX, secondY);
  if (angle > 90.f)
    distance = -distance;
  int angleRound = std::round(angle);
  if (angleRound >= 180)
  {
    angleRound -= 180;
  }

  int distanceRound = SinogramCreatorTools::roundToNearesMultiplicity(distance + fMaxReconstructionLayerRadius, fReconstructionDistanceAccuracy);
  if (distanceRound >= maxDistanceNumber || angleRound >= kReconstructionMaxAngle)
  {
    std::cout << "Distance or angle > then max, distance: " << distanceRound << " angle: " << angleRound << std::endl;
  }
  if (distanceRound < 0)
    distanceRound = 0;
  return std::make_pair(distanceRound, angleRound);
}
