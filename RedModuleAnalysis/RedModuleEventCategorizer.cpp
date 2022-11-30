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

  // Coincidence offset for one-sided calibrations (WLS, ref. module)
  if (isOptionSet(fParams.getOptions(), kCoincidencesOffsetParamKey))
  {
    fCoincidencesOffset = getOptionAsDouble(fParams.getOptions(), kCoincidencesOffsetParamKey);
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

  // Getting bools for saving control and calibration histograms
  if (isOptionSet(fParams.getOptions(), kSaveControlHistosParamKey))
  {
    fSaveControlHistos = getOptionAsBool(fParams.getOptions(), kSaveControlHistosParamKey);
  }

  if (isOptionSet(fParams.getOptions(), kSaveCalibHistosParamKey))
  {
    fSaveCalibHistos = getOptionAsBool(fParams.getOptions(), kSaveCalibHistosParamKey);
  }

  if (isOptionSet(fParams.getOptions(), kFillHistoWithWLSHitsParamKey))
  {
    fFillHistosWithWLSHits = getOptionAsBool(fParams.getOptions(), kFillHistoWithWLSHitsParamKey);
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

      vector<const JPetPhysRecoHit*> wlsHits;
      vector<const JPetPhysRecoHit*> redHits;
      vector<const JPetPhysRecoHit*> refHits;

      for (uint i = 0; i < event.getHits().size(); ++i)
      {
        auto hit = dynamic_cast<const JPetPhysRecoHit*>(event.getHits().at(i));
        if (hit->getScin().getSlot().getID() == 201)
        {
          wlsHits.push_back(hit);
        }
        if (hit->getScin().getSlot().getID() == 202 || hit->getScin().getSlot().getID() == 203)
        {
          redHits.push_back(hit);
        }
        if (hit->getScin().getSlot().getID() == 204)
        {
          refHits.push_back(hit);
        }
      }

      for (auto refHit : refHits)
      {
        for (auto redHit : redHits)
        {
          double tof = EventCategorizerTools::calculateTOF(redHit, refHit);

          if (fSaveControlHistos)
          {
            getStatistics().fillHistogram("2g_red_tot", redHit->getToT());
            getStatistics().fillHistogram("2g_red_tof", tof);
            getStatistics().fillHistogram("2g_red_tof_scin", redHit->getScin().getID(), tof);
            getStatistics().fillHistogram("2g_red_tof_scin", refHit->getScin().getID(), tof);
          }

          if (EventCategorizerTools::checkFor2Gamma(redHit, refHit, getStatistics(), false, f2gThetaDiff, f2gTimeDiff, fToTCutAnniMin, fToTCutAnniMax,
                                                    fSourcePos))
          {
            JPetEvent newEvent = event;
            newEvent.addEventType(JPetEventType::k2Gamma);
            events.push_back(newEvent);

            if (fSaveControlHistos)
            {
              TVector3 annhilationPoint = EventCategorizerTools::calculateAnnihilationPoint(redHit, refHit);

              getStatistics().fillHistogram("ap_red_tof", tof);
              getStatistics().fillHistogram("ap_red_tof_scin", redHit->getScin().getID(), tof);
              getStatistics().fillHistogram("ap_red_tof_scin", refHit->getScin().getID(), tof);

              getStatistics().fillHistogram("ap_red_xy", annhilationPoint.X(), annhilationPoint.Y());
              getStatistics().fillHistogram("ap_red_zx", annhilationPoint.Z(), annhilationPoint.X());
              getStatistics().fillHistogram("ap_red_zy", annhilationPoint.Z(), annhilationPoint.Y());
              getStatistics().fillHistogram("ap_red_xy_zoom", annhilationPoint.X(), annhilationPoint.Y());
              getStatistics().fillHistogram("ap_red_zx_zoom", annhilationPoint.Z(), annhilationPoint.X());
              getStatistics().fillHistogram("ap_red_zy_zoom", annhilationPoint.Z(), annhilationPoint.Y());
            }
          }
        }
        for (auto wlsHit : wlsHits)
        {
          double tof = EventCategorizerTools::calculateTOF(wlsHit, refHit);

          if (fSaveControlHistos)
          {
            getStatistics().fillHistogram("2g_wls_tot", wlsHit->getToT());
            getStatistics().fillHistogram("2g_wls_tof", tof);
            getStatistics().fillHistogram("2g_wls_tof_scin", wlsHit->getScin().getID(), tof);
            getStatistics().fillHistogram("2g_wls_tof_scin", refHit->getScin().getID(), tof);
          }

          if (EventCategorizerTools::checkFor2Gamma(wlsHit, refHit, getStatistics(), false, f2gThetaDiff, f2gTimeDiff, fToTCutAnniMin, fToTCutAnniMax,
                                                    fSourcePos))
          {
            JPetEvent newEvent = event;
            newEvent.addEventType(JPetEventType::k2Gamma);
            events.push_back(newEvent);

            if (fSaveControlHistos)
            {
              TVector3 annhilationPoint = EventCategorizerTools::calculateAnnihilationPoint(wlsHit, refHit);

              getStatistics().fillHistogram("ap_wls_tof", tof);
              getStatistics().fillHistogram("ap_wls_tof_scin", wlsHit->getScin().getID(), tof);
              getStatistics().fillHistogram("ap_wls_tof_scin", refHit->getScin().getID(), tof);

              getStatistics().fillHistogram("ap_wls_xy", annhilationPoint.X(), annhilationPoint.Y());
              getStatistics().fillHistogram("ap_wls_zx", annhilationPoint.Z(), annhilationPoint.X());
              getStatistics().fillHistogram("ap_wls_zy", annhilationPoint.Z(), annhilationPoint.Y());
              getStatistics().fillHistogram("ap_wls_xy_zoom", annhilationPoint.X(), annhilationPoint.Y());
              getStatistics().fillHistogram("ap_wls_zx_zoom", annhilationPoint.Z(), annhilationPoint.X());
              getStatistics().fillHistogram("ap_wls_zy_zoom", annhilationPoint.Z(), annhilationPoint.Y());
            }
          }
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

  // Histograms for 2 gamama events
  getStatistics().createHistogramWithAxes(new TH1D("2g_red_tot", "2 gamma event ToT - red-ref hits", 201, 0.0, fToTHistoUpperLimit),
                                          "Time over Threshold [ps]", "Number of Hits");

  getStatistics().createHistogramWithAxes(new TH1D("2g_wls_tot", "2 gamma event ToT - wls-ref hits", 201, 0.0, fToTHistoUpperLimit),
                                          "Time over Threshold [ps]", "Number of Hits");

  getStatistics().createHistogramWithAxes(new TH1D("2g_red_tof", "2 gamma event TOF - red-ref hits", 201, -fMaxTimeDiff, fMaxTimeDiff),
                                          "Time of Flight [ps]", "Number of Hit Pairs");

  getStatistics().createHistogramWithAxes(new TH1D("2g_wls_tof", "2 gamma event TOF - wls-ref hits", 201, -fMaxTimeDiff, fMaxTimeDiff),
                                          "Time of Flight [ps]", "Number of Hit Pairs");

  getStatistics().createHistogramWithAxes(new TH2D("2g_red_tof_scin", "2 gamma event TOF - red-ref hits per scintillator", maxScinID - minScinID + 1,
                                                   minScinID - 0.5, maxScinID + 0.5, 201, -fMaxTimeDiff, fMaxTimeDiff),
                                          "Scintillator ID", "Time of Flight [ps]");

  getStatistics().createHistogramWithAxes(new TH2D("2g_wls_tof_scin", "2 gamma event TOF - red-ref hits per scintillator", maxScinID - minScinID + 1,
                                                   minScinID - 0.5, maxScinID + 0.5, 201, -fMaxTimeDiff, fMaxTimeDiff),
                                          "Scintillator ID", "Time of Flight [ps]");

  // Events after cut - defined as annihilation event
  getStatistics().createHistogramWithAxes(new TH1D("ap_red_tof", "Red-Ref Annihilation pairs Time of Flight", 201, -fMaxTimeDiff, fMaxTimeDiff),
                                          "Time of Flight [ps]", "Number of Annihilation Pairs");

  getStatistics().createHistogramWithAxes(new TH1D("ap_wls_tof", "WLS-Ref Annihilation pairs Time of Flight", 201, -fMaxTimeDiff, fMaxTimeDiff),
                                          "Time of Flight [ps]", "Number of Annihilation Pairs");

  getStatistics().createHistogramWithAxes(new TH2D("ap_red_tof_scin", "Red-Ref Annihilation pairs Time of Flight per scintillator",
                                                   maxScinID - minScinID + 1, minScinID - 0.5, maxScinID + 0.5, 201, -fMaxTimeDiff, fMaxTimeDiff),
                                          "Scintillator ID", "Time of Flight [ps]");

  getStatistics().createHistogramWithAxes(new TH2D("ap_wls_tof_scin", "WLS-Ref Annihilation pairs Time of Flight per scintillator",
                                                   maxScinID - minScinID + 1, minScinID - 0.5, maxScinID + 0.5, 201, -fMaxTimeDiff, fMaxTimeDiff),
                                          "Scintillator ID", "Time of Flight [ps]");

  getStatistics().createHistogramWithAxes(new TH2D("ap_red_xy", "XY position of annihilation point (bin 0.5 cm)", 202, -50.5, 50.5, 202, -50.5, 50.5),
                                          "X position [cm]", "Y position [cm]");

  getStatistics().createHistogramWithAxes(new TH2D("ap_red_zx", "ZX position of annihilation point (bin 0.5 cm)", 202, -50.5, 50.5, 202, -50.5, 50.5),
                                          "Z position [cm]", "X position [cm]");

  getStatistics().createHistogramWithAxes(new TH2D("ap_red_zy", "ZY position of annihilation point (bin 0.5 cm)", 202, -50.5, 50.5, 202, -50.5, 50.5),
                                          "Z position [cm]", "Y position [cm]");

  getStatistics().createHistogramWithAxes(new TH2D("ap_wls_xy", "XY position of annihilation point (bin 0.5 cm)", 202, -50.5, 50.5, 202, -50.5, 50.5),
                                          "X position [cm]", "Y position [cm]");

  getStatistics().createHistogramWithAxes(new TH2D("ap_wls_zx", "ZX position of annihilation point (bin 0.5 cm)", 202, -50.5, 50.5, 202, -50.5, 50.5),
                                          "Z position [cm]", "X position [cm]");

  getStatistics().createHistogramWithAxes(new TH2D("ap_wls_zy", "ZY position of annihilation point (bin 0.5 cm)", 202, -50.5, 50.5, 202, -50.5, 50.5),
                                          "Z position [cm]", "Y position [cm]");

  getStatistics().createHistogramWithAxes(
      new TH2D("ap_red_xy_zoom", "XY position of annihilation point (bin 0.25 cm)", 132, -16.5, 16.5, 132, -16.5, 16.5), "X position [cm]",
      "Y position [cm]");

  getStatistics().createHistogramWithAxes(
      new TH2D("ap_red_zx_zoom", "ZX position of annihilation point (bin 0.25 cm)", 132, -16.5, 16.5, 132, -16.5, 16.5), "Z position [cm]",
      "X position [cm]");

  getStatistics().createHistogramWithAxes(
      new TH2D("ap_red_zy_zoom", "ZY position of annihilation point (bin 0.25 cm)", 132, -16.5, 16.5, 132, -16.5, 16.5), "Z position [cm]",
      "Y position [cm]");

  getStatistics().createHistogramWithAxes(
      new TH2D("ap_wls_xy_zoom", "XY position of annihilation point (bin 0.25 cm)", 132, -16.5, 16.5, 132, -16.5, 16.5), "X position [cm]",
      "Y position [cm]");

  getStatistics().createHistogramWithAxes(
      new TH2D("ap_wls_zx_zoom", "ZX position of annihilation point (bin 0.25 cm)", 132, -16.5, 16.5, 132, -16.5, 16.5), "Z position [cm]",
      "X position [cm]");

  getStatistics().createHistogramWithAxes(
      new TH2D("ap_wls_zy_zoom", "ZY position of annihilation point (bin 0.25 cm)", 132, -16.5, 16.5, 132, -16.5, 16.5), "Z position [cm]",
      "Y position [cm]");

  // Scattering tests
  getStatistics().createHistogramWithAxes(new TH1D("scatter_test_dist_abs", "Scatter Test - Distance Difference", 201, 0.0, 120.0), "Dist Diff [cm]",
                                          "Number of Hit Pairs");

  getStatistics().createHistogramWithAxes(new TH1D("scatter_test_dist_rel", "Scatter Test - Distance Difference", 201, -120.0, 120.0),
                                          "Dist Diff [cm]", "Number of Hit Pairs");

  getStatistics().createHistogramWithAxes(new TH1D("scatter_test_time_abs", "Scatter Test - Time Difference", 201, 0.0, 10000.0), "Time Diff [ps]",
                                          "Number of Hit Pairs");

  getStatistics().createHistogramWithAxes(new TH1D("scatter_test_time_rel", "Scatter Test - Time Difference", 201, -5000.0, 5000.0), "Time Diff [ps]",
                                          "Number of Hit Pairs");

  getStatistics().createHistogramWithAxes(new TH1D("scatter_test_abs_pass", "Passed Scatter Test - Time Difference", 201, 0.0, 10000.0),
                                          "Time Diff [ps]", "Number of Hit Pairs");

  getStatistics().createHistogramWithAxes(new TH1D("scatter_test_abs_fail", "Failed Scatter Test - Time Difference", 201, 0.0, 10000.0),
                                          "Time Diff [ps]", "Number of Hit Pairs");

  getStatistics().createHistogramWithAxes(new TH1D("scatter_test_rel_pass", "Passed Scatter Test - Time Difference", 201, -5000.0, 5000.0),
                                          "Time Diff [ps]", "Number of Hit Pairs");

  getStatistics().createHistogramWithAxes(new TH1D("scatter_test_rel_fail", "Failed Scatter Test - Time Difference", 201, -5000.0, 5000.0),
                                          "Time Diff [ps]", "Number of Hit Pairs");

  getStatistics().createHistogramWithAxes(
      new TH2D("scatter_angle_time", "Scatter angle vs. scatter test measure", 201, -4000.0, 6000.0, 181, -0.5, 180.5), "Time Diff [ps]",
      "Scatter angle");

  getStatistics().createHistogramWithAxes(
      new TH2D("scatter_angle_time_small", "Scatter angle vs. scatter test measure", 201, -4000.0, 6000.0, 41, 139.5, 180.5), "Time Diff [ps]",
      "Scatter angle");

  getStatistics().createHistogramWithAxes(
      new TH2D("scatter_angle_time_pass", "Passed Scatter angle vs. scatter test measure", 201, -4000.0, 6000.0, 181, -0.5, 180.5), "Time Diff [ps]",
      "Scatter angle");

  getStatistics().createHistogramWithAxes(
      new TH2D("scatter_angle_time_fail", "Failed Scatter angle vs. scatter test measure", 201, -4000.0, 6000.0, 181, -0.5, 180.5), "Time Diff [ps]",
      "Scatter angle");
}
