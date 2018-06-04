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
 *  @file RilterEvents.h
 */

#ifndef FILTEREVENTS_H
#define FILTEREVENTS_H

#ifdef __CINT__
//when cint is used instead of compiler, override word is not recognized
//nevertheless it's needed for checking if the structure of project is correct
#define override
#endif

#include "JPetUserTask/JPetUserTask.h"
#include <memory>

/**
 * @brief Task that filter events and output reco.unk.evt for futher reconstruction.
 * It defines 6 User Params:
 * - FilterEvents_CUT_ON_Z_VALUE_float
 * - FilterEvents_CUT_ON_LOR_DISTANCE_FROM_CENTER_float
 * - FilterEvents_ANNIHILATION_POINT_Z_float
 * - FilterEvents_TOT_MIN_VALUE_IN_NS_float
 * - FilterEvents_TOT_MAX_VALUE_IN_NS_float
 * - FilterEvents_ANGLE_DELTA_MIN_VALUE_float
 */
class FilterEvents : public JPetUserTask
{

public:
  FilterEvents(const char *name);
  virtual ~FilterEvents();
  virtual bool init() override;
  virtual bool exec() override;
  virtual bool terminate() override;

private:
  bool cutOnZ(const JPetHit &first, const JPetHit &second);
  bool cutOnLORDistanceFromCenter(const JPetHit &first, const JPetHit &second);
  float angleDelta(const JPetHit &first, const JPetHit &second);
  double calculateSumOfTOTsOfHit(const JPetHit &hit);
  double calculateSumOfTOTs(const JPetPhysSignal &signal);
  bool checkConditions(const JPetHit &first, const JPetHit &second);
  void setUpOptions();

  const std::string kCutOnZValueKey = "FilterEvents_CUT_ON_Z_VALUE_float";
  const std::string kCutOnLORDistanceKey = "FilterEvents_CUT_ON_LOR_DISTANCE_FROM_CENTER_float";

  const std::string kCutOnTOTMinValueKey = "FilterEvents_TOT_MIN_VALUE_IN_NS_float";
  const std::string kCutOnTOTMaxValueKey = "FilterEvents_TOT_MAX_VALUE_IN_NS_float";
  const std::string kCutOnAngleDeltaMinValueKey = "FilterEvents_ANGLE_DELTA_MIN_VALUE_float";

  const int kNumberOfHitsInEventHisto = 10;
  const int kNumberOfConditions = 6;

  float fCUT_ON_Z_VALUE = 23;
  float fCUT_ON_LOR_DISTANCE_FROM_CENTER = 25;

  float fTOT_MIN_VALUE_IN_NS = 15;
  float fTOT_MAX_VALUE_IN_NS = 25;
  float fANGLE_DELTA_MIN_VALUE = 20;
};

#endif /*  !FILTEREVENTS_H */
