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
 * with the assumption that TOF error = 0. Before reconstructruction, the data are not preselected
 * it should be preselected using FilterEvents Task.
 * The point of annihilation is reconstructed for all combination of registered hits in given events.
 * Additional optional options:
 * - ImageReco_Annihilation_Point_Z_float
 * - ImageReco_Xrange_On_3D_Histogram_int
 * - ImageReco_Yrange_On_3D_Histogram_int
 * - ImageReco_Zrange_On_3D_Histogram_int
 * - ImageReco_Bin_Multiplier_double
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
  void setUpOptions();

  const std::string kCutOnAnnihilationPointZKey = "ImageReco_Annihilation_Point_Z_float";
  const std::string kXRangeOn3DHistogramKey = "ImageReco_Xrange_On_3D_Histogram_int";
  const std::string kYRangeOn3DHistogramKey = "ImageReco_Yrange_On_3D_Histogram_int";
  const std::string kZRangeOn3DHistogramKey = "ImageReco_Zrange_On_3D_Histogram_int";
  const std::string kBinMultiplierKey = "ImageReco_Bin_Multiplier_double";

  int fXRange = 50;
  int fYRange = 50;
  int fZRange = 30;
  double fBinMultiplier = 12;
  int fNumberOfBinsX = fXRange * fBinMultiplier; //1 bin is 8.33 mm, TBufferFile cannot write more then 1073741822 bytes
  int fNumberOfBinsY = fYRange * fBinMultiplier;
  int fNnumberOfBinsZ = fZRange * fBinMultiplier;
  float fANNIHILATION_POINT_Z = 23;

  const int kNumberOfHitsInEventHisto = 10;
  const int kNumberOfConditions = 6;
};

#endif /*  !IMAGERECO_H */
