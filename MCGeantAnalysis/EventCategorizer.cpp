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
 *  @file EventCategorizer.cpp
 */

#include <JPetOptionsTools/JPetOptionsTools.h>
#include <JPetWriter/JPetWriter.h>
#include "EventCategorizerTools.h"
#include "EventCategorizer.h"
#include <iostream>

using namespace jpet_options_tools;
using namespace std;

EventCategorizer::EventCategorizer(const char* name): JPetUserTask(name) {}

bool EventCategorizer::init()
{
  INFO("Event categorization started.");
  // Parameter for back to back categorization
  if (isOptionSet(fParams.getOptions(), kBack2BackSlotThetaDiffParamKey))
    fB2BSlotThetaDiff = getOptionAsFloat(fParams.getOptions(), kBack2BackSlotThetaDiffParamKey);
  else
    WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.",
      kBack2BackSlotThetaDiffParamKey.c_str(), fB2BSlotThetaDiff));
  // Parameter for scattering determination
  if (isOptionSet(fParams.getOptions(), kScatterTOFTimeDiffParamKey))
    fScatterTOFTimeDiff = getOptionAsFloat(fParams.getOptions(), kScatterTOFTimeDiffParamKey);
  else
    WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.",
      kScatterTOFTimeDiffParamKey.c_str(), fScatterTOFTimeDiff));
  // Parameters for deexcitation TOT cut
  if (isOptionSet(fParams.getOptions(), kDeexTOTCutMinParamKey))
    fDeexTOTCutMin = getOptionAsFloat(fParams.getOptions(), kDeexTOTCutMinParamKey);
  else
    WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.",
      kDeexTOTCutMinParamKey.c_str(), fDeexTOTCutMin));
  if (isOptionSet(fParams.getOptions(), kDeexTOTCutMaxParamKey))
    fDeexTOTCutMax = getOptionAsFloat(fParams.getOptions(), kDeexTOTCutMaxParamKey);
  else
    WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.",
      kDeexTOTCutMaxParamKey.c_str(), fDeexTOTCutMax));
  // Getting bool for saving histograms
  if (isOptionSet(fParams.getOptions(), kSaveControlHistosParamKey))
    fSaveControlHistos = getOptionAsBool(fParams.getOptions(), kSaveControlHistosParamKey);

  // Input events type
  fOutputEvents = new JPetTimeWindow("JPetEvent");
  // Initialise hisotgrams
  if(fSaveControlHistos) initialiseHistograms();
  return true;
}

bool EventCategorizer::exec()
{
  if (auto timeWindow = dynamic_cast<const JPetTimeWindow* const>(fEvent)) {
    vector<JPetEvent> events;
    for (uint i = 0; i < timeWindow->getNumberOfEvents(); i++) {
      const auto& event = dynamic_cast<const JPetEvent&>(timeWindow->operator[](i));

      // Check types of current event
      bool is2Gamma = EventCategorizerTools::checkFor2Gamma(event, getStatistics(),
        fSaveControlHistos, fB2BSlotThetaDiff);
      bool is3Gamma = EventCategorizerTools::checkFor3Gamma(event, getStatistics(), fSaveControlHistos);
      bool isPrompt = EventCategorizerTools::checkForPrompt(event, getStatistics(),
        fSaveControlHistos, fDeexTOTCutMin, fDeexTOTCutMax);
      bool isScattered = EventCategorizerTools::checkForScatter(event, getStatistics(),
        fSaveControlHistos, fScatterTOFTimeDiff);

      JPetEvent newEvent = event;
      if(is2Gamma) newEvent.addEventType(JPetEventType::k2Gamma);
      if(is3Gamma) newEvent.addEventType(JPetEventType::k3Gamma);
      if(isPrompt) newEvent.addEventType(JPetEventType::kPrompt);
      if(isScattered) newEvent.addEventType(JPetEventType::kScattered);

      if(fSaveControlHistos){
        for(auto hit : event.getHits())
          getStatistics().getHisto2D("All_XYpos")->Fill(hit.getPosX(), hit.getPosY());
      }
      events.push_back(newEvent);
    }
    saveEvents(events);
  } else return false;
  return true;
}

bool EventCategorizer::terminate()
{
  INFO("Event categorization completed.");
  return true;
}

void EventCategorizer::saveEvents(const vector<JPetEvent>& events)
{
  for (const auto& event : events) fOutputEvents->add<JPetEvent>(event);
}

