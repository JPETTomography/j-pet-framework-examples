/**
 *  @copyright Copyright 2018 The J-PET Framework Authors. All rights reserved.
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

class SinogramCreatorTools
{
public:
  using Point = std::pair<float, float>;

  static unsigned int roundToNearesMultiplicity(float numberToRound, float muliFactor);
  static int calculateAngle(float firstX, float firstY, float secondX, float secondY);

  static float calculateDistance(float firstX, float firstY, float secondX, float secondY);

private:
  SinogramCreatorTools() = delete;
  ~SinogramCreatorTools() = delete;
  SinogramCreatorTools(const SinogramCreatorTools&) = delete;
  SinogramCreatorTools& operator=(const SinogramCreatorTools&) = delete;

  static float calculateNorm(float firstX, float firstY, float secondX, float secondY);
};

#endif /*  !SINOGRAMCREATORTOOLS_H */