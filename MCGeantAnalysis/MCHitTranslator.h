/**
 *  @copyright Copyright 2016 The J-PET Framework Authors. All rights reserved.
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
 *  @file MCHitTranslator.h
 */

#ifndef MCHitTranslator_H
#define MCHitTranslator_H

#include <map>
#include <vector>
#include <JPetUserTask/JPetUserTask.h>
#include <JPetMCHit/JPetMCHit.h>
#include "MCHitTranslatorTools.h"

class JPetWriter;

#ifdef __CINT__
//when cint is used instead of compiler, override word is not recognized
//nevertheless it's needed for checking if the structure of project is correct
#   define override
#endif

/**
 * @brief      Module responsible for creating JPetMCHit from GEANT MC simulations
 *
 */
class MCHitTranslator: public JPetUserTask
{

public:
  MCHitTranslator(const char* name);
  virtual ~MCHitTranslator();
  virtual bool init() override;
  virtual bool exec() override;
  virtual bool terminate() override;


protected :

  bool kFirstTime = true;
//  MCHitTranslatorTools::SignalsContainer fAllSignalsInTimeWindow;
  MCHitTranslatorTools fHitTools;
//
//  void fillSignalsMap(const JPetPhysSignal& signal);
  void saveHits(const std::vector<JPetMCHit>& hits);
//  const std::string fTimeWindowWidthParamKey = "MCHitTranslator_TimeWindowWidth_float";

};

#endif 
