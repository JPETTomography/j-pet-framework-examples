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
 *  @file SinogramCreatorMC.h
 */

#ifndef SINOGRAMCREATORMC_H
#define SINOGRAMCREATORMC_H

#ifdef __CINT__
// when cint is used instead of compiler, override word is not recognized
// nevertheless it's needed for checking if the structure of project is correct
#define override
#endif

#include "JPetFilterRamLak.h"
#include "SinogramCreator.h"

class SinogramCreatorMC : public SinogramCreator {
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
  void generateSinogram();

  const std::string kOutFileNameKey = "SinogramCreatorMC_OutFileName_std::string";
  const std::string kReconstructionDistanceAccuracy = "SinogramCreatorMC_ReconstructionDistanceAccuracy_float";
  const std::string kZSplitNumber = "SinogramCreatorMC_SinogramZSplitNumber_int";
  const std::string kScintillatorLenght = "SinogramCreatorMC_ScintillatorLenght_float";
  const std::string kMaxReconstructionRadius = "SinogramCreatorMC_MaxReconstructionRadius_float";
  const std::string kInputDataKey = "SinogramCreatorMC_InputDataPaths_std::vector<std::string>";
  const std::string kEnableNonPerperdicualLOR = "SinogramCreatorMC_EnableNonPerperdicalLOR_bool";
  const std::string kEnableTOFReconstrution = "SinogramCreatorMC_EnableTOFReconstruction_bool";

  std::vector<std::string> fInputData{"sinogram_data.txt"};
  std::string fOutFileName = "sinogramMC";
};

#endif /*  !SINOGRAMCREATORMC_H */