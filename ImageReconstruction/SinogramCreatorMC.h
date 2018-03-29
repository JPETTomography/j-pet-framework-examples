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
 *  @file SinogramCreatorMC.h
 */

#ifndef SINOGRAMCREATORMC_H
#define SINOGRAMCREATORMC_H

#ifdef __CINT__
//when cint is used instead of compiler, override word is not recognized
//nevertheless it's needed for checking if the structure of project is correct
#define override
#endif

#include "JPetUserTask/JPetUserTask.h"
#include "SinogramCreatorTools.h"
#include "JPetHit/JPetHit.h"
#include <vector>
#include <string>

/**
 * @brief Module creating sinogram from data
 *
 * It implements creating sinogram from data, from 1 layer and data readed from ASCII file.
 *
 * It defines 6 user options:
 * - "SinogramCreatorMC_OutFileName_std::string": defines output file name where sinogram is saved
 * - "SinogramCreatorMC_ReconstructionLayerRadius_float": defines radius of reconstruction layer
 * - "SinogramCreatorMC_ReconstructionDistanceAccuracy_float": defines maximal round value for distance, in cm, e.g. 0.1 means 1px in sinogram corresponds to 0.1 cm in reality
 *
 */
class SinogramCreatorMC : public JPetUserTask
{
public:
  SinogramCreatorMC(const char* name);
  virtual ~SinogramCreatorMC();
  virtual bool init() override;
  virtual bool exec() override;
  virtual bool terminate() override;

private:
  SinogramCreatorMC(const SinogramCreatorMC&) = delete;
  SinogramCreatorMC& operator=(const SinogramCreatorMC&) = delete;

  void setUpOptions();
  using SinogramResultType = std::vector<std::vector<unsigned int>>;

  SinogramResultType* fSinogram = nullptr;

  const std::string kOutFileNameKey = "SinogramCreatorMC_OutFileName_std::string";
  const std::string kReconstructionLayerRadiusKey = "SinogramCreatorMC_ReconstructionLayerRadius_float";
  const std::string kReconstructionDistanceAccuracy = "SinogramCreatorMC_ReconstructionDistanceAccuracy_float";

  const int kReconstructionMaxAngle = 180;
  const float EPSILON = 0.000001f;

  std::string fOutFileName = "sinogramMC.ppm";
  unsigned int fMaxValueInSinogram = 0; // to fill later in output file

  float fReconstructionLayerRadius = 42.5f;
  float fReconstructionDistanceAccuracy = 0.01f; // in cm, 0.01mm accuracy

  std::vector<std::pair<SinogramCreatorTools::Point, SinogramCreatorTools::Point>> hitsVector;
};

#endif /*  !SINOGRAMCREATORMC_H */