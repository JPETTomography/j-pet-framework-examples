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

#include <iostream> //TODO DELETE

unsigned int SinogramCreatorTools::roundToNearesMultiplicity(float numberToRound, float accuracy)
{
  return std::floor((numberToRound / accuracy) + (accuracy / 2));
}

int SinogramCreatorTools::calculateAngle(float firstX, float firstY, float secondX, float secondY)
{
  float dx = firstX - secondX;
  float dy = firstY - secondY;
  float angle = 0.f;
  angle = std::atan2(dy, dx) * 180.f / M_PI;

  if (angle < 0.f)
    angle = angle + 180.f;
  int angleRound = std::round(angle);
  return angleRound >= 180 ? angleRound - 180 : angleRound;
}

float SinogramCreatorTools::calculateDistance(float firstX, float firstY, float secondX, float secondY)
{
  if (firstX > secondX)
  {
    std::swap(firstX, secondX);
    std::swap(firstY, secondY);
  }
  float norm = calculateNorm(firstX, firstY, secondX, secondY);
  if (std::abs(norm) < 0.000001f)
  {
    return 0;
  }
  return ((secondX * firstY) - (secondY * firstX)) / norm;
}

float SinogramCreatorTools::calculateNorm(float firstX, float firstY, float secondX, float secondY)
{
  return std::sqrt(std::pow((secondY - firstY), 2) + std::pow((secondX - firstX), 2));
}