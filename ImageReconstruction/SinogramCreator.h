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
 *  @file SinogramCreator.h
 */

#ifndef SINOGRAMCREATOR_H
#define SINOGRAMCREATOR_H

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
 * It implements creating sinogram from data, but only for 1st layer.
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
  bool checkLayer(const JPetHit& hit);
  using SinogramResultType = std::vector<std::vector<unsigned int>>;

  SinogramResultType* fSinogram = nullptr;

  const std::string kOutFileNameKey = "SinogramCreator_OutFileName_std::string";
  const std::string kReconstructionLayerRadiusKey = "SinogramCreator_ReconstructionLayerRadius_float";
  const std::string kReconstructionStartAngle = "SinogramCreator_ReconstructionStartAngle_float";
  const std::string kReconstructionEndAngle = "SinogramCreator_ReconstructionEndAngle_float";
  const std::string kReconstructionDistanceAccuracy = "SinogramCreator_ReconstructionDistanceAccuracy_float";
  const std::string kReconstructionAngleStep = "SinogramCreator_ReconstructionAngleStep_float";

  std::string fOutFileName = "sinogram.ppm";
  unsigned int fMaxValueInSinogram = 0; // to fill later output file

  float fReconstructionLayerRadius = 42.5f;
  float fReconstructionStartAngle = 0.f;
  float fReconstructionEndAngle = 180.f;
  float fReconstructionDistanceAccuracy = 0.01f; // in cm, 5mm accuracy
  float fReconstructionAngleStep = 0.5f;
};

#endif /*  !SINOGRAMCREATOR_H */