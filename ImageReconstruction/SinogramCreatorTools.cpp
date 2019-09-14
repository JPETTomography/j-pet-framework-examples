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
  if (std::abs(slope) < kEPSILON)
    slope = -1.f;
  float perpendicularSlope = -(1.f / slope);
  float d = firstY - (slope * firstX);
  float x = -d / (perpendicularSlope - slope);
  float y = perpendicularSlope * x;

  if (std::abs(d) < kEPSILON)
    d = 1.f;

  float xAngle = -d / (perpendicularSlope - slope);
  float yAngle = perpendicularSlope * xAngle;

  float angle = std::atan2(yAngle, xAngle);
  if (yAngle < 0) angle = -angle;
  angle *= (180.f / M_PI);
  const bool sign = x < 0.f;
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
  float dx = secondHit.X() - firstHit.X();
  float dy = secondHit.Y() - firstHit.Y();
  //float dz = secondHit.Z() - firstHit.Z();

  if (std::abs(dx) < kEPSILON) {
    if(std::abs(dy) < kEPSILON) {
      //std::cout << "{ " << firstHit.X() << ", " << r << ", " << firstHit.Z() << "}, {" << secondHit.X() << ", " << -r << ", " << secondHit.Z() << "}" << std::endl;
      return std::make_pair(TVector3(firstHit.X(), r, firstHit.Z()), TVector3(secondHit.X(), -r, secondHit.Z())); // make sure it is correct
    }
    float te = (r - firstHit.X()) / dy;
    float z1e = firstHit.Z() + (secondHit.Z() - firstHit.Z()) * te;
    float z2e = secondHit.Z() + (firstHit.Z() - secondHit.Z()) * te;
    //std::cout << "{ " << firstHit.X() << ", " << r << ", " << z1e << "}, {" << secondHit.X() << ", " << -r << ", " << z2e << "}" << std::endl;
    return std::make_pair(TVector3(firstHit.X(), r, z1e), TVector3(firstHit.X(), -r, z2e));
  }
  float a = dy / dx;
  float b = firstHit.Y() - firstHit.X() * a;
  if(std::abs(b) < kEPSILON) {
    float D = std::sqrt(1 + a * a);
    float x1d = r / D;
    float x2d = -r / D;

    float y1d = a * x1d;
    float y2d = a * x2d;

    float td = (x1d - firstHit.X()) / dx;
    float z1d = firstHit.Z() + (secondHit.Z() - firstHit.Z() * td);
    float z2d = secondHit.Z() + (firstHit.Z() - secondHit.Z() * td);
    //std::cout << "{ " << x1d << ", " << y1d << ", " << z1d << "}, {" << x2d << ", " << y2d << ", " << z2d << "}" << std::endl;
    return std::make_pair(TVector3(x1d, y1d, z1d), TVector3(x2d, y2d, z2d));
  }
  float A = 1 + a;
  float B = 2 * a * b;
  float C = b * b - r * r;

  float delta = B * B - (4 * A * C);
  if(delta <= 0) {
    //std::cout << "Same points" << std::endl;
    WARNING("Could not find 2 intersection points with single layer remap, returning same points..");
    return std::make_pair(firstHit, secondHit);
  }

  float x1 = (-B + std::sqrt(delta)) / 2 * A;
  float x2 = (-B - std::sqrt(delta)) / 2 * A;

  float y1 = a * x1 + b;
  float y2 = a * x2 + b;

  float t = (x1 - firstHit.X()) / dx;
  float z1 = firstHit.Z() + (secondHit.Z() - firstHit.Z() * t);
  float z2 = secondHit.Z() + (firstHit.Z() - secondHit.Z() * t);
  //std::cout << "{ " << x1 << ", " << y1 << ", " << z1 << "}, {" << x2 << ", " << y2 << ", " << z2 << "}" << std::endl;
  return std::make_pair(TVector3(x1, y1, z1), TVector3(x2, y2, z2));
}

// indepvar[0] = dist_xy
// indepvar[1] = -abs(z) for NEMA Phantom only
double SinogramCreatorTools::getPolyFit(std::vector<double> indepvar)
{
  const std::vector<std::vector<double>> modelTerms{{6, 0}, {5, 1}, {5, 0}, {4, 2}, {4, 1}, {4, 0}, {3, 3}, {3, 2}, {3, 1}, {3, 0},
                                                    {2, 4}, {2, 3}, {2, 2}, {2, 1}, {2, 0}, {1, 5}, {1, 4}, {1, 3}, {1, 2}, {1, 1},
                                                    {1, 0}, {0, 6}, {0, 5}, {0, 4}, {0, 3}, {0, 2}, {0, 1}, {0, 0}};
  const std::vector<double> coefficients{
      -4.134801885659948e-07, -2.300107024422413e-08, 1.63726635335888e-05,   1.73001508731807e-06,  1.359112755021784e-05, -0.0003483024181997085,
      3.0824911712077e-06,    -1.671743900149295e-05, -0.0002755710560288588, 0.006195186956691909,  2.139575490180591e-06, -3.604595062410784e-05,
      -0.0001028583160383859, 0.001337241556475614,   -0.07459337521834991,   4.354695109287826e-06, 7.061215552547179e-05, 0.0006737187696296268,
      0.002313067110373971,   0.002549526937584952,   0.3926701790901366,     1.707921126250134e-06, 5.241438152857635e-06, -0.0005029848848941915,
      -0.005095141886803388,  -0.01607730907583253,   -0.03130108005714758,   0.2797848665191068};

  int nt = modelTerms.size();
  assert(nt > 0);
  double ypred = 0;
  for (int i = 0; i < nt; i++)
  {
    double t = 1;
    for (unsigned int j = 0; j < modelTerms[0].size(); j++)
    {
      t = t * std::pow(indepvar[j], modelTerms.at(i).at(j));
    }
    ypred = ypred + t * coefficients[i];
  }
  return ypred;
}
