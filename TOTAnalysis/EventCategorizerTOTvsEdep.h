/**
 *  @copyright Copyright 2020 The J-PET Framework Authors. All rights reserved.
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
 *  @file EventCategorizer.h
 */

#ifndef EVENTCATEGORIZER_H
#define EVENTCATEGORIZER_H

#include "../LargeBarrelAnalysis/EventCategorizerTools.h"
#include "../LargeBarrelAnalysis/HitFinderTools.h"
#include <JPetCommonTools/JPetCommonTools.h>
#include <JPetEvent/JPetEvent.h>
#include <JPetHit/JPetHit.h>
#include <JPetUserTask/JPetUserTask.h>
#include <map>
#include <vector>

using namespace std;
class JPetWriter;

#ifdef __CINT__
#define override
#endif

class EventCategorizer : public JPetUserTask
{
public:
  EventCategorizer(const char* name);
  virtual ~EventCategorizer() {}
  virtual bool init() override;
  virtual bool exec() override;
  virtual bool terminate() override;

private:
  const double kLightVelocity_cm_ns = 29.9792458; // cm /ns
  const double kNsToPs = 1000;                    // nano second to pico second converter
  const double kPsToNs = 0.001;                   // pico second to nano second converter

  const std::string kTOTvsEdepAnniSelUpperParamKey = "TOTvsEdep_AnniSelUpper_float";
  const std::string kTOTvsEdepDeexSelUpperParamKey = "TOTvsEdep_DeexSelUpper_float";

  double fAnniSelUpper = 35.;
  double fDeexSelUpper = 65.0;  
  
  int fEventNumber = 0;
  int fTotal3HitEvents = 0;
  double TimeDiff_From = 10; // ns   time diff between Prompt and annihilation photon from o-Ps
  double TimeDiff_To = 100;  // ns
  bool fSaveControlHistos = true;
  void initialiseHistograms();
  void saveEvents(const std::vector<JPetEvent>& event);
  void deexcitationSelection(const std::vector<double>& angles, const JPetHit& firstHit2, const JPetHit& secondHit2, const JPetHit& thirdHit2);
  void annihilationSelection(const std::vector<double>& angles, const JPetHit& firstHit2, const JPetHit& secondHit2, const JPetHit& thirdHit2);
  std::vector<JPetHit> reorderHits(std::vector<JPetHit> hits);
  std::vector<JPetEvent> analyseThreeHitEvent(const JPetEvent* event);
  std::vector<double_t> scatterAnalysis(const JPetHit& orig, const JPetHit& scatter, const JPetHit&, double);
  double calcAngle(const JPetHit& orig, const JPetHit& scatter);
  void writeSelected(JPetHit orig, JPetHit scatter, std::vector<double_t> values, bool isScatter, double, double, int ScinID);
};

#endif /* !EVENTCATEGORIZER_H */
