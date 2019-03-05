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
 *  @file SinogramCreatorTools.h
 */

#ifndef SINOGRAMCREATORTOOLS_H
#define SINOGRAMCREATORTOOLS_H

#ifdef __CINT__
//when cint is used instead of compiler, override word is not recognized
//nevertheless it's needed for checking if the structure of project is correct
#define override
#endif

#include <utility>
#include <cmath>
#include <tuple>

class SinogramCreatorTools
{
public:
  using Point = std::pair<float, float>;

  static unsigned int roundToNearesMultiplicity(float numberToRound, float muliFactor);
  static std::pair<int, bool> getAngleAndDistanceSign(float firstX, float firstY, float secondX, float secondY);

  static float calculateDistance(float firstX, float firstY, float secondX, float secondY);

  static std::pair<int, int> getSinogramRepresentation(float firstX, float firstY, float secondX, float secondY, float fMaxReconstructionLayerRadius, float fReconstructionDistanceAccuracy, int maxDistanceNumber, int kReconstructionMaxAngle);

  static float calculateLORSlice(float x1, float y1, float z1, float t1,
                                 float x2, float y2, float z2, float t2);

private:
  SinogramCreatorTools() = delete;
  ~SinogramCreatorTools() = delete;
  SinogramCreatorTools(const SinogramCreatorTools&) = delete;
  SinogramCreatorTools& operator=(const SinogramCreatorTools&) = delete;

  static std::tuple<float, float, float> cart2sph(float x, float y, float z);
  static std::tuple<float, float, float> sph2cart(float theta, float phi, float r);
};

#endif /*  !SINOGRAMCREATORTOOLS_H */