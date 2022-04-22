/**
 *  @copyright Copyright 2022 The J-PET Framework Authors. All rights reserved.
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
 *  @file RedModuleEventCategorizer.cpp
 */

#include "RedModuleEventCategorizer.h"
#include "../ModularDetectorAnalysis/CalibrationTools.h"
#include "../ModularDetectorAnalysis/EventCategorizerTools.h"
#include <JPetOptionsTools/JPetOptionsTools.h>
#include <JPetWriter/JPetWriter.h>
#include <boost/property_tree/json_parser.hpp>
#include <iostream>

using namespace jpet_options_tools;
using namespace std;

RedModuleEventCategorizer::RedModuleEventCategorizer(const char* name) : JPetUserTask(name) {}

RedModuleEventCategorizer::~RedModuleEventCategorizer() {}

bool RedModuleEventCategorizer::init()
{
  INFO("Event categorization started.");

  // Reading user parameters
  // Event time window
  if (isOptionSet(fParams.getOptions(), kEventTimeParamKey))
  {
    fEventTimeWindow = getOptionAsDouble(fParams.getOptions(), kEventTimeParamKey);
  }

  if (isOptionSet(fParams.getOptions(), kRefDetSlotIDParamKey))
  {
    fRefDetSlotID = getOptionAsInt(fParams.getOptions(), kRefDetSlotIDParamKey);
  }
  INFO(Form("Using slot with ID %d as reference detector.", fRefDetSlotID));

  // Reading file with constants to property tree
  if (isOptionSet(fParams.getOptions(), kConstantsFileParamKey))
  {
    boost::property_tree::read_json(getOptionAsString(fParams.getOptions(), kConstantsFileParamKey), fConstansTree);
  }

  // Getting bools for saving histograms
  if (isOptionSet(fParams.getOptions(), kSaveControlHistosParamKey))
  {
    fSaveControlHistos = getOptionAsBool(fParams.getOptions(), kSaveControlHistosParamKey);
  }

  // Input events type
  fOutputEvents = new JPetTimeWindow("JPetEvent");

  // Initialise hisotgrams
  if (fSaveControlHistos)
  {
    initialiseHistograms();
  }

  return true;
}

