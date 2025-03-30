/**
 *  @copyright Copyright 2025 The J-PET Framework Authors. All rights reserved.
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
 *  @file ReadNTuple.cpp
 */

#include "ReadNTuple.h"
#include "JPetTaskIO/JPetInputHandlerNTU.h"
#include <Hits/JPetMCRecoHit/JPetMCRecoHit.h>
#include <Hits/JPetPhysRecoHit/JPetPhysRecoHit.h>
#include <JPetCommonTools/JPetCommonTools.h>
#include <JPetOptionsTools/JPetOptionsTools.h>
#include <iostream>

using namespace jpet_options_tools;
using namespace std;

ReadNTuple::ReadNTuple(const char* name) : JPetUserTask(name) {}

bool ReadNTuple::init()
{
  INFO("Reading ntupes from tree and buiding events");

  fOutputEvents = new JPetTimeWindow("JPetEvent");

  return true;
}

bool ReadNTuple::exec()
{
  if (auto entry = dynamic_cast<const JPetNTUData* const>(fEvent))
  {
    JPetEvent event;
    for (int i = 0; i < entry->fNumberOfHits; i++)
    {
      auto hit =
          new JPetBaseHit(entry->fHitTimes->at(i), entry->fHitTOTs->at(i), entry->fHitPos->at(i), getParamBank().getScin(entry->fHitScinIDs->at(i)));
      event.addHit(hit);
    }
    saveEvent(event);
  }
  else
  {
    return false;
  }
  return true;
}

bool ReadNTuple::terminate()
{
  INFO("Reading NTuple finshed.");
  return true;
}

void ReadNTuple::saveEvent(JPetEvent& event) { fOutputEvents->add<JPetEvent>(event); }
