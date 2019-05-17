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
#include "JPetLoggerInclude.h"
#include <iostream>
#include <math.h>

unsigned int SinogramCreatorTools::roundToNearesMultiplicity(double numberToRound, double accuracy)
{
  return std::floor((numberToRound / accuracy) + (accuracy / 2.));
}

std::pair<int, float> SinogramCreatorTools::getAngleAndDistance(float firstX, float firstY, float secondX, float secondY)
{
  float dx = (secondX - firstX);
  float dy = (secondY - firstY);
  if (std::abs(dx) < kEPSILON) return std::make_pair(0, firstX);
  if (std::abs(dy) < kEPSILON) return std::make_pair(90, firstY);
  float slope = dy / dx;
  float perpendicularSlope = -(1 / slope);
  float d = firstY - (slope * firstX);
  float x = -d / (perpendicularSlope - slope);
  float y = perpendicularSlope * x;

  float angle = std::atan2(y, x) * (180.f / M_PI);
  const bool sign = y < 0.f;
  angle = fmod(angle + 360.f, 180.f);
  int angleResult = std::round(angle);
  angleResult = angleResult % 180;
  float distance = std::sqrt(std::pow((x), 2) + std::pow((y), 2));
  if (!sign) distance = -distance;
  return std::make_pair(angleResult, distance);
}

std::pair<int, int> SinogramCreatorTools::getSinogramRepresentation(float firstX, float firstY, float secondX, float secondY,
                                                                    float fMaxReconstructionLayerfSingleLayerRadius, float fReconstructionDistanceAccuracy,
                                                                    int maxDistanceNumber, int kReconstructionMaxAngle)
{
  std::pair<int, float> angleAndDistance = SinogramCreatorTools::getAngleAndDistance(firstX, firstY, secondX, secondY);

  int distanceRound =
      SinogramCreatorTools::roundToNearesMultiplicity(angleAndDistance.second + fMaxReconstructionLayerfSingleLayerRadius, fReconstructionDistanceAccuracy);
  if (distanceRound >= maxDistanceNumber || angleAndDistance.first >= kReconstructionMaxAngle)
  {
    std::cout << "Distance or angle > then max, distance: " << distanceRound << " (max : " << maxDistanceNumber << ")"
              << " angle: " << angleAndDistance.first << " (max: " << kReconstructionMaxAngle << ")" << std::endl;
  }
  if (distanceRound < 0) distanceRound = 0;
  return std::make_pair(distanceRound, angleAndDistance.first);
}

std::tuple<float, float, float> SinogramCreatorTools::cart2sph(float x, float y, float z)
{
  float theta = std::atan2(y, x);
  float phi = std::atan2(z, std::sqrt(x * x + y * y));
  float r = std::sqrt(x * x + y * y + z * z);
  return std::make_tuple(theta, phi, r);
}

std::tuple<float, float, float> SinogramCreatorTools::sph2cart(float theta, float phi, float r)
{
  float x = r * std::cos(phi) * std::cos(theta);
  float y = r * std::cos(phi) * std::sin(theta);
  float z = r * std::sin(phi);
  return std::make_tuple(x, y, z);
}

float SinogramCreatorTools::calculateLORSlice(float x1, float y1, float z1, double t1, float x2, float y2, float z2, double t2)
{
  float shiftX2 = x2 - x1;
  float shiftY2 = y2 - y1;
  float shiftZ2 = z2 - z1;

  float theta;
  float phi;
  float r;

  std::tie(theta, phi, r) = cart2sph(shiftX2, shiftY2, shiftZ2);

  const static float speed_of_light = 0.0299792458f;

  double diffR = speed_of_light * (t2 - t1) / 2.f;

  double r0 = r / 2.f - diffR;

  float resultX;
  float resultY;
  float resultZ;

  std::tie(resultX, resultY, resultZ) = sph2cart(theta, phi, r0);

  resultZ += z1;

  return resultZ;
}

int SinogramCreatorTools::getSplitRangeNumber(float firstZ, float secondZ, const std::vector<std::pair<float, float>>& zSplitRange)
{
  for (unsigned int i = 0; i < zSplitRange.size(); i++)
  {
    if (firstZ >= zSplitRange[i].first && firstZ <= zSplitRange[i].second && secondZ >= zSplitRange[i].first && secondZ <= zSplitRange[i].second)
      return i;
  }
  return -1;
}

int SinogramCreatorTools::getSplitRangeNumber(float z, const std::vector<std::pair<float, float>>& zSplitRange)
{
  for (unsigned int i = 0; i < zSplitRange.size(); i++)
  {
    if (z >= zSplitRange[i].first && z <= zSplitRange[i].second) return i;
  }
  return -1;
}

int SinogramCreatorTools::getSinogramSlice(float firstX, float firstY, float firstZ, double firstTOF, float secondX, float secondY, float secondZ,
                                           double secondTOF, const std::vector<std::pair<float, float>>& zSplitRange)
{
  float result = calculateLORSlice(firstX, firstY, firstZ, firstTOF, secondX, secondY, secondZ, secondTOF);
  return getSplitRangeNumber(result, zSplitRange);
}

unsigned int SinogramCreatorTools::getTOFSlice(double firstTOF, double secondTOF, double sliceSize)
{
  double tofDiff = (secondTOF - firstTOF) / 2.;
  return tofDiff / sliceSize;
}

//TODO: add time remapping also
std::pair<TVector3,TVector3> SinogramCreatorTools::remapToSingleLayer(const TVector3& firstHit, const TVector3& secondHit, const float r)
{
  float dx = (secondHit.X() - firstHit.X());
  float dy = (secondHit.Y() - firstHit.Y());
  if (std::abs(dx) < kEPSILON) return std::make_pair(TVector3(firstHit.X(), r, firstHit.Z()), TVector3(firstHit.X(), -r, secondHit.Z()));
  float a = dy / dx;
  float b = firstHit.Y() - firstHit.X() * a;
  if(std::abs(b) < kEPSILON) {
    float D = std::sqrt(1 + a * a);
    float x1d = r / D;
    float x2d = -r / D;

    float y1d = a * x1d;
    float y2d = a * x2d;
    //std::cout << "{ " << x1d << ", " << y1d << "}, {" << x2d << ", " << y2d << "}" << std::endl;
    return std::make_pair(TVector3(x1d, y1d, firstHit.Z()), TVector3(x2d, y2d, secondHit.Z()));
  }
  float A = 1 + a;
  float B = 2 * a * b;
  float C = b * b - r * r;

  float delta = B * B - (4 * A * C);
  if(delta <= 0) {
    WARNING("Could not find 2 interaction points with single layer remap, returning same points..");
    return std::make_pair(firstHit, secondHit);
  }

  float x1 = (-B + std::sqrt(delta)) / 2 * A;
  float x2 = (-B - std::sqrt(delta)) / 2 * A;

  float y1 = a * x1 + b;
  float y2 = a * x2 + b;

  //std::cout << "{ " << x1 << ", " << y1 << "}, {" << x2 << ", " << y2 << "}" << std::endl;
  return std::make_pair(TVector3(x1, y1, firstHit.Z()), TVector3(x2, y2, secondHit.Z()));
}
