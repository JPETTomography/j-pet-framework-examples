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
#include <math.h>

unsigned int SinogramCreatorTools::roundToNearesMultiplicity(float numberToRound, float accuracy) {
  return std::floor((numberToRound / accuracy) + (accuracy / 2));
}

std::pair<int, bool> SinogramCreatorTools::getAngleAndDistanceSign(float firstX, float firstY, float secondX, float secondY) {
  const float dx = (firstX + secondX) / 2.f;
  const float dy = (firstY + secondY) / 2.f;
  float angle = std::atan2(dy, dx) * (180.f / M_PI);
  const bool sign = angle < 0;
  angle = fmod(angle + 360.f, 180.f);
  int angleResult = std::round(angle);
  angleResult = angleResult % 180;

  return std::make_pair(angleResult, sign);
}

float SinogramCreatorTools::calculateDistance(float firstX, float firstY, float secondX, float secondY) {
  const float dx = (firstX + secondX) / 2.f;
  const float dy = (firstY + secondY) / 2.f;
  return std::sqrt(std::pow((dx), 2) + std::pow((dy), 2));
}

std::pair<int, int> SinogramCreatorTools::getSinogramRepresentation(float firstX, float firstY, float secondX, float secondY,
                                                                    float fMaxReconstructionLayerRadius, float fReconstructionDistanceAccuracy,
                                                                    int maxDistanceNumber, int kReconstructionMaxAngle) {
  float distance = SinogramCreatorTools::calculateDistance(firstX, firstY, secondX, secondY);
  const auto angle = SinogramCreatorTools::getAngleAndDistanceSign(firstX, firstY, secondX, secondY);
  if(angle.second)
    distance = -distance;

  int distanceRound = SinogramCreatorTools::roundToNearesMultiplicity(distance + fMaxReconstructionLayerRadius, fReconstructionDistanceAccuracy);
  if (distanceRound >= maxDistanceNumber || angle.first >= kReconstructionMaxAngle) {
    std::cout << "Distance or angle > then max, distance: " << distanceRound << " (max : " << maxDistanceNumber << ")"
              << " angle: " << angle.first << " (max: " << kReconstructionMaxAngle << ")" << std::endl;
  }
  if (distanceRound < 0)
    distanceRound = 0;
  return std::make_pair(distanceRound, angle.first);
}

std::tuple<float, float, float> SinogramCreatorTools::cart2sph(float x, float y, float z) {
  float theta = std::atan2(y, x);
  float phi = std::atan2(z, std::sqrt(x * x + y * y));
  float r = std::sqrt(x * x + y * y + z * z);
  return std::make_tuple(theta, phi, r);
}

std::tuple<float, float, float> SinogramCreatorTools::sph2cart(float theta, float phi, float r) {
  float x = r * std::cos(phi) * std::cos(theta);
  float y = r * std::cos(phi) * std::sin(theta);
  float z = r * std::sin(phi);
  return std::make_tuple(x, y, z);
}

float SinogramCreatorTools::calculateLORSlice(float x1, float y1, float z1, float t1, float x2, float y2, float z2, float t2) {
  float shiftX2 = x2 - x1;
  float shiftY2 = y2 - y1;
  float shiftZ2 = z2 - z1;

  float theta;
  float phi;
  float r;

  std::tie(theta, phi, r) = cart2sph(shiftX2, shiftY2, shiftZ2);

  const float speed_of_light = 0.0299792458f;

  float diffR = speed_of_light * (t2 - t1) / 2.f;

  float r0 = r / 2.f - diffR;

  float resultX;
  float resultY;
  float resultZ;

  std::tie(resultX, resultY, resultZ) = sph2cart(theta, phi, r0);

  resultZ += z1;

  return resultZ;
}
