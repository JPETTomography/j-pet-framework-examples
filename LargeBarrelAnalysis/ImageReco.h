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

#include <JPetUserTask/JPetUserTask.h>
#include <memory>

/**
 * @brief TODO...
 *
 * TODO...
 *
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
  bool calculateReconstructedPosition(const JPetHit& firstHit, const JPetHit& secondHit);
  bool cutOnZ(const JPetHit& first, const JPetHit& second);
  bool cutOnLORDistanceFromCenter(const JPetHit& first, const JPetHit& second);
  float angleDelta(const JPetHit& first, const JPetHit& second);
  double calculateSumOfTOTsOfHit(const JPetHit& hit);
  double calculateSumOfTOTs(const JPetPhysSignal& signal);
  bool checkConditions(const JPetHit& first, const JPetHit& second);
};

#endif /*  !IMAGERECO_H */
