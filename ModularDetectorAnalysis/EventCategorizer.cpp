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

#include <boost/property_tree/json_parser.hpp>

#include "EventCategorizer.h"
#include "EventCategorizerTools.h"
#include <JPetOptionsTools/JPetOptionsTools.h>
#include <JPetWriter/JPetWriter.h>
#include <iostream>

using namespace jpet_options_tools;
using namespace std;

EventCategorizer::EventCategorizer(const char* name) : JPetUserTask(name) {}

EventCategorizer::~EventCategorizer() {}

bool EventCategorizer::init()
{
  INFO("Event categorization started.");
  // Parameter for back to back categorization
  if (isOptionSet(fParams.getOptions(), kBack2BackSlotThetaDiffParamKey))
  {
    fB2BSlotThetaDiff = getOptionAsDouble(fParams.getOptions(), kBack2BackSlotThetaDiffParamKey);
  }
  else
  {
    WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.", kBack2BackSlotThetaDiffParamKey.c_str(),
                 fB2BSlotThetaDiff));
  }

  // Parameter for scattering determination
  if (isOptionSet(fParams.getOptions(), kScatterTOFTimeDiffParamKey))
  {
    fScatterTOFTimeDiff = getOptionAsDouble(fParams.getOptions(), kScatterTOFTimeDiffParamKey);
  }
  else
  {
    WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.", kScatterTOFTimeDiffParamKey.c_str(),
                 fScatterTOFTimeDiff));
  }

  // Reading file with constants to property tree
  if (isOptionSet(fParams.getOptions(), kConstantsFileParamKey))
  {
    boost::property_tree::read_json(getOptionAsString(fParams.getOptions(), kConstantsFileParamKey), fConstansTree);
  }

  if (isOptionSet(fParams.getOptions(), kMaxTimeDiffParamKey))
  {
    fMaxTimeDiff = getOptionAsDouble(fParams.getOptions(), kMaxTimeDiffParamKey);
  }
  else
  {
    WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.", kMaxTimeDiffParamKey.c_str(), fMaxTimeDiff));
  }

  // Reading TOT cut values
  if (isOptionSet(fParams.getOptions(), kTOTCutAnniMinParamKey))
  {
    fTOTCutAnniMin = getOptionAsDouble(fParams.getOptions(), kTOTCutAnniMinParamKey);
  }
  else
  {
    WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.", kTOTCutAnniMinParamKey.c_str(), fTOTCutAnniMin));
  }

  if (isOptionSet(fParams.getOptions(), kTOTCutAnniMaxParamKey))
  {
    fTOTCutAnniMax = getOptionAsDouble(fParams.getOptions(), kTOTCutAnniMaxParamKey);
  }
  else
  {
    WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.", kMaxTimeDiffParamKey.c_str(), fTOTCutAnniMax));
  }

  if (isOptionSet(fParams.getOptions(), kTOTCutDeexMinParamKey))
  {
    fTOTCutDeexMin = getOptionAsDouble(fParams.getOptions(), kTOTCutDeexMinParamKey);
  }
  else
  {
    WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.", kMaxTimeDiffParamKey.c_str(), fTOTCutDeexMin));
  }

  if (isOptionSet(fParams.getOptions(), kTOTCutDeexMaxParamKey))
  {
    fTOTCutDeexMax = getOptionAsDouble(fParams.getOptions(), kTOTCutDeexMaxParamKey);
  }
  else
  {
    WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.", kMaxTimeDiffParamKey.c_str(), fTOTCutDeexMax));
  }
  // Getting bool for saving histograms
  if (isOptionSet(fParams.getOptions(), kSaveControlHistosParamKey))
  {
    fSaveControlHistos = getOptionAsBool(fParams.getOptions(), kSaveControlHistosParamKey);
  }
  if (isOptionSet(fParams.getOptions(), kSaveCalibHistosParamKey))
  {
    fSaveCalibHistos = getOptionAsBool(fParams.getOptions(), kSaveCalibHistosParamKey);
  }

  // Input events type
  fOutputEvents = new JPetTimeWindow("JPetEvent");
  // Initialise hisotgrams
  if (fSaveControlHistos)
    initialiseHistograms();
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

      // Check types of current event
      bool is2Gamma = EventCategorizerTools::checkFor2Gamma(event, getStatistics(), fSaveControlHistos, fSaveCalibHistos, fB2BSlotThetaDiff,
                                                            fMaxTimeDiff, fTOTCutAnniMin, fTOTCutAnniMax, fConstansTree);

      // Select hits for TOF calibration, if making calibraiton
      if (fSaveCalibHistos)
      {
        EventCategorizerTools::selectForCalibration(event, getStatistics(), fSaveControlHistos, fSaveCalibHistos, fTOTCutAnniMin, fTOTCutAnniMax,
                                                    fTOTCutDeexMin, fTOTCutDeexMax, fConstansTree);
      }

      // bool is3Gamma = EventCategorizerTools::checkFor3Gamma(
      //   event, getStatistics(), fSaveControlHistos
      // );
      // bool isPrompt = EventCategorizerTools::checkForPrompt(
      //   event, getStatistics(), fSaveControlHistos, fTOTCutDeexMin, fTOTCutDeexMax
      // );
      // bool isScattered = EventCategorizerTools::checkForScatter(
      //   event, getStatistics(), fSaveControlHistos, fScatterTOFTimeDiff
      // );

      JPetEvent newEvent = event;
      if (is2Gamma)
        newEvent.addEventType(JPetEventType::k2Gamma);
      // if(is3Gamma) newEvent.addEventType(JPetEventType::k3Gamma);
      // if(isPrompt) newEvent.addEventType(JPetEventType::kPrompt);
      // if(isScattered) newEvent.addEventType(JPetEventType::kScattered);
      //
      // if(fSaveControlHistos){
      //   for(auto hit : event.getHits()){
      //     getStatistics().getHisto2D("All_XYpos")->Fill(hit.getPosX(), hit.getPosY());
      //   }
      // }
      events.push_back(newEvent);
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

  auto minSiPMID = getParamBank().getPMs().begin()->first;
  auto maxSiPMID = getParamBank().getPMs().rbegin()->first;

  // Histograms for 2Gamama selection
  getStatistics().createHistogram(new TH1F("2g_tdiff", "2 gamma event - registration time difference", 200, 0.0, 1.1 * fMaxTimeDiff));
  getStatistics().getHisto1D("2g_tdiff")->GetXaxis()->SetTitle("Time difference [ps]");
  getStatistics().getHisto1D("2g_tdiff")->GetYaxis()->SetTitle("Number of Hit Pairs");

  getStatistics().createHistogram(new TH1F("2g_tot", "2 gamma event average TOT scaled", 100, 0.0, 400000.0));
  getStatistics().getHisto1D("2g_tot")->GetXaxis()->SetTitle("Time over Threshold [ps]");
  getStatistics().getHisto1D("2g_tot")->GetYaxis()->SetTitle("Number of Hits");

  getStatistics().createHistogram(new TH2F("2g_tot_scin", "2 gamma event average TOT scaled per scin", maxScinID - minScinID + 1, minScinID - 0.5,
                                           maxScinID + 0.5, 200, 0.0, 400000.0));
  getStatistics().getHisto2D("2g_tot_scin")->GetXaxis()->SetTitle("Reversd Time over Threshold [1/ps]");
  getStatistics().getHisto2D("2g_tot_scin")->GetYaxis()->SetTitle("Number of Hits");

  getStatistics().createHistogram(new TH2F("2g_revtot_scin", "2 gamma event reversed TOT per scin", maxScinID - minScinID + 1, minScinID - 0.5,
                                           maxScinID + 0.5, 200, -0.0001, 0.0001));
  getStatistics().getHisto2D("2g_revtot_scin")->GetXaxis()->SetTitle("Reversd Time over Threshold [1/ps]");
  getStatistics().getHisto2D("2g_revtot_scin")->GetYaxis()->SetTitle("Number of Hits");

  // TOF for hit pairs
  getStatistics().createHistogram(new TH1F("2g_tof_conv", "2 gamma event TOF calculated by convention", 200, -15000.0, 15000.0));
  getStatistics().getHisto1D("2g_tof_conv")->GetXaxis()->SetTitle("Time of Flight [ps]");
  getStatistics().getHisto1D("2g_tof_conv")->GetYaxis()->SetTitle("Number of Hit Pairs");

  getStatistics().createHistogram(new TH1F("2g_tof_conv_corr", "2 gamma event TOF by convention with corrections", 200, -15000.0, 15000.0));
  getStatistics().getHisto1D("2g_tof_conv_corr")->GetXaxis()->SetTitle("Time of Flight [ps]");
  getStatistics().getHisto1D("2g_tof_conv_corr")->GetYaxis()->SetTitle("Number of Hit Pairs");

  getStatistics().createHistogram(new TH1F("2g_hit_tdiff", "Opposite module hits A-B time difference", 100, -15000.0, 15000.0));
  getStatistics().getHisto1D("2g_hit_tdiff")->GetXaxis()->SetTitle("A-B Signal Time Difference [ps]");
  getStatistics().getHisto1D("2g_hit_tdiff")->GetYaxis()->SetTitle("Number of Hit Pairs");

  // Cut stats
  getStatistics().createHistogram(
      new TH1F("2g_hit_scin", "2 hit event - scintillator occupancy", maxScinID - minScinID + 1, minScinID - 0.5, maxScinID + 0.5));
  getStatistics().getHisto1D("2g_hit_scin")->GetXaxis()->SetTitle("Scin ID");
  getStatistics().getHisto1D("2g_hit_scin")->GetYaxis()->SetTitle("Number of Hits");

  getStatistics().createHistogram(
      new TH1F("cut_stats_a1", "Hits after angle cut - scintillator occupancy", maxScinID - minScinID + 1, minScinID - 0.5, maxScinID + 0.5));
  getStatistics().getHisto1D("cut_stats_a1")->GetXaxis()->SetTitle("Scin ID");
  getStatistics().getHisto1D("cut_stats_a1")->GetYaxis()->SetTitle("Number of Hits");

  getStatistics().createHistogram(
      new TH1F("cut_stats_a2", "Hits after angle cut - scintillator occupancy", maxScinID - minScinID + 1, minScinID - 0.5, maxScinID + 0.5));
  getStatistics().getHisto1D("cut_stats_a2")->GetXaxis()->SetTitle("Scin ID");
  getStatistics().getHisto1D("cut_stats_a2")->GetYaxis()->SetTitle("Number of Hits");

  getStatistics().createHistogram(new TH1F("cut_stats_tdiff", "Hits after time difference cut - scintillator occupancy", maxScinID - minScinID + 1,
                                           minScinID - 0.5, maxScinID + 0.5));
  getStatistics().getHisto1D("cut_stats_tdiff")->GetXaxis()->SetTitle("Scin ID");
  getStatistics().getHisto1D("cut_stats_tdiff")->GetYaxis()->SetTitle("Number of Hits");

  getStatistics().createHistogram(
      new TH1F("cut_stats_tot", "Hits after TOT cut - scintillator occupancy", maxScinID - minScinID + 1, minScinID - 0.5, maxScinID + 0.5));
  getStatistics().getHisto1D("cut_stats_tot")->GetXaxis()->SetTitle("Scin ID");
  getStatistics().getHisto1D("cut_stats_tot")->GetYaxis()->SetTitle("Number of Hits");

  // Events after cut - defined as annihilation event
  getStatistics().createHistogram(new TH2F("ap_revtot_scin", "Annihilation pairs reversed TOT per scin", maxScinID - minScinID + 1, minScinID - 0.5,
                                           maxScinID + 0.5, 200, -0.0001, 0.0001));
  getStatistics().getHisto2D("ap_revtot_scin")->GetXaxis()->SetTitle("Reversd Time over Threshold [1/ps]");
  getStatistics().getHisto2D("ap_revtot_scin")->GetYaxis()->SetTitle("Number of Hits");

  getStatistics().createHistogram(new TH1F("ap_hit_tdiff", "Annihilation pairs hits A-B time difference after TOT cut", 100, -15000.0, 15000.0));
  getStatistics().getHisto1D("ap_hit_tdiff")->GetXaxis()->SetTitle("A-B Signal Time Difference [ps]");
  getStatistics().getHisto1D("ap_hit_tdiff")->GetYaxis()->SetTitle("Number of Hit Pairs");

  getStatistics().createHistogram(new TH1F("ap_tof", "Annihilation pairs Time of Flight", 200, -15000.0, 15000.0));
  getStatistics().getHisto1D("ap_tof")->GetXaxis()->SetTitle("Time of Flight [ps]");
  getStatistics().getHisto1D("ap_tof")->GetYaxis()->SetTitle("Number of Annihilation Pairs");

  getStatistics().createHistogram(new TH1F("ap_tof_conv", "Annihilation pairs Time of Flight", 200, -15000.0, 15000.0));
  getStatistics().getHisto1D("ap_tof_conv")->GetXaxis()->SetTitle("Time of Flight [ps]");
  getStatistics().getHisto1D("ap_tof_conv")->GetYaxis()->SetTitle("Number of Annihilation Pairs");

  getStatistics().createHistogram(new TH1F("ap_tof_corr", "Annihilation pairs Time of Flight", 200, -15000.0, 15000.0));
  getStatistics().getHisto1D("ap_tof_corr")->GetXaxis()->SetTitle("Time of Flight [ps]");
  getStatistics().getHisto1D("ap_tof_corr")->GetYaxis()->SetTitle("Number of Annihilation Pairs");

  getStatistics().createHistogram(new TH1F("ap_tof_conv_corr", "Annihilation pairs Time of Flight", 200, -15000.0, 15000.0));
  getStatistics().getHisto1D("ap_tof_conv_corr")->GetXaxis()->SetTitle("Time of Flight [ps]");
  getStatistics().getHisto1D("ap_tof_conv_corr")->GetYaxis()->SetTitle("Number of Annihilation Pairs");

  getStatistics().createHistogram(new TH2F("ap_xy", "XY position of annihilation point", 101, -50.5, 50.5, 101, -50.5, 50.5));
  getStatistics().getHisto2D("ap_xy")->GetXaxis()->SetTitle("Y position [cm]");
  getStatistics().getHisto2D("ap_xy")->GetYaxis()->SetTitle("X position [cm]");

  getStatistics().createHistogram(new TH2F("ap_xz", "XZ position of annihilation point", 101, -50.5, 50.5, 101, -50.5, 50.5));
  getStatistics().getHisto2D("ap_xz")->GetXaxis()->SetTitle("X position [cm]");
  getStatistics().getHisto2D("ap_xz")->GetYaxis()->SetTitle("Z position [cm]");

  getStatistics().createHistogram(new TH2F("ap_yz", "YZ position of annihilation point", 101, -50.5, 50.5, 101, -50.5, 50.5));
  getStatistics().getHisto2D("ap_yz")->GetXaxis()->SetTitle("Y position [cm]");
  getStatistics().getHisto2D("ap_yz")->GetYaxis()->SetTitle("Z position [cm]");

  getStatistics().createHistogram(new TH2F("ap_xy_zoom", "XY position of annihilation point", 101, -10.5, 10.5, 101, -10.5, 10.5));
  getStatistics().getHisto2D("ap_xy_zoom")->GetXaxis()->SetTitle("Y position [cm]");
  getStatistics().getHisto2D("ap_xy_zoom")->GetYaxis()->SetTitle("X position [cm]");

  getStatistics().createHistogram(new TH2F("ap_xz_zoom", "XZ position of annihilation point", 101, -10.5, 10.5, 101, -10.5, 10.5));
  getStatistics().getHisto2D("ap_xz_zoom")->GetXaxis()->SetTitle("X position [cm]");
  getStatistics().getHisto2D("ap_xz_zoom")->GetYaxis()->SetTitle("Z position [cm]");

  getStatistics().createHistogram(new TH2F("ap_yz_zoom", "YZ position of annihilation point", 101, -10.5, 10.5, 101, -10.5, 10.5));
  getStatistics().getHisto2D("ap_yz_zoom")->GetXaxis()->SetTitle("Y position [cm]");
  getStatistics().getHisto2D("ap_yz_zoom")->GetYaxis()->SetTitle("Z position [cm]");

  // Histograms for scintillator synchronization constatns
  if (fSaveCalibHistos)
  {
    // 2D TOF per Scin
    getStatistics().createHistogram(new TH2F("ap_tof_scin", "TOF of annihilation event per Scin", maxScinID - minScinID + 1, minScinID - 0.5,
                                             maxScinID + 0.5, 200, -15000.0, 15000.0));
    getStatistics().getHisto2D("ap_tof_scin")->GetXaxis()->SetTitle("Scintillator ID");
    getStatistics().getHisto2D("ap_tof_scin")->GetYaxis()->SetTitle("Time of Flight [ps]");

    getStatistics().createHistogram(new TH2F("ap_tof_conv_scin", "TOF by convention of annihilation event per Scin", maxScinID - minScinID + 1,
                                             minScinID - 0.5, maxScinID + 0.5, 200, -15000.0, 15000.0));
    getStatistics().getHisto2D("ap_tof_conv_scin")->GetXaxis()->SetTitle("Scintillator ID");
    getStatistics().getHisto2D("ap_tof_conv_scin")->GetYaxis()->SetTitle("Time of Flight [ps]");

    getStatistics().createHistogram(new TH2F("ap_tof_corr_scin", "Corrected TOF of annihilation event per Scin", maxScinID - minScinID + 1,
                                             minScinID - 0.5, maxScinID + 0.5, 200, -15000.0, 15000.0));
    getStatistics().getHisto2D("ap_tof_corr_scin")->GetXaxis()->SetTitle("Scintillator ID");
    getStatistics().getHisto2D("ap_tof_corr_scin")->GetYaxis()->SetTitle("Time of Flight [ps]");

    getStatistics().createHistogram(new TH2F("ap_tof_conv_corr_scin", "Corrected TOF by convention of annihilation event per Scin",
                                             maxScinID - minScinID + 1, minScinID - 0.5, maxScinID + 0.5, 200, -15000.0, 15000.0));
    getStatistics().getHisto2D("ap_tof_conv_corr_scin")->GetXaxis()->SetTitle("Scintillator ID");
    getStatistics().getHisto2D("ap_tof_conv_corr_scin")->GetYaxis()->SetTitle("Time of Flight [ps]");

    getStatistics().createHistogram(new TH2F("ap_tof_conv_corr_z", "Corrected TOF by convention of annihilation event vs. z-position of hits", 101,
                                             -25.5, 25.5, 200, -5000.0, 5000.0));
    getStatistics().getHisto2D("ap_tof_conv_corr_z")->GetXaxis()->SetTitle("z-position of hit [cm]");
    getStatistics().getHisto2D("ap_tof_conv_corr_z")->GetYaxis()->SetTitle("Time of Flight [ps]");

    // Synchronization of TOF with annihilaion-deexcitation pairs
    getStatistics().createHistogram(new TH2F("tdiff_annih_scin", "A-D time difference for annihilation hit per scin", maxScinID - minScinID + 1,
                                             minScinID - 0.5, maxScinID + 0.5, 200, -20000.0, 20000.0));
    getStatistics().getHisto2D("tdiff_annih_scin")->GetXaxis()->SetTitle("Time diffrence [ps]");
    getStatistics().getHisto2D("tdiff_annih_scin")->GetYaxis()->SetTitle("Number of A-D pairs");

    getStatistics().createHistogram(new TH2F("tdiff_deex_scin", "A-D time difference for deex hit per scin", maxScinID - minScinID + 1,
                                             minScinID - 0.5, maxScinID + 0.5, 200, -20000.0, 20000.0));
    getStatistics().getHisto2D("tdiff_deex_scin")->GetXaxis()->SetTitle("Time diffrence [ps]");
    getStatistics().getHisto2D("tdiff_deex_scin")->GetYaxis()->SetTitle("Number of A-D pairs");

    // for (int scinID = minScinID; scinID <= maxScinID; scinID++)
    // {
    //   getStatistics().createHistogram(new TH2F(Form("time_walk_scin_%d", scinID), Form("TOF vs. reversed TOT, time walk effect Scin %d", scinID),
    //   200,
    //                                            -fMaxTimeDiff, fMaxTimeDiff, 200, -0.0005, 0.0005));
    //   getStatistics().getHisto2D(Form("time_walk_scin_%d", scinID))->GetXaxis()->SetTitle("ToF [ps]");
    //   getStatistics().getHisto2D(Form("time_walk_scin_%d", scinID))->GetYaxis()->SetTitle("Reversed TOT [1/ps]");
    // }
  }

  // Time difference between SiPM thresholds
  // getStatistics().createHistogram(new TH2F("ap_sig_thr_tdiff", "Time difference between SiPM thresholds", maxSiPMID - minSiPMID + 1, minSiPMID -
  // 0.5,
  //                                          maxSiPMID + 0.5, 200, -10000.0, 10000.0));
  // getStatistics().getHisto2D("ap_sig_thr_tdiff")->GetXaxis()->SetTitle("SiPM ID");
  // getStatistics().getHisto2D("ap_sig_thr_tdiff")->GetYaxis()->SetTitle("Time diffrence [ps]");

  // Histograms for 3Gamama category
  // getStatistics().createHistogram(
  //   new TH2F("3Gamma_Angles", "Relative angles - transformed", 251, -0.5, 250.5, 201, -0.5, 200.5));
  // getStatistics().getHisto2D("3Gamma_Angles")->GetXaxis()->SetTitle("Relative angle 1-2");
  // getStatistics().getHisto2D("3Gamma_Angles")->GetYaxis()->SetTitle("Relative angle 2-3");

  // Histograms for scattering category
  // getStatistics().createHistogram(
  //   new TH1F("ScatterTOF_TimeDiff", "Difference of Scatter TOF and hits time difference",
  //     200, 0.0, 3.0*fScatterTOFTimeDiff));
  // getStatistics().getHisto1D("ScatterTOF_TimeDiff")->GetXaxis()->SetTitle("Scat_TOF & time diff [ps]");
  // getStatistics().getHisto1D("ScatterTOF_TimeDiff")->GetYaxis()->SetTitle("Number of Hit Pairs");

  // getStatistics().createHistogram(
  //    new TH2F("ScatterAngle_PrimaryTOT", "Angle of scattering vs. TOT of primary hits",
  //     181, -0.5, 180.5, 200, 0.0, 40000.0));
  // getStatistics().getHisto2D("ScatterAngle_PrimaryTOT")->GetXaxis()->SetTitle("Scattering Angle");
  // getStatistics().getHisto2D("ScatterAngle_PrimaryTOT")->GetYaxis()->SetTitle("TOT of primary hit [ps]");

  // getStatistics().createHistogram(
  //    new TH2F("ScatterAngle_ScatterTOT", "Angle of scattering vs. TOT of scattered hits",
  //     181, -0.5, 180.5, 200, 0.0, 40000.0));
  // getStatistics().getHisto2D("ScatterAngle_ScatterTOT")->GetXaxis()->SetTitle("Scattering Angle");
  // getStatistics().getHisto2D("ScatterAngle_ScatterTOT")->GetYaxis()->SetTitle("TOT of scattered hit [ps]");

  // Histograms for deexcitation
  // getStatistics().createHistogram(
  //   new TH1F("Deex_TOT_cut", "TOT of all hits with deex cut (30,50) ns",
  //     200, 25000.0, 55000.0));
  // getStatistics().getHisto1D("Deex_TOT_cut")->GetXaxis()->SetTitle("TOT [ps]");
  // getStatistics().getHisto1D("Deex_TOT_cut")->GetYaxis()->SetTitle("Number of Hits");
}
