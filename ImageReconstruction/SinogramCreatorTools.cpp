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

unsigned int SinogramCreatorTools::roundToNearesMultiplicity(float numberToRound, float accuracy) {
  return std::floor((numberToRound / accuracy) + (accuracy / 2));
}

float SinogramCreatorTools::calculateAngle(float firstX, float firstY, float secondX, float secondY) {
  float dx = (firstX + secondX) / 2.f;
  float dy = (firstY + secondY) / 2.f;
  int angle = std::round(std::atan2(dy, dx) * 180.f / M_PI);
  angle = (angle + 360) % 180;

  return angle;
}

float SinogramCreatorTools::calculateDistance(float firstX, float firstY, float secondX, float secondY) {
  float norm = calculateNorm(firstX, firstY, secondX, secondY);
  if (std::abs(norm) < 0.00001f) {
    return 0.f;
  }
  float dy = (firstY + secondY) / 2.f; // calculate middle point between firstY and secondY
  float sign = dy < 0 ? -1.f : 1.f;
  return sign * (((secondX * firstY) - (secondY * firstX)) / norm);
}

float SinogramCreatorTools::calculateNorm(float firstX, float firstY, float secondX, float secondY) {
  return std::sqrt(std::pow((secondY - firstY), 2) + std::pow((secondX - firstX), 2));
}

void SinogramCreatorTools::swapIfNeeded(float& firstX, float& firstY, float& secondX, float& secondY) {
  if (firstX > secondX) {
    std::swap(firstX, secondX);
    std::swap(firstY, secondY);
  }
}

void SinogramCreatorTools::swapIfNeeded(float& firstX, float& firstY, float& firstZ, float& firstT, float& secondX, float& secondY, float& secondZ,
                                        float& secondT) {
  if (firstX > secondX) {
    std::swap(firstX, secondX);
    std::swap(firstY, secondY);
    std::swap(firstZ, secondZ);
    std::swap(firstT, secondT);
  }
}

std::pair<int, int> SinogramCreatorTools::getSinogramRepresentation(float firstX, float firstY, float secondX, float secondY,
                                                                    float fMaxReconstructionLayerRadius, float fReconstructionDistanceAccuracy,
                                                                    int maxDistanceNumber, int kReconstructionMaxAngle) {
  const float distance = SinogramCreatorTools::calculateDistance(firstX, firstY, secondX, secondY);
  const float angle = SinogramCreatorTools::calculateAngle(firstX, firstY, secondX, secondY);

  int distanceRound = SinogramCreatorTools::roundToNearesMultiplicity(distance + fMaxReconstructionLayerRadius, fReconstructionDistanceAccuracy);
  if (distanceRound >= maxDistanceNumber || angleRound >= kReconstructionMaxAngle) {
    std::cout << "Distance or angle > then max, distance: " << distanceRound << " (max : " << maxDistanceNumber << ")"
              << " angle: " << angleRound << " (max: " << kReconstructionMaxAngle << ")" << std::endl;
  }
  if (distanceRound < 0)
    distanceRound = 0;
  return std::make_pair(distanceRound, angleRound);
}

std::tuple<float, float, float> SinogramCreatorTools::cart2sph(float x, float y, float z) {
  float azimuth = std::atan2(y, x);
  float elevation = std::atan2(z, std::sqrt(x * x + y * y));
  float r = std::sqrt(x * x + y * y + z * z);
  return std::make_tuple(azimuth, elevation, r);
}

std::tuple<float, float, float> SinogramCreatorTools::sph2cart(float azimuth, float elevation, float r) {
  float x = r * std::cos(elevation) * std::cos(azimuth);
  float y = r * std::cos(elevation) * std::sin(azimuth);
  float z = r * std::sin(elevation);
  return std::make_tuple(x, y, z);
}

float SinogramCreatorTools::calculateLORSlice(float x1, float y1, float z1, float t1, float x2, float y2, float z2, float t2) {
  swapIfNeeded(x1, y1, z1, t1, x2, y2, z2, t2);
  float shiftX2 = x2 - x1;
  float shiftY2 = y2 - y1;
  float shiftZ2 = z2 - z1;

  float azimuth;
  float elevation;
  float r;

  std::tie(azimuth, elevation, r) = cart2sph(shiftX2, shiftY2, shiftZ2);

  const float speed_of_light = 0.0299792458f;

  float diffR = speed_of_light * (t2 - t1) / 2.f;

  float r0 = r / 2.f - diffR;

  float resultX;
  float resultY;
  float resultZ;

  std::tie(resultX, resultY, resultZ) = sph2cart(azimuth, elevation, r0);

  // resultX += x1;
  // resultY += y1;
  resultZ += z1;

  return resultZ;
}
