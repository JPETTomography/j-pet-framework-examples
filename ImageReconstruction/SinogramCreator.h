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
 *  @file SinogramCreator.h
 */

#ifndef SINOGRAMCREATOR_H
#define SINOGRAMCREATOR_H

#ifdef __CINT__
//when cint is used instead of compiler, override word is not recognized
//nevertheless it's needed for checking if the structure of project is correct
#define override
#endif

#include "JPetGeomMapping/JPetGeomMapping.h"
#include "JPetUserTask/JPetUserTask.h"
#include "SinogramCreatorTools.h"
#include "JPetHit/JPetHit.h"
#include <vector>
#include <string>

/**
 * @brief Module creating sinogram from data
 *
 * Input: *.unk.evt
 * Output: *.sino with root diagrams and sinogram*.ppm with calculated sinogram.
 *
 *
 * It defines 5 user options:
 * - "SinogramCreator_OutFileName_std::string": defines output file name where sinogram is saved
 * - "SinogramCreator_ReconstructionLayerRadius_float": defines radius of reconstruction layer
 * - "SinogramCreator_ReconstructionDistanceAccuracy_float": defines maximal round value for distance, in cm, e.g. 0.1 means 1 bin in sinogram corresponds to 0.1 cm in reality
 * - "SinogramCreator_SinogramZSplitNumber_int": defines number of splits around "z" coordinate
 * - "SinogramCreator_ScintillatorLenght_float": defines scintillator lenght in "z" coordinate
 */
class SinogramCreator : public JPetUserTask
{
public:
  SinogramCreator(const char* name);
  virtual ~SinogramCreator();
  virtual bool init() override;
  virtual bool exec() override;
  virtual bool terminate() override;

private:
  SinogramCreator(const SinogramCreator&) = delete;
  SinogramCreator& operator=(const SinogramCreator&) = delete;

  void setUpOptions();
  bool checkSplitRange(float firstZ, float secondZ, int i);
  using SinogramResultType = std::vector<std::vector<unsigned int>>;

  SinogramResultType** fSinogram = nullptr;

  const std::string kOutFileNameKey = "SinogramCreator_OutFileName_std::string";
  const std::string kReconstructionDistanceAccuracy = "SinogramCreator_ReconstructionDistanceAccuracy_float";
  const std::string kZSplitNumber = "SinogramCreator_SinogramZSplitNumber_int";
  const std::string kScintillatorLenght = "SinogramCreator_ScintillatorLenght_float";

  const int kReconstructionMaxAngle = 180;
  const float EPSILON = 0.000001f;
  int fZSplitNumber = 1;
  std::vector<std::pair<float, float>> fZSplitRange;

  std::string fOutFileName = "sinogram";
  int* fMaxValueInSinogram = nullptr; // to fill later in output file
  int* fCurrentValueInSinogram = nullptr;

  float fMaxReconstructionLayerRadius = 0.f;    //in cm
  float fReconstructionDistanceAccuracy = 0.1f; //in cm, 1mm accuracy
  float fScintillatorLenght = 50.0f;            //in cm
};

#endif /*  !SINOGRAMCREATOR_H */