void EventCategorizer::initialiseHistograms(){

  // General histograms
  getStatistics().createHistogram(
    new TH2F("All_XYpos", "Hit position XY", 242, -60.5, 60.5, 121, -60.5, 60.5));
  getStatistics().getHisto2D("All_XYpos")->GetXaxis()->SetTitle("Hit X position [cm]");
  getStatistics().getHisto2D("All_XYpos")->GetYaxis()->SetTitle("Hit Y position [cm]");

  // Histograms for 2Gamama category
  getStatistics().createHistogram(
    new TH1F("2Gamma_Zpos", "B2B hits Z position", 200, -50.0, 50.0));
  getStatistics().getHisto1D("2Gamma_Zpos")->GetXaxis()->SetTitle("Z axis position [cm]");
  getStatistics().getHisto1D("2Gamma_Zpos")->GetYaxis()->SetTitle("Number of Hits");

  getStatistics().createHistogram(
    new TH1F("2Gamma_TimeDiff", "B2B hits time difference", 100, -10000.0, 10000.0));
  getStatistics().getHisto1D("2Gamma_TimeDiff")->GetXaxis()->SetTitle("Time Difference [ps]");
  getStatistics().getHisto1D("2Gamma_TimeDiff")->GetYaxis()->SetTitle("Number of Hit Pairs");

  getStatistics().createHistogram(
    new TH1F("2Gamma_Dist", "B2B hits distance", 200, -100.0, 100.0));
  getStatistics().getHisto1D("2Gamma_Dist")->GetXaxis()->SetTitle("Distance [cm]");
  getStatistics().getHisto1D("2Gamma_Dist")->GetYaxis()->SetTitle("Number of Hit Pairs");

  getStatistics().createHistogram(
    new TH1F("Annih_TOF", "Annihilation pairs Time of Flight", 200, -3000.0,3000.0));
  getStatistics().getHisto1D("Annih_TOF")->GetXaxis()->SetTitle("Time of Flight [ps]");
  getStatistics().getHisto1D("Annih_TOF")->GetYaxis()->SetTitle("Number of Annihilation Pairs");

  getStatistics().createHistogram(
     new TH2F("AnnihPoint_XY", "XY position of annihilation point", 121, -60.5, 60.5, 121, -60.5, 60.5));
  getStatistics().getHisto2D("AnnihPoint_XY")->GetXaxis()->SetTitle("X position [cm]");
  getStatistics().getHisto2D("AnnihPoint_XY")->GetYaxis()->SetTitle("Y position [cm]");

  getStatistics().createHistogram(
    new TH2F("AnnihPoint_XZ", "XZ position of annihilation point", 121, -60.5, 60.5, 121, -60.5, 60.5));
  getStatistics().getHisto2D("AnnihPoint_XZ")->GetXaxis()->SetTitle("X position [cm]");
  getStatistics().getHisto2D("AnnihPoint_XZ")->GetYaxis()->SetTitle("Z position [cm]");

  getStatistics().createHistogram(
    new TH2F("AnnihPoint_YZ", "YZ position of annihilation point", 121, -60.5, 60.5, 121, -60.5, 60.5));
  getStatistics().getHisto2D("AnnihPoint_YZ")->GetXaxis()->SetTitle("Y position [cm]");
  getStatistics().getHisto2D("AnnihPoint_YZ")->GetYaxis()->SetTitle("Z position [cm]");

  // Histograms for 3Gamama category
  getStatistics().createHistogram(
    new TH2F("3Gamma_Angles", "Relative angles - transformed", 251, -0.5, 250.5, 201, -0.5, 200.5));
  getStatistics().getHisto2D("3Gamma_Angles")->GetXaxis()->SetTitle("Relative angle 1-2");
  getStatistics().getHisto2D("3Gamma_Angles")->GetYaxis()->SetTitle("Relative angle 2-3");

  // Histograms for scattering category
  getStatistics().createHistogram(
    new TH1F("ScatterTOF_TimeDiff", "Difference of Scatter TOF and hits time difference",
      200, 0.0, 3.0*fScatterTOFTimeDiff));
  getStatistics().getHisto1D("ScatterTOF_TimeDiff")->GetXaxis()->SetTitle("Scat_TOF & time diff [ps]");
  getStatistics().getHisto1D("ScatterTOF_TimeDiff")->GetYaxis()->SetTitle("Number of Hit Pairs");

  getStatistics().createHistogram(
     new TH2F("ScatterAngle_PrimaryTOT", "Angle of scattering vs. TOT of primary hits",
      181, -0.5, 180.5, 200, 0.0, 40000.0));
  getStatistics().getHisto2D("ScatterAngle_PrimaryTOT")->GetXaxis()->SetTitle("Scattering Angle");
  getStatistics().getHisto2D("ScatterAngle_PrimaryTOT")->GetYaxis()->SetTitle("TOT of primary hit [ps]");

  getStatistics().createHistogram(
     new TH2F("ScatterAngle_ScatterTOT", "Angle of scattering vs. TOT of scattered hits",
      181, -0.5, 180.5, 200, 0.0, 40000.0));
  getStatistics().getHisto2D("ScatterAngle_ScatterTOT")->GetXaxis()->SetTitle("Scattering Angle");
  getStatistics().getHisto2D("ScatterAngle_ScatterTOT")->GetYaxis()->SetTitle("TOT of scattered hit [ps]");

  // Histograms for deexcitation
  getStatistics().createHistogram(
    new TH1F("Deex_TOT_cut", "TOT of all hits with deex cut (30,50) ns",
      200, 25000.0, 55000.0));
  getStatistics().getHisto1D("Deex_TOT_cut")->GetXaxis()->SetTitle("TOT [ps]");
  getStatistics().getHisto1D("Deex_TOT_cut")->GetYaxis()->SetTitle("Number of Hits");
}
