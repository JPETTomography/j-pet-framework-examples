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
 *  @file ReconstructionTask.h
 */

#ifndef ReconstructionTask_H
#define ReconstructionTask_H

#ifdef __CINT__
// when cint is used instead of compiler, override word is not recognized
// nevertheless it's needed for checking if the structure of project is correct
#define override
#endif

#include <string>
#include <vector>

#include "JPetFilterCosine.h"
#include "JPetFilterHamming.h"
#include "JPetFilterHann.h"
#include "JPetFilterInterface.h"
#include "JPetFilterNone.h"
#include "JPetFilterRidgelet.h"
#include "JPetFilterSheppLogan.h"
#include "JPetRecoImageTools.h"
#include "JPetUserTask/JPetUserTask.h"

#include "JPetSinogramType.h"

class ReconstructionTask : public JPetUserTask
{
public:
  explicit ReconstructionTask(const char* name);
  virtual ~ReconstructionTask();
  virtual bool init() override;
  virtual bool exec() override;
  virtual bool terminate() override;

  enum kFilterType
  {
    kFilterNotFound,
    kFilterNone,
    kFilterCosine,
    kFilterHamming,
    kFilterHann,
    kFilterRidgelet,
    kFilterSheppLogan
  };

  enum kWeightingType
  {
    kWeightNotFound,
    kFBP,
    kTOFFBP
  };

private:
  ReconstructionTask(const ReconstructionTask&) = delete;
  ReconstructionTask& operator=(const ReconstructionTask&) = delete;

  /**
   * @brief Helper function used in saving result
   * \param result matrix to find max value
   * \return max value of matrix
   */
  int getMaxValue(const JPetSinogramType::SparseMatrix& result);
  /**
   * @brief Helper function used to save results(sinograms and reconstructed images)
   * \param result resulted matrix to save
   * \param outputFileName name with path where to save result
   */
  void saveResult(const JPetSinogramType::SparseMatrix& result, const std::string& outputFileName);

  /**
   * @brief Function where all options from user params are readed and setted.
   * Should be called before any other functions.
   */

  void setUpOptions();

  const std::string kInFileNameKey = "SinogramCreator_OutFileName_std::string"; // input file for Reconstruction is output from SinogramCreator

  const std::string kReconstructSliceNumbers = "ReconstructionTask_ReconstructSliceNumbers_std::vector<int>";

  const std::string kFilterCutOffValueBegin = "ReconstructionTask_FilterCutOffValueBegin_float";
  const std::string kFilterCutOffValueEnd = "ReconstructionTask_FilterCutOffValueEnd_float";
  const std::string kFilterCutOffValueStep = "ReconstructionTask_FilterCutOffValueStep_float";

  const std::string kFilterName = "ReconstructionTask_FilterName_std::string";
  const std::string kFilterAlpha = "ReconstructionTask_FilterAlphaValue_float";
  const std::string kReconstructionName = "ReconstructionTask_ReconstructionType_std::string";

  const std::string kLORTOFSigma = "ReconstructionTask_LORTOFSigma_float";

  const std::string kOutFileNameKey = "ReconstructionTask_OutFileName_std::string";

  std::vector<int> fReconstructSliceNumbers; // reconstruct only slices that was given in userParams

  float fCutOffValueBegin = 1.f;
  float fCutOffValueEnd = 1.f;
  float fCutOffValueStep = 0.01f;

  float fFilterAlphaValue = 0.1f;

  float fLORTOFSigma = 150.f;

  std::string fFilterName = "RamLak";
  std::string fReconstructionName = "FBP";
  std::string fOutFileName = "sinogram.root";
  std::string fInFileName = "sinogram.root";

  JPetSinogramType* fSinogram = nullptr;
};

#endif /*  !ReconstructionTask_H */
