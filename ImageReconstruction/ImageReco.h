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
 *  @file ImageReco.h
 */

#ifndef IMAGERECO_H
#define IMAGERECO_H

#ifdef __CINT__
//when cint is used instead of compiler, override word is not recognized
//nevertheless it's needed for checking if the structure of project is correct
#define override
#endif

#include "JPetUserTask/JPetUserTask.h"
#include <memory>

/**
 * @brief Image reconstruction module which should be used for quick, rough data tests.
 *
 * It implements a simple image reconstruction algorithm adapted from G. Korcyl's code.
 * It is equivalent to one back-projection without any filtering but including TOF information,
 * with the assumption that TOF error = 0. Before reconstructruction, the data are preselected
 * based on the set of conditions defined in the json file such as:
 * - ImageReco_CUT_ON_Z_VALUE_float
 * - ImageReco_CUT_ON_LOR_DISTANCE_FROM_CENTER_float
 * - ImageReco_ANNIHILATION_POINT_Z_float
 * - ImageReco_TOT_MIN_VALUE_IN_NS_float
 * - ImageReco_TOT_MAX_VALUE_IN_NS_float
 * - ImageReco_ANGLE_DELTA_MIN_VALUE_float
 * The point of annihilation is reconstructed for all combination of registered hits in given events.
 * Additional optional options:
 * - ImageReco_XRANGE_ON_3D_HISTOGRAM_int
 * - ImageReco_YRANGE_ON_3D_HISTOGRAM_int
 * - ImageReco_ZRANGE_ON_3D_HISTOGRAM_int
 * - ImageReco_BIN_MULTIPLIER_double
 */
class ImageReco : public JPetUserTask
{

public:
  ImageReco(const char* name);
  virtual ~ImageReco();
  virtual bool init() override;
  virtual bool exec() override;
  virtual bool terminate() override;
private:
  /* Calculated as:
  * x = hit1.X + (vdx / 2.0) + ((vdx / sqrt(|vdx^2 + vdy^2 + vdz^2|) * TOF))
  * y = hit1.Y + (vdy / 2.0) + ((vdy / sqrt(|vdx^2 + vdy^2 + vdz^2|) * TOF))
  * z = hit1.Z + (vdz / 2.0) + ((vdz / sqrt(|vdx^2 + vdy^2 + vdz^2|) * TOF))
  *
  * Where TOF is calculated as:
  * if hit1.Y > hit2.Y
  * TOF = ((hit1.timeA + hit1.timeB) / 2.0) - ((hit2.timeA + hit2.timeB) / 2.0) * 30
  * else:
  * TOF = ((hit2.timeA + hit2.timeB) / 2.0) - ((hit1.timeA + hit1.timeB) / 2.0) * 30
  */
  bool calculateAnnihilationPoint(const JPetHit& firstHit, const JPetHit& secondHit);
  bool cutOnZ(const JPetHit& first, const JPetHit& second);
  bool cutOnLORDistanceFromCenter(const JPetHit& first, const JPetHit& second);
  float angleDelta(const JPetHit& first, const JPetHit& second);
  double calculateSumOfTOTsOfHit(const JPetHit& hit);
  double calculateSumOfTOTs(const JPetPhysSignal& signal);
  bool checkConditions(const JPetHit& first, const JPetHit& second);

  const std::string kCutOnZValueKey = "ImageReco_CUT_ON_Z_VALUE_float";
  const std::string kCutOnLORDistanceKey = "ImageReco_CUT_ON_LOR_DISTANCE_FROM_CENTER_float";
  const std::string kCutOnAnnihilationPointZKey = "ImageReco_ANNIHILATION_POINT_Z_float";
  const std::string kCutOnTOTMinValueKey = "ImageReco_TOT_MIN_VALUE_IN_NS_float";
  const std::string kCutOnTOTMaxValueKey = "ImageReco_TOT_MAX_VALUE_IN_NS_float";
  const std::string kCutOnAngleDeltaMinValueKey = "ImageReco_ANGLE_DELTA_MIN_VALUE_float";

  const std::string kXRangeOn3DHistogramKey = "ImageReco_XRANGE_ON_3D_HISTOGRAM_int";
  const std::string kYRangeOn3DHistogramKey = "ImageReco_YRANGE_ON_3D_HISTOGRAM_int";
  const std::string kZRangeOn3DHistogramKey = "ImageReco_ZRANGE_ON_3D_HISTOGRAM_int";
  const std::string kBinMultiplierKey = "ImageReco_BIN_MULTIPLIER_double";

  int fXRange = 50;
  int fYRange = 50;
  int fZRange = 30;
  double fBinMultiplier = 12;
  int fNumberOfBinsX = fXRange * fBinMultiplier; //1 bin is 8 1/3 mm, TBufferFile cannot write more then 1073741822 bytes
  int fNumberOfBinsY = fYRange * fBinMultiplier;
  int fNnumberOfBinsZ = fZRange * fBinMultiplier;

  const int kNumberOfHitsInEventHisto = 10;
  const int kNumberOfConditions = 6;

  float fCUT_ON_Z_VALUE = 23;
  float fCUT_ON_LOR_DISTANCE_FROM_CENTER = 25;
  float fANNIHILATION_POINT_Z = 23;
  float fTOT_MIN_VALUE_IN_NS = 15;
  float fTOT_MAX_VALUE_IN_NS = 25;
  float fANGLE_DELTA_MIN_VALUE = 20;
};

#endif /*  !IMAGERECO_H */
