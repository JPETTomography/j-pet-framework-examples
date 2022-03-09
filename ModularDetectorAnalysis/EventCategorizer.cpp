/**
 *  @copyright Copyright 2021 The J-PET Framework Authors. All rights reserved.
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
 *  @file EventCategorizer.cpp
 */

#include "EventCategorizer.h"
#include "CalibrationTools.h"
#include "EventCategorizerTools.h"
#include <JPetOptionsTools/JPetOptionsTools.h>
#include <JPetWriter/JPetWriter.h>
#include <boost/property_tree/json_parser.hpp>
#include <iostream>

using namespace jpet_options_tools;
using namespace std;

EventCategorizer::EventCategorizer(const char* name) : JPetUserTask(name) {}

EventCategorizer::~EventCategorizer() {}

bool EventCategorizer::init()
{
  INFO("Event categorization started.");

  // Reading user parameters
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

  if (isOptionSet(fParams.getOptions(), kToTCutDeexMinParamKey))
  {
    fToTCutDeexMin = getOptionAsDouble(fParams.getOptions(), kToTCutDeexMinParamKey);
  }
  else
  {
    WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.", kToTCutDeexMinParamKey.c_str(), fToTCutDeexMin));
  }

  if (isOptionSet(fParams.getOptions(), kToTCutDeexMaxParamKey))
  {
    fToTCutDeexMax = getOptionAsDouble(fParams.getOptions(), kToTCutDeexMaxParamKey);
  }
  else
  {
    WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.", kToTCutDeexMaxParamKey.c_str(), fToTCutDeexMax));
  }

  // For plotting ToT histograms
  if (isOptionSet(fParams.getOptions(), kToTHistoUpperLimitParamKey))
  {
    fToTHistoUpperLimit = getOptionAsDouble(fParams.getOptions(), kToTHistoUpperLimitParamKey);
  }

  // Cuts around source position
  if (isOptionSet(fParams.getOptions(), kSourceDistCutXYParamKey))
  {
    fSourceDistXYCut = getOptionAsDouble(fParams.getOptions(), kSourceDistCutXYParamKey);
  }
  else
  {
    WARNING(
        Form("No value of the %s parameter provided by the user. Using default value of %lf.", kSourceDistCutXYParamKey.c_str(), fSourceDistXYCut));
  }

  if (isOptionSet(fParams.getOptions(), kSourceDistCutZParamKey))
  {
    fSourceDistZCut = getOptionAsDouble(fParams.getOptions(), kSourceDistCutZParamKey);
  }
  else
  {
    WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.", kSourceDistCutZParamKey.c_str(), fSourceDistZCut));
  }

  // LOR cuts
  if (isOptionSet(fParams.getOptions(), kLORAngleCutParamKey))
  {
    fLORAngleCut = getOptionAsDouble(fParams.getOptions(), kLORAngleCutParamKey);
  }
  else
  {
    WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.", kLORAngleCutParamKey.c_str(), fLORAngleCut));
  }

  if (isOptionSet(fParams.getOptions(), kLORPosZCutParamKey))
  {
    fLORPosZCut = getOptionAsDouble(fParams.getOptions(), kLORPosZCutParamKey);
  }
  else
  {
    WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.", kLORPosZCutParamKey.c_str(), fLORPosZCut));
  }

  // Source position
  if (isOptionSet(fParams.getOptions(), kSourcePosXParamKey) && isOptionSet(fParams.getOptions(), kSourcePosYParamKey) &&
      isOptionSet(fParams.getOptions(), kSourcePosZParamKey))
  {
    auto x = getOptionAsDouble(fParams.getOptions(), kSourcePosXParamKey);
    auto y = getOptionAsDouble(fParams.getOptions(), kSourcePosYParamKey);
    auto z = getOptionAsDouble(fParams.getOptions(), kSourcePosZParamKey);
    fSourcePos.SetXYZ(x, y, z);
    INFO(Form("Source position is: %lf, %lf, %lf", x, y, z));
  }
  else
  {
    fSourcePos.SetXYZ(0.0, 0.0, 0.0);
    INFO("Source is positioned in (0, 0, 0).");
  }

  // Reading file with constants to property tree
  if (isOptionSet(fParams.getOptions(), kConstantsFileParamKey))
  {
    boost::property_tree::read_json(getOptionAsString(fParams.getOptions(), kConstantsFileParamKey), fConstansTree);
  }

  // Set the type of scatter test - default is simple parameter cut
  if (isOptionSet(fParams.getOptions(), kScatterTestTypeParamKey))
  {
    auto type = getOptionAsString(fParams.getOptions(), kScatterTestTypeParamKey);
    if (type == "simple_param")
    {
      fTestType = EventCategorizerTools::kSimpleParam;
    }
    else if (type == "min_max")
    {
      fTestType = EventCategorizerTools::kMinMaxParams;
    }
    else if (type == "lorentz_expo")
    {
      fTestType = EventCategorizerTools::kLorentzExponent;
    }
    else if (type == "gaus_expo")
    {
      fTestType = EventCategorizerTools::kGaussExponent;
    }
    else if (type == "landau_expo")
    {
      fTestType = EventCategorizerTools::kLandauExponent;
    }
  }

  if (isOptionSet(fParams.getOptions(), kScatterTOFTimeDiffParamKey))
  {
    fScatterTOFTimeDiff = getOptionAsDouble(fParams.getOptions(), kScatterTOFTimeDiffParamKey);
  }

  if (isOptionSet(fParams.getOptions(), kScatterTimeMinParamKey))
  {
    fScatterTimeMin = getOptionAsDouble(fParams.getOptions(), kScatterTimeMinParamKey);
  }
  if (isOptionSet(fParams.getOptions(), kScatterTimeMaxParamKey))
  {
    fScatterTimeMax = getOptionAsDouble(fParams.getOptions(), kScatterTimeMaxParamKey);
  }

  if (isOptionSet(fParams.getOptions(), kScatterAngleMinParamKey))
  {
    fScatterAngleMin = getOptionAsDouble(fParams.getOptions(), kScatterAngleMinParamKey);
  }
  if (isOptionSet(fParams.getOptions(), kScatterAngleMaxParamKey))
  {
    fScatterAngleMax = getOptionAsDouble(fParams.getOptions(), kScatterAngleMaxParamKey);
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

  // Variable used in measurements with Trento setup - rotation of Z axis with respect of
  // vertical direction in Earth frame
  if (isOptionSet(fParams.getOptions(), kDetectorYRotation))
  {
    fDetectorYRotationDeg = getOptionAsDouble(fParams.getOptions(), kDetectorYRotation);
  }
  else
  {
    WARNING(
        Form("No value of the %s parameter provided by the user. Using default value of %lf.", kDetectorYRotation.c_str(), fDetectorYRotationDeg));
  }

  if (isOptionSet(fParams.getOptions(), kCosmicMaxThetaDeg))
  {
    fCosmicMaxThetaDiffDeg = getOptionAsDouble(fParams.getOptions(), kCosmicMaxThetaDeg);
  }
  else
  {
    WARNING(
        Form("No value of the %s parameter provided by the user. Using default value of %lf.", kCosmicMaxThetaDeg.c_str(), fCosmicMaxThetaDiffDeg));
  }

  // Getting bools for saving histograms
  if (isOptionSet(fParams.getOptions(), kSaveControlHistosParamKey))
  {
    fSaveControlHistos = getOptionAsBool(fParams.getOptions(), kSaveControlHistosParamKey);
  }
  if (isOptionSet(fParams.getOptions(), kSaveCalibHistosParamKey))
  {
    fSaveCalibHistos = getOptionAsBool(fParams.getOptions(), kSaveCalibHistosParamKey);
  }
  if (isOptionSet(fParams.getOptions(), kTrentoCalibrationParamKey))
  {
    fTrentoCalibHistos = getOptionAsBool(fParams.getOptions(), kTrentoCalibrationParamKey);
  }

  // Input events type
  fOutputEvents = new JPetTimeWindow("JPetEvent");

  // Initialise hisotgrams
  if (fSaveControlHistos)
  {
    initialiseHistograms();
  }

  if (fSaveControlHistos)
  {
    initialiseCalibrationHistograms(fTrentoCalibHistos);
  }
  return true;
}

bool EventCategorizer::exec()
{
  if (auto timeWindow = dynamic_cast<const JPetTimeWindow* const>(fEvent))
  {
    vector<JPetEvent> events;
    for (uint i = 0; i < timeWindow->getNumberOfEvents(); i++)
    {
      const auto& event = dynamic_cast<const JPetEvent&>(timeWindow->operator[](i));

      if (fSaveCalibHistos)
      {
        // Method evauated fot Trento setup
        if (fTrentoCalibHistos)
        {
          CalibrationTools::selectCosmicsForToF(event, getStatistics(), fSaveControlHistos, fCosmicMaxThetaDiffDeg, fDetectorYRotationDeg);
        }
        else
        {
          CalibrationTools::selectForTOF(event, getStatistics(), fSaveControlHistos, fToTCutAnniMin, fToTCutAnniMax, fToTCutDeexMin, fToTCutDeexMax,
                                         fTestType, fScatterTOFTimeDiff, fScatterTimeMin, fScatterTimeMax, fScatterAngleMin, fScatterAngleMax,
                                         fConstansTree);

          CalibrationTools::selectForTimeWalk(event, getStatistics(), fSaveControlHistos, f2gThetaDiff, f2gTimeDiff, fToTCutAnniMin, fToTCutAnniMax,
                                              fSourcePos, fTestType, fScatterTOFTimeDiff, fScatterTimeMin, fScatterTimeMax, fScatterAngleMin,
                                              fScatterAngleMax, fConstansTree);
        }
      }

      bool is2Gamma = EventCategorizerTools::checkFor2Gamma(event, getStatistics(), fSaveControlHistos, f2gThetaDiff, f2gTimeDiff, fToTCutAnniMin,
                                                            fToTCutAnniMax, fSourcePos, fTestType, fScatterTOFTimeDiff, fScatterTimeMin,
                                                            fScatterTimeMax, fScatterAngleMin, fScatterAngleMax, fConstansTree);

      // Select hits for TOF calibration, if making calibraiton

      // Selection of other type of events is currently not used
      // bool is3Gamma = EventCategorizerTools::checkFor3Gamma(
      //   event, getStatistics(), fSaveControlHistos
      // );
      // bool isPrompt = EventCategorizerTools::checkForPrompt(
      //   event, getStatistics(), fSaveControlHistos, fToTCutDeexMin, fToTCutDeexMax
      // );
      // bool isScattered = EventCategorizerTools::checkForScatter(
      //   event, getStatistics(), fSaveControlHistos, fScatterTOFTimeDiff
      // );

      // JPetEvent newEvent = event;
      // if (is2Gamma)
      // {
      //   newEvent.addEventType(JPetEventType::k2Gamma);
      //   events.push_back(newEvent);
      // }
      // if(is3Gamma) newEvent.addEventType(JPetEventType::k3Gamma);
      // if(isPrompt) newEvent.addEventType(JPetEventType::kPrompt);
      // if(isScattered) newEvent.addEventType(JPetEventType::kScattered);
      //
      // if(fSaveControlHistos){
      //   for(auto hit : event.getHits()){
      //     getStatistics().getHisto2D("All_XYpos")->Fill(hit.getPosX(), hit.getPosY());
      //   }
      // }
    }
    saveEvents(events);
  }
  else
  {
    return false;
  }
  return true;
}

bool EventCategorizer::terminate()
{
  INFO("Event categorization completed.");
  return true;
}

void EventCategorizer::saveEvents(const vector<JPetEvent>& events)
{
  for (const auto& event : events)
  {
    fOutputEvents->add<JPetEvent>(event);
  }
}

void EventCategorizer::initialiseHistograms()
{
  auto minScinID = getParamBank().getScins().begin()->first;
  auto maxScinID = getParamBank().getScins().rbegin()->first;

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

  getStatistics().createHistogramWithAxes(new TH2D("ap_yx", "YX position of annihilation point (bin 0.5 cm)", 202, -50.5, 50.5, 202, -50.5, 50.5),
                                          "Y position [cm]", "X position [cm]");

  getStatistics().createHistogramWithAxes(new TH2D("ap_zx", "ZX position of annihilation point (bin 0.5 cm)", 202, -50.5, 50.5, 202, -50.5, 50.5),
                                          "Z position [cm]", "X position [cm]");

  getStatistics().createHistogramWithAxes(new TH2D("ap_zy", "ZY position of annihilation point (bin 0.5 cm)", 202, -50.5, 50.5, 202, -50.5, 50.5),
                                          "Z position [cm]", "Y position [cm]");

  getStatistics().createHistogramWithAxes(
      new TH2D("ap_yx_zoom", "YX position of annihilation point (bin 0.25 cm)", 132, -16.5, 16.5, 132, -16.5, 16.5), "Y position [cm]",
      "X position [cm]");

  getStatistics().createHistogramWithAxes(
      new TH2D("ap_zx_zoom", "ZX position of annihilation point (bin 0.25 cm)", 132, -16.5, 16.5, 132, -16.5, 16.5), "Z position [cm]",
      "X position [cm]");

  getStatistics().createHistogramWithAxes(
      new TH2D("ap_zy_zoom", "ZY position of annihilation point (bin 0.25 cm)", 132, -16.5, 16.5, 132, -16.5, 16.5), "Z position [cm]",
      "Y position [cm]");

  // Histograms for scattering category
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

void EventCategorizer::initialiseCalibrationHistograms(bool includeTrento)
{
  auto minScinID = getParamBank().getScins().begin()->first;
  auto maxScinID = getParamBank().getScins().rbegin()->first;

  // Synchronization of TOF with annihilaion-deexcitation pairs
  getStatistics().createHistogramWithAxes(new TH2D("tdiff_anni_scin", "A-D time difference for annihilation hit per scin", maxScinID - minScinID + 1,
                                                   minScinID - 0.5, maxScinID + 0.5, 200, -fMaxTimeDiff, fMaxTimeDiff),
                                          "Scin ID", "Time diffrence [ps]");

  getStatistics().createHistogramWithAxes(new TH2D("tdiff_deex_scin", "A-D time difference for deex hit per scin", maxScinID - minScinID + 1,
                                                   minScinID - 0.5, maxScinID + 0.5, 200, -fMaxTimeDiff, fMaxTimeDiff),
                                          "Scin ID", "Time diffrence [ps]");

  // Time walk histograms
  double revToTLimit = 0.000000015;

  getStatistics().createHistogramWithAxes(
      new TH2D("time_walk_ab_tdiff", "AB TDiff vs. reversed ToT", 200, -fMaxTimeDiff / 2.0, fMaxTimeDiff / 2.0, 200, -revToTLimit, revToTLimit),
      "AB Time Difference [ps]", "Reversed ToT [1/ps]");

  getStatistics().createHistogramWithAxes(new TH2D("time_walk_ab_tdiff_z_cut", "AB TDiff vs. reversed ToT for hits in the center of the strips", 200,
                                                   -fMaxTimeDiff / 2.0, fMaxTimeDiff / 2.0, 200, -revToTLimit, revToTLimit),
                                          "AB Time Difference [ps]", "Reversed ToT [1/ps]");

  getStatistics().createHistogramWithAxes(
      new TH2D("time_walk_tof", "TOF vs. reversed ToT", 200, -fMaxTimeDiff / 2.0, fMaxTimeDiff / 2.0, 200, -revToTLimit, revToTLimit),
      "Time of Flight [ps]", "Reversed ToT [1/ps]");

  getStatistics().createHistogramWithAxes(new TH2D("time_walk_tof_z_cut", "TOF vs. reversed ToT for hits in the center of the strips", 200,
                                                   -fMaxTimeDiff / 2.0, fMaxTimeDiff / 2.0, 200, -revToTLimit, revToTLimit),
                                          "Time of Flight [ps]", "Reversed ToT [1/ps]");

  auto minChannelID = getParamBank().getChannels().begin()->first;
  auto maxChannelID = getParamBank().getChannels().rbegin()->first;

  getStatistics().createHistogramWithAxes(new TH2D("evtcat_channel_offsets", "Offset of Channel in Matrix vs. Channel ID in annihilation hits",
                                                   maxChannelID - minChannelID + 1, minChannelID - 0.5, maxChannelID + 0.5, 200, -3000.0, 3000.0),
                                          "Channel ID", "Offset");

  if (includeTrento)
  {
    // Cosmic ToF - histograms for Trento setup
    for (int scinID = 201; scinID <= 226; ++scinID)
    {
      getStatistics().createHistogramWithAxes(new TH2D(Form("cosmic_tof_tdiff_scin_%d_all", scinID),
                                                       Form("Time of Flight between hits from scin ID %d and from layer below", scinID), 26, 200.5,
                                                       226.5, 200, -fMaxTimeDiff, fMaxTimeDiff),
                                              "Scintillator ID", "Time of Flight [ps]");

      getStatistics().createHistogramWithAxes(new TH2D(Form("cosmic_tof_tdiff_scin_%d_cut", scinID),
                                                       Form("Time of Flight between hits from scin ID %d and from layer below", scinID), 26, 200.5,
                                                       226.5, 200, -fMaxTimeDiff, fMaxTimeDiff),
                                              "Scintillator ID", "Time of Flight [ps]");

      getStatistics().createHistogramWithAxes(new TH2D(Form("cosmic_tof_offset_scin_%d_all", scinID),
                                                       Form("Time of Flight between hits from scin ID %d and from layer below", scinID), 26, 200.5,
                                                       226.5, 200, -fMaxTimeDiff, fMaxTimeDiff),
                                              "Scintillator ID", "Time of Flight [ps]");

      getStatistics().createHistogramWithAxes(new TH2D(Form("cosmic_tof_offset_scin_%d_cut", scinID),
                                                       Form("Time of Flight between hits from scin ID %d and from layer below", scinID), 26, 200.5,
                                                       226.5, 200, -fMaxTimeDiff, fMaxTimeDiff),
                                              "Scintillator ID", "Time of Flight [ps]");
    }

    getStatistics().createHistogramWithAxes(new TH1D("cosmic_hits_x_diff_all", "X-Position difference of two comsic hits", 120, -30.0, 30.0),
                                            "positon diff [cm]", "Number of pairs");
    getStatistics().createHistogramWithAxes(
        new TH1D("cosmic_hits_x_diff_cut", "X-Position difference of two comsic hits after angle cut", 120, -30.0, 30.0), "positon diff [cm]",
        "Number of pairs");

    getStatistics().createHistogramWithAxes(new TH1D("cosmic_hits_z_diff_all", "Z-Position difference of two comsic hits", 120, -30.0, 30.0),
                                            "positon diff [cm]", "Number of pairs");
    getStatistics().createHistogramWithAxes(
        new TH1D("cosmic_hits_z_diff_cut", "Z-Position difference of two comsic hits after angle cut", 120, -30.0, 30.0), "positon diff [cm]",
        "Number of pairs");

    getStatistics().createHistogramWithAxes(new TH1D("cosmic_hits_y_diff_all", "Y-Position difference of two comsic hits", 120, -30.0, 30.0),
                                            "positon diff [cm]", "Number of pairs");
    getStatistics().createHistogramWithAxes(
        new TH1D("cosmic_hits_y_diff_cut", "Y-Position difference of two comsic hits after angle cut", 120, -30.0, 30.0), "positon diff [cm]",
        "Number of pairs");

    getStatistics().createHistogramWithAxes(new TH1D("cosmic_hits_theta_xz_all", "Theta of two comsic hits", 360, 0.0, 360.0), "theta [deg]",
                                            "Number of pairs");
    getStatistics().createHistogramWithAxes(new TH1D("cosmic_hits_theta_xz_cut", "Theta of two comsic hits", 360, 0.0, 360.0), "theta [deg]",
                                            "Number of pairs");

    getStatistics().createHistogramWithAxes(new TH1D("cosmic_hits_theta_xy_all", "Theta of two comsic hits", 360, 0.0, 360.0), "theta [deg]",
                                            "Number of pairs");
    getStatistics().createHistogramWithAxes(new TH1D("cosmic_hits_theta_xy_cut", "Theta of two comsic hits", 360, 0.0, 360.0), "theta [deg]",
                                            "Number of pairs");
  }
}
