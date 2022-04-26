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

  if (isOptionSet(fParams.getOptions(), k2gThetaDiffParamKey))
  {
    f2gThetaDiff = getOptionAsDouble(fParams.getOptions(), k2gThetaDiffParamKey);
  }
  else
  {
    WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.", k2gThetaDiffParamKey.c_str(), f2gThetaDiff));
  }

  if (isOptionSet(fParams.getOptions(), k2gTimeDiffParamKey))
  {
    f2gTimeDiff = getOptionAsDouble(fParams.getOptions(), k2gTimeDiffParamKey);
  }
  else
  {
    WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.", k2gTimeDiffParamKey.c_str(), f2gTimeDiff));
  }

  // Reading ToT cut values
  if (isOptionSet(fParams.getOptions(), kToTCutAnniMinParamKey))
  {
    fToTCutAnniMin = getOptionAsDouble(fParams.getOptions(), kToTCutAnniMinParamKey);
  }
  else
  {
    WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.", kToTCutAnniMinParamKey.c_str(), fToTCutAnniMin));
  }

  if (isOptionSet(fParams.getOptions(), kToTCutAnniMaxParamKey))
  {
    fToTCutAnniMax = getOptionAsDouble(fParams.getOptions(), kToTCutAnniMaxParamKey);
  }
  else
  {
    WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.", kToTCutAnniMaxParamKey.c_str(), fToTCutAnniMax));
  }

  // Time variable used as +- axis limits for histograms with time spectra
  if (isOptionSet(fParams.getOptions(), kMaxTimeDiffParamKey))
  {
    fMaxTimeDiff = getOptionAsDouble(fParams.getOptions(), kMaxTimeDiffParamKey);
  }
  else
  {
    WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.", kMaxTimeDiffParamKey.c_str(), fMaxTimeDiff));
  }

  // For plotting ToT histograms
  if (isOptionSet(fParams.getOptions(), kToTHistoUpperLimitParamKey))
  {
    fToTHistoUpperLimit = getOptionAsDouble(fParams.getOptions(), kToTHistoUpperLimitParamKey);
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

  fSourcePos.SetXYZ(0.0, 0.0, 0.0);

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

          // Red - black -- finding coincidences
          if (slot1ID == 202 && slot2ID == 204 || slot1ID == 203 && slot2ID == 204 || slot2ID == 202 && slot1ID == 204 ||
              slot2ID == 203 && slot1ID == 204)
          {
            getStatistics().fillHistogram("hit_tdiff_red_black", tDiff);
            getStatistics().fillHistogram("hit_tdiff_red_black_scin", scin1ID, tDiff);
            getStatistics().fillHistogram("hit_tdiff_red_black_scin", scin2ID, tDiff);

            if (EventCategorizerTools::checkFor2Gamma(firstHit, secondHit, getStatistics(), fSaveControlHistos, f2gThetaDiff, f2gTimeDiff,
                                                      fToTCutAnniMin, fToTCutAnniMax, fSourcePos))
            {
              JPetEvent newEvent = event;
              newEvent.addEventType(JPetEventType::k2Gamma);
              events.push_back(newEvent);
            }
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

  // Red module specific histograms
  getStatistics().createHistogramWithAxes(new TH1D("hit_tdiff_red_wls", "hit_tdiff_red_wls", 200, 0.0, fEventTimeWindow), "tdiff [ps]", "hit pairs");

  getStatistics().createHistogramWithAxes(new TH2D("hit_tdiff_red_wls_scin", "hit_tdiff_red_wls_scin", maxScinID - minScinID + 1, minScinID - 0.5,
                                                   maxScinID + 0.5, 201, 0.0, fEventTimeWindow),
                                          "Scintillator ID", "time difference [ps]");

  getStatistics().createHistogramWithAxes(new TH1D("hit_tdiff_red_black", "hit_tdiff_red_black", 200, 0.0, fEventTimeWindow), "tdiff [ps]",
                                          "hit pairs");

  getStatistics().createHistogramWithAxes(new TH2D("hit_tdiff_red_black_scin", "hit_tdiff_red_black_scin", maxScinID - minScinID + 1, minScinID - 0.5,
                                                   maxScinID + 0.5, 201, 0.0, fEventTimeWindow),
                                          "Scintillator ID", "time difference [ps]");

  // Histograms for 2 gamama events
  getStatistics().createHistogramWithAxes(new TH1D("2g_tot", "2 gamma event - average ToT scaled", 201, 0.0, fToTHistoUpperLimit),
                                          "Time over Threshold [ps]", "Number of Hits");

  getStatistics().createHistogramWithAxes(new TH2D("2g_tot_scin", "2 gamma event - average ToT scaled per scintillator", maxScinID - minScinID + 1,
                                                   minScinID - 0.5, maxScinID + 0.5, 201, 0.0, fToTHistoUpperLimit),
                                          "Scintillator ID", "Time over Threshold [ps]");

  getStatistics().createHistogramWithAxes(
      new TH2D("2g_tot_z_pos", "2 gamma event - average ToT scaled vs. hit z position", 101, -25.5, 25.5, 201, 0.0, fToTHistoUpperLimit),
      "Hit z position [cm]", "Time over Threshold [ps]");

  getStatistics().createHistogramWithAxes(new TH1D("2g_tof", "2 gamma event - TOF calculated by convention", 201, -fMaxTimeDiff, fMaxTimeDiff),
                                          "Time of Flight [ps]", "Number of Hit Pairs");

  getStatistics().createHistogramWithAxes(new TH2D("2g_tof_scin", "2 gamma event - TOF calculated by convention per scintillator",
                                                   maxScinID - minScinID + 1, minScinID - 0.5, maxScinID + 0.5, 201, -fMaxTimeDiff, fMaxTimeDiff),
                                          "Scintillator ID", "Time of Flight [ps]");

  getStatistics().createHistogramWithAxes(new TH1D("2g_ab_tdiff", "2 gamma event - hits A-B time difference", 201, -fMaxTimeDiff, fMaxTimeDiff),
                                          "A-B Signal Time Difference [ps]", "Number of Hit Pairs");

  getStatistics().createHistogramWithAxes(new TH2D("2g_ab_tdiff_scin", "2 gamma event - hits A-B time difference per scintillator",
                                                   maxScinID - minScinID + 1, minScinID - 0.5, maxScinID + 0.5, 201, -fMaxTimeDiff, fMaxTimeDiff),
                                          "Scintillator ID", "A-B Signal Time Difference [ps]");

  getStatistics().createHistogramWithAxes(new TH1D("2g_theta", "2 gamma event - flight vectors theta", 181, -0.5, 180.5), "Angle [degree]",
                                          "Number of Hit Pairs");

  getStatistics().createHistogramWithAxes(new TH2D("2g_theta_scin", "2 gamma event - flight vectors theta per scintillator",
                                                   maxScinID - minScinID + 1, minScinID - 0.5, maxScinID + 0.5, 181, -0.5, 180.5),
                                          "Scintillator ID", "Angle [degree]");

  getStatistics().createHistogramWithAxes(
      new TH2D("2g_theta_z_pos", "2 gamma event - flight vectors theta vs. hit z position", 101, -25.5, 25.5, 181, -0.5, 180.5),
      "Hit z position [cm]", "Angle [degree]");

  // Cut stats
  getStatistics().createHistogramWithAxes(
      new TH1D("cut_stats_none", "Hit pairs before cuts - scintillator occupancy", maxScinID - minScinID + 1, minScinID - 0.5, maxScinID + 0.5),
      "Scintillator ID", "Number of Hits");

  getStatistics().createHistogramWithAxes(
      new TH1D("cut_stats_a1", "Hits after theta cut - scintillator occupancy", maxScinID - minScinID + 1, minScinID - 0.5, maxScinID + 0.5),
      "Scintillator ID", "Number of Hits");

  getStatistics().createHistogramWithAxes(
      new TH1D("cut_stats_a2", "Hits after theta cut - scintillator occupancy", maxScinID - minScinID + 1, minScinID - 0.5, maxScinID + 0.5),
      "Scintillator ID", "Number of Hits");

  getStatistics().createHistogramWithAxes(new TH1D("cut_stats_tof", "Hits after time difference cut - scintillator occupancy",
                                                   maxScinID - minScinID + 1, minScinID - 0.5, maxScinID + 0.5),
                                          "Scintillator ID", "Number of Hits");

  getStatistics().createHistogramWithAxes(
      new TH1D("cut_stats_tot", "Hits after ToT cut - scintillator occupancy", maxScinID - minScinID + 1, minScinID - 0.5, maxScinID + 0.5),
      "Scintillator ID", "Number of Hits");

  // Cut result on other observables
  // TOF cut
  getStatistics().createHistogramWithAxes(new TH1D("tof_cut_tot", "2 gamma event after TOF cut - average ToT scaled", 201, 0.0, fToTHistoUpperLimit),
                                          "Time over Threshold [ps]", "Number of Hits");

  getStatistics().createHistogramWithAxes(
      new TH1D("tof_cut_ab_tdiff", "2 gamma event after TOF cut - hits A-B time difference", 201, -fMaxTimeDiff, fMaxTimeDiff),
      "A-B Signal Time Difference [ps]", "Number of Hits");

  getStatistics().createHistogramWithAxes(new TH1D("tof_cut_theta", "2 gamma event after TOF cut - theta between flight vectors", 181, -0.5, 180.5),
                                          "Angle [degree]", "Number of Hit Pairs");

  // After theta angle cut - back to back requirement
  getStatistics().createHistogramWithAxes(
      new TH1D("theta_cut_tof", "2 gamma event after theta cut - TOF calculated by convention", 201, -fMaxTimeDiff, fMaxTimeDiff),
      "Time of Flight [ps]", "Number of Hit Pairs");

  getStatistics().createHistogramWithAxes(
      new TH1D("theta_cut_tot", "2 gamma event after theta cut - average ToT scaled", 201, 0.0, fToTHistoUpperLimit), "Time over Threshold [ps]",
      "Number of Hit Pairs");

  getStatistics().createHistogramWithAxes(
      new TH1D("theta_cut_ab_tdiff", "2 gamma event after theta cut - hits A-B time difference", 201, -fMaxTimeDiff, fMaxTimeDiff),
      "A-B Signal Time Difference [ps]", "Number of Hit Pairs");

  // After ToT cut
  getStatistics().createHistogramWithAxes(
      new TH1D("tot_cut_tof", "2 gamma event after ToT cut - TOF calculated by convention", 201, -fMaxTimeDiff, fMaxTimeDiff),
      "A-B Signal Time Difference [ps]", "Number of Hit Pairs");

  getStatistics().createHistogramWithAxes(
      new TH1D("tot_cut_ab_tdiff", "2 gamma event after ToT cut - hits A-B time difference", 201, -fMaxTimeDiff, fMaxTimeDiff),
      "A-B Signal Time Difference [ps]", "Number of Hit Pairs");

  getStatistics().createHistogramWithAxes(new TH1D("tot_cut_theta", "2 gamma event after ToT cut - theta between flight vectors", 181, -0.5, 180.5),
                                          "Angle [degree]", "Number of Hit Pairs");

  // Events after cut - defined as annihilation event
  getStatistics().createHistogramWithAxes(new TH1D("ap_tot", "Annihilation pairs average ToT scaled", 201, 0.0, fToTHistoUpperLimit),
                                          "Time over Threshold [ps]", "Number of Annihilation Pairs");

  getStatistics().createHistogramWithAxes(new TH2D("ap_tot_scin", "Annihilation pairs average ToT scaled per scintillator", maxScinID - minScinID + 1,
                                                   minScinID - 0.5, maxScinID + 0.5, 201, 0.0, fToTHistoUpperLimit),
                                          "Scintillator ID", "Time over Threshold [ps]");

  getStatistics().createHistogramWithAxes(
      new TH1D("ap_ab_tdiff", "Annihilation pairs hits A-B time difference after ToT cut", 201, -fMaxTimeDiff, fMaxTimeDiff),
      "A-B Signal Time Difference [ps]", "Number of Annihilation Pairs");

  getStatistics().createHistogramWithAxes(new TH2D("ap_ab_tdiff_scin", "Annihilation pairs hits A-B time difference per scintillator",
                                                   maxScinID - minScinID + 1, minScinID - 0.5, maxScinID + 0.5, 201, -fMaxTimeDiff, fMaxTimeDiff),
                                          "Scintillator ID", "A-B Signal Time Difference [ps]");

  getStatistics().createHistogramWithAxes(new TH1D("ap_tof", "Annihilation pairs Time of Flight", 201, -fMaxTimeDiff, fMaxTimeDiff),
                                          "Time of Flight [ps]", "Number of Annihilation Pairs");

  getStatistics().createHistogramWithAxes(new TH2D("ap_tof_scin", "Annihilation pairs Time of Flight per scintillator", maxScinID - minScinID + 1,
                                                   minScinID - 0.5, maxScinID + 0.5, 201, -fMaxTimeDiff, fMaxTimeDiff),
                                          "Scintillator ID", "Time of Flight [ps]");

  getStatistics().createHistogramWithAxes(new TH2D("ap_xy", "XY position of annihilation point (bin 0.5 cm)", 202, -50.5, 50.5, 202, -50.5, 50.5),
                                          "X position [cm]", "Y position [cm]");

  getStatistics().createHistogramWithAxes(new TH2D("ap_zx", "ZX position of annihilation point (bin 0.5 cm)", 202, -50.5, 50.5, 202, -50.5, 50.5),
                                          "Z position [cm]", "X position [cm]");

  getStatistics().createHistogramWithAxes(new TH2D("ap_zy", "ZY position of annihilation point (bin 0.5 cm)", 202, -50.5, 50.5, 202, -50.5, 50.5),
                                          "Z position [cm]", "Y position [cm]");

  getStatistics().createHistogramWithAxes(
      new TH2D("ap_xy_zoom", "XY position of annihilation point (bin 0.25 cm)", 132, -16.5, 16.5, 132, -16.5, 16.5), "X position [cm]",
      "Y position [cm]");

  getStatistics().createHistogramWithAxes(
      new TH2D("ap_zx_zoom", "ZX position of annihilation point (bin 0.25 cm)", 132, -16.5, 16.5, 132, -16.5, 16.5), "Z position [cm]",
      "X position [cm]");

  getStatistics().createHistogramWithAxes(
      new TH2D("ap_zy_zoom", "ZY position of annihilation point (bin 0.25 cm)", 132, -16.5, 16.5, 132, -16.5, 16.5), "Z position [cm]",
      "Y position [cm]");
}
