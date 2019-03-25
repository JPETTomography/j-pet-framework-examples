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
 *  @file SinogramCreatorTOF.h
 */

#ifndef SINOGRAMCREATORTOF_H
#define SINOGRAMCREATORTOF_H

#ifdef __CINT__
// when cint is used instead of compiler, override word is not recognized
// nevertheless it's needed for checking if the structure of project is correct
#define override
#endif

#include "JPetFilterRamLak.h"
#include "SinogramCreator.h"

class SinogramCreatorTOF : public SinogramCreator
{
public:
  SinogramCreatorTOF(const char* name);
  virtual ~SinogramCreatorTOF();
  virtual bool init() override;
  virtual bool exec() override;
  virtual bool terminate() override;

private:
  SinogramCreatorTOF(const SinogramCreatorTOF&) = delete;
  SinogramCreatorTOF& operator=(const SinogramCreatorTOF&) = delete;

  bool analyzeHits(const float firstX, const float firstY, const float firstZ, const double firstTOF, const float secondX, const float secondY,
                   const float secondZ, const double secondTOF);
  void setUpOptions();
  void generateSinogram();

  const std::string kOutFileNameKey = "SinogramCreatorTOF_OutFileName_std::string";
  const std::string kReconstructionDistanceAccuracy = "SinogramCreatorTOF_ReconstructionDistanceAccuracy_float";
  const std::string kZSplitNumber = "SinogramCreatorTOF_SinogramZSplitNumber_int";
  const std::string kScintillatorLenght = "SinogramCreatorTOF_ScintillatorLenght_float";
  const std::string kMaxReconstructionRadius = "SinogramCreatorTOF_MaxReconstructionRadius_float";
  const std::string kInputDataKey = "SinogramCreatorTOF_InputDataPaths_std::vector<std::string>";
  const std::string kEnableObliqueLORRemapping = "SinogramCreatorTOF_EnableObliqueLORRemapping_bool";
  const std::string kEnableTOFReconstruction = "SinogramCreatorTOF_EnableKDEReconstruction_bool";
  const std::string kTOFSliceSize = "SinogramCreatorTOF_TOFSliceSize_float";
  const std::string kReconstructSliceNumbers = "SinogramCreatorTOF_ReconstructSliceNumber_std::vector<int>";

  std::vector<std::string> fInputData{"sinogram_data.txt"};
  std::string fOutFileName = "sinogramMC";
  float fTOFSliceSize = 100.f;

  JPetRecoImageTools::Matrix3D* fSinogramDataTOF = nullptr;
};

#endif /*  !SINOGRAMCREATORTOF_H */
