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
// when cint is used instead of compiler, override word is not recognized
// nevertheless it's needed for checking if the structure of project is correct
#define override
#endif

#include <cmath>
#include <tuple>
#include <utility>
#include <vector>
#include "TVector3.h"

class SinogramCreatorTools
{
public:
  static unsigned int roundToNearesMultiplicity(double numberToRound, double muliFactor);
  static std::pair<int, float> getAngleAndDistance(float firstX, float firstY, float secondX, float secondY);

  static std::pair<int, int> getSinogramRepresentation(float firstX, float firstY, float secondX, float secondY, float fMaxReconstructionLayerRadius,
                                                       float fReconstructionDistanceAccuracy, int maxDistanceNumber, int kReconstructionMaxAngle);

  static float calculateLORSlice(float x1, float y1, float z1, double t1, float x2, float y2, float z2, double t2);

  static int getSplitRangeNumber(float firstZ, float secondZ, const std::vector<std::pair<float, float>>& zSplitRange);
  static int getSplitRangeNumber(float z, const std::vector<std::pair<float, float>>& zSplitRange);
  static int getSinogramSlice(float firstX, float firstY, float firstZ, double firstTOF, float secondX, float secondY, float secondZ,
                              double secondTOF, const std::vector<std::pair<float, float>>& zSplitRange);
  static unsigned int getTOFSlice(double firstTOF, double secondTOF, double sliceSize);

  static std::pair<TVector3,TVector3> remapToSingleLayer(const TVector3& firstHit, const TVector3& secondHit, const float radius);
private:
  SinogramCreatorTools() = delete;
  ~SinogramCreatorTools() = delete;
  SinogramCreatorTools(const SinogramCreatorTools&) = delete;
  SinogramCreatorTools& operator=(const SinogramCreatorTools&) = delete;

  static std::tuple<float, float, float> cart2sph(float x, float y, float z);
  static std::tuple<float, float, float> sph2cart(float theta, float phi, float r);

  static constexpr float kEPSILON = 0.00001f;
};
#endif /*  !SINOGRAMCREATORTOOLS_H */
