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

#include <JPetUserTask/JPetUserTask.h>
#include "SinogramCreatorTools.h"
#include <JPetHit/JPetHit.h>
#include <vector>

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

  bool checkLayer(const JPetHit& hit);
  using SinogramResultType = std::vector<std::vector<int>>;

  SinogramResultType* fSinogram = nullptr;

  float fElipsonForHits = 0.30f;
  const float kReconstructionLayerRadius = 42.5f; //TODO CHECK REAL VALUE!!
  const float kReconstructionStartAngle = 0.f;
  const float kReconstructionEndAngle = 180.f;
  const float kReconstructionDistanceAccuracy = 0.5f; // in cm, 5mm accuracy
  const unsigned int kNumberOfScintillatorsInReconstructionLayer = 48; // number of scintillators in full layer
};

#endif /*  !SINOGRAMCREATOR_H */