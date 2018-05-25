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
//when cint is used instead of compiler, override word is not recognized
//nevertheless it's needed for checking if the structure of project is correct
#define override
#endif

#include "JPetUserTask/JPetUserTask.h"
#include "SinogramCreatorTools.h"
#include "JPetHit/JPetHit.h"
#include <vector>
#include <string>

class SinogramCreatorMC : public JPetUserTask
{
public:
  SinogramCreatorMC(const char *name);
  virtual ~SinogramCreatorMC();
  virtual bool init() override;
  virtual bool exec() override;
  virtual bool terminate() override;

private:
  SinogramCreatorMC(const SinogramCreatorMC &) = delete;
  SinogramCreatorMC &operator=(const SinogramCreatorMC &) = delete;

  void generateSinogram();
  void setUpOptions();
  bool checkSplitRange(float firstZ, float secondZ, int i);
  using SinogramResultType = std::vector<std::vector<unsigned int>>;

  SinogramResultType **fSinogram = nullptr;

  const std::string kOutFileNameKey = "SinogramCreatorMC_OutFileName_std::string";
  const std::string kReconstructionDistanceAccuracy = "SinogramCreatorMC_ReconstructionDistanceAccuracy_float";
  const std::string kZSplitNumber = "SinogramCreatorMC_SinogramZSplitNumber_int";
  const std::string kScintillatorLenght = "SinogramCreatorMC_ScintillatorLenght_float";
  const std::string kMaxReconstructionRadius = "SinogramCreatorMC_MaxReconstructionRadius_float";
  const std::string kInputDataKey = "SinogramCreatorMC_InputDataPath_std::string";

  const int kReconstructionMaxAngle = 180;
  const float EPSILON = 0.000001f;
  int fZSplitNumber = 1;
  std::vector<std::pair<float, float>> fZSplitRange;

  std::string fOutFileName = "sinogramMC";
  std::string fInputData = "sinogram_data.txt";
  int *fMaxValueInSinogram = nullptr; // to fill later in output file
  int *fCurrentValueInSinogram = nullptr;

  float fMaxReconstructionLayerRadius = 0.f;    //in cm
  float fReconstructionDistanceAccuracy = 0.1f; //in cm, 1mm accuracy
  float fScintillatorLenght = 50.0f;            //in cm
};

#endif /*  !SINOGRAMCREATORMC_H */