/**
 *  @copyright Copyright 2021 The J-PET Framework Authors. All rights reserved.
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
 */
#ifndef INTERFILEMLPRECONSTRUCTION_H
#define INTERFILEMLPRECONSTRUCTION_H

#include "JPetUserTask/JPetUserTask.h"
#include <memory>

class InterfileMLPReconstruction : public JPetUserTask
{

public:
  InterfileMLPReconstruction(const char* name);
  virtual ~InterfileMLPReconstruction();
  virtual bool init() override;
  virtual bool exec() override;
  virtual bool terminate() override;

private:
  void setUpOptions();

  const std::string kOutputPath             = "InterfileMLPReconstruction_OutputPath_std::string";
  const std::string kXRangeOn3DHistogramKey = "InterfileMLPReconstruction_Xrange_On_3D_Histogram_int";
  const std::string kYRangeOn3DHistogramKey = "InterfileMLPReconstruction_Yrange_On_3D_Histogram_int";
  const std::string kZRangeOn3DHistogramKey = "InterfileMLPReconstruction_Zrange_On_3D_Histogram_int";
  const std::string kBinMultiplierKey       = "InterfileMLPReconstruction_Bin_Multiplier_double";

  std::string fOutputPath = "out";
  int fXRange = 50;
  int fYRange = 50;
  int fZRange = 30;
  double fBinMultiplier = 12;
  int fNumberOfBinsX = fXRange * fBinMultiplier; //1 bin is 8.33 mm, TBufferFile cannot write more then 1073741822 bytes
  int fNumberOfBinsY = fYRange * fBinMultiplier;
  int fNnumberOfBinsZ = fZRange * fBinMultiplier;
};

#endif /*  !INTERFILEMLPRECONSTRUCTION_H */