bool RedModuleEventCategorizer::exec()
{
  if (auto timeWindow = dynamic_cast<const JPetTimeWindow* const>(fEvent))
  {
    vector<JPetEvent> events;
    for (uint i = 0; i < timeWindow->getNumberOfEvents(); i++)
    {
      const auto& event = dynamic_cast<const JPetEvent&>(timeWindow->operator[](i));

      for (uint i = 0; i < event.getHits().size(); i++)
      {
        auto firstHit = dynamic_cast<const JPetPhysRecoHit*>(event.getHits().at(i));
        if (!firstHit)
        {
          continue;
        }

        for (uint j = i + 1; j < event.getHits().size(); j++)
        {
          auto secondHit = dynamic_cast<const JPetPhysRecoHit*>(event.getHits().at(j));
          if (!secondHit)
          {
            continue;
          }

          auto tDiff = secondHit->getTime() - firstHit->getTime();
          auto slot1Type = firstHit->getScin().getSlot().getType();
          auto slot2Type = secondHit->getScin().getSlot().getType();
          auto slot1ID = firstHit->getScin().getSlot().getID();
          auto slot2ID = secondHit->getScin().getSlot().getID();
          auto scin1ID = firstHit->getScin().getID();
          auto scin2ID = secondHit->getScin().getID();

          // WLS - Red module coincidences
          if (slot1Type == JPetSlot::WLS && slot2Type == JPetSlot::Module && scin2ID != fRefDetSlotID)
          {
            getStatistics().fillHistogram("hit_tdiff_red_wls", tDiff);
            getStatistics().fillHistogram("hit_tdiff_red_wls_scin", scin2ID, tDiff);
          }
          if (slot2Type == JPetSlot::WLS && slot1Type == JPetSlot::Module && scin1ID != fRefDetSlotID)
          {
            getStatistics().fillHistogram("hit_tdiff_red_wls", tDiff);
            getStatistics().fillHistogram("hit_tdiff_red_wls_scin", scin1ID, tDiff);
          }

          // Red - black -- several attempts of finding coincidences
          if (slot1ID == 202 && slot2ID == 204)
          {
            getStatistics().fillHistogram("hit_tdiff_red_black", tDiff);
            getStatistics().fillHistogram("hit_tdiff_red_black_scin", scin1ID, tDiff);
          }
          if (slot1ID == 203 && slot2ID == 204)
          {
            getStatistics().fillHistogram("hit_tdiff_red_black", tDiff);
            getStatistics().fillHistogram("hit_tdiff_red_black_scin", scin1ID, tDiff);
          }
          if (slot2ID == 202 && slot1ID == 204)
          {
            getStatistics().fillHistogram("hit_tdiff_red_black", tDiff);
            getStatistics().fillHistogram("hit_tdiff_red_black_scin", scin2ID, tDiff);
          }
          if (slot2ID == 203 && slot1ID == 204)
          {
            getStatistics().fillHistogram("hit_tdiff_red_black", tDiff);
            getStatistics().fillHistogram("hit_tdiff_red_black_scin", scin2ID, tDiff);
          }

          // if (scin1ID == fRefDetSlotID && slot2Type == JPetSlot::Module)
          // {
          //   getStatistics().fillHistogram("hit_tdiff_red_black", secondHit->getTime() - firstHit->getTime());
          // }
          // else if (scin2ID == fRefDetSlotID && slot1Type == JPetSlot::Module)
          // {
          //   getStatistics().fillHistogram("hit_tdiff_red_black", firstHit->getTime() - secondHit->getTime());
          // }
          //
          // if (slot1Type == JPetSlot::Module && slot2Type == JPetSlot::Module)
          // {
          //   if (scin1ID == fRefDetSlotID)
          //   {
          //     getStatistics().fillHistogram("hit_tdiff_red_black", secondHit->getTime() - firstHit->getTime());
          //   }
          //   if (scin2ID == fRefDetSlotID)
          //   {
          //     getStatistics().fillHistogram("hit_tdiff_red_black", firstHit->getTime() - secondHit->getTime());
          //   }
          // }
        }
      }
    }
    saveEvents(events);
  }
  else
  {
    return false;
  }
  return true;
}

bool RedModuleEventCategorizer::terminate()
{
  INFO("Event categorization completed.");
  return true;
}

void RedModuleEventCategorizer::saveEvents(const vector<JPetEvent>& events)
{
  for (const auto& event : events)
  {
    fOutputEvents->add<JPetEvent>(event);
  }
}

void RedModuleEventCategorizer::initialiseHistograms()
{
  auto minScinID = getParamBank().getScins().begin()->first;
  auto maxScinID = getParamBank().getScins().rbegin()->first;

  getStatistics().createHistogramWithAxes(new TH1D("hit_tdiff_red_wls", "hit_tdiff_red_wls", 200, -fEventTimeWindow, fEventTimeWindow), "tdiff [ps]",
                                          "hit pairs");

  getStatistics().createHistogramWithAxes(new TH2D("hit_tdiff_red_wls_scin", "hit_tdiff_red_wls_scin", maxScinID - minScinID + 1, minScinID - 0.5,
                                                   maxScinID + 0.5, 201, -fEventTimeWindow, fEventTimeWindow),
                                          "Scintillator ID", "time difference [ps]");

  getStatistics().createHistogramWithAxes(new TH1D("hit_tdiff_red_black", "hit_tdiff_red_black", 200, -fEventTimeWindow, fEventTimeWindow),
                                          "tdiff [ps]", "hit pairs");

  getStatistics().createHistogramWithAxes(new TH2D("hit_tdiff_red_black_scin", "hit_tdiff_red_black_scin", maxScinID - minScinID + 1, minScinID - 0.5,
                                                   maxScinID + 0.5, 201, -fEventTimeWindow, fEventTimeWindow),
                                          "Scintillator ID", "time difference [ps]");
}
