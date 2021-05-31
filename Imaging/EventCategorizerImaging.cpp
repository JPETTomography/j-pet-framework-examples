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
 *  @file EventCategorizerImaging.cpp
 */

#include "EventCategorizerImaging.h"
#include "EventCategorizerTools.h"

#include <JPetOptionsTools/JPetOptionsTools.h>
#include <JPetWriter/JPetWriter.h>
#include <iostream>

using namespace jpet_options_tools;

using namespace std;

EventCategorizerImaging::EventCategorizerImaging(const char* name) : JPetUserTask(name) {}

bool EventCategorizerImaging::init()
{
  INFO("Imaging streaming started.");

  fOutputEvents = new JPetTimeWindow("JPetEvent");

  if (isOptionSet(fParams.getOptions(), kMinAnnihilationParamKey)) {
    fMinAnnihilationTOT = getOptionAsFloat(fParams.getOptions(), kMinAnnihilationParamKey);
  } else {
    WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.", kMinAnnihilationParamKey.c_str(),
                 fMinAnnihilationTOT));
  }
  if (isOptionSet(fParams.getOptions(), kMaxAnnihilationParamKey)) {
    fMaxAnnihilationTOT = getOptionAsFloat(fParams.getOptions(), kMaxAnnihilationParamKey);
  } else {
    WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.", kMaxAnnihilationParamKey.c_str(),
                 fMaxAnnihilationTOT));
  }
  if (isOptionSet(fParams.getOptions(), kMinPromptTOTParamKey)) {
    fMinPromptTOT = getOptionAsFloat(fParams.getOptions(), kMinPromptTOTParamKey);
  } else {
    WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.", kMinPromptTOTParamKey.c_str(),
                 fMinPromptTOT));
  }
  if (isOptionSet(fParams.getOptions(), kMaxPromptTOTParamKey)) {
    fMaxPromptTOT = getOptionAsFloat(fParams.getOptions(), kMaxPromptTOTParamKey);
  } else {
    WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.", kMaxPromptTOTParamKey.c_str(),
                 fMaxPromptTOT));
  }
  if (isOptionSet(fParams.getOptions(), kMaxZPosParamKey)) {
    fMaxZPos = getOptionAsFloat(fParams.getOptions(), kMaxZPosParamKey);
  } else {
    WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.", kMaxZPosParamKey.c_str(), fMaxZPos));
  }
  if (isOptionSet(fParams.getOptions(), kMaxDistOfDecayPlaneFromCenterParamKey)) {
    fMaxDistOfDecayPlaneFromCenter = getOptionAsFloat(fParams.getOptions(), kMaxDistOfDecayPlaneFromCenterParamKey);
  } else {
    WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.", kMaxDistOfDecayPlaneFromCenterParamKey.c_str(),
                 fMaxDistOfDecayPlaneFromCenter));
  }
  if (isOptionSet(fParams.getOptions(), kMaxTimeDiffParamKey)) {
    fMaxTimeDiff = getOptionAsFloat(fParams.getOptions(), kMaxTimeDiffParamKey);
  } else {
    WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.", kMaxTimeDiffParamKey.c_str(), fMaxTimeDiff));
  }
  if (isOptionSet(fParams.getOptions(), kBackToBackAngleWindowParamKey)) {
    fBackToBackAngleWindow = getOptionAsFloat(fParams.getOptions(), kBackToBackAngleWindowParamKey);
  } else {
    WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.", kBackToBackAngleWindowParamKey.c_str(),
                 fBackToBackAngleWindow));
  }
  if (isOptionSet(fParams.getOptions(), kDecayInto3MinAngleParamKey)) {
    fDecayInto3MinAngle = getOptionAsFloat(fParams.getOptions(), kDecayInto3MinAngleParamKey);
  } else {
    WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.", kDecayInto3MinAngleParamKey.c_str(),
                 fDecayInto3MinAngle));
  }
  if (isOptionSet(fParams.getOptions(), kMaxScattTestValueParamKey)) {
    fScattTestValue = getOptionAsFloat(fParams.getOptions(), kMaxScattTestValueParamKey);
  } else {
    WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.", kMaxScattTestValueParamKey.c_str(),
                 fScattTestValue));
  }
  if (isOptionSet(fParams.getOptions(), kMinAngleWindowValueParamKey)) {
    fMinAngleWindow = getOptionAsFloat(fParams.getOptions(), kMinAngleWindowValueParamKey);
  } else {
    WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.", kMinAngleWindowValueParamKey.c_str(),
                 fMinAngleWindow));
  }
  if (fSaveControlHistos)
  {
    //Dynamic binning is not neccessary, as it is enough to plot histogram before and after cut in the same ranges to compare.
      
    getStatistics().createHistogram(new TH1F("2Gamma_TimeDiff", "2 Gamma Hits Time Difference", 200, -0.025, 9.975));
    getStatistics().getHisto1D("2Gamma_TimeDiff")->SetXTitle("Hits time difference [ns]");
    getStatistics().getHisto1D("2Gamma_TimeDiff")->SetYTitle("Counts");

    //1 deg bin can generate artifacts on the angle -> changing to 0.1 deg bin
    getStatistics().createHistogram(new TH1F("2Gamma_ThetaDiff", "2 Gamma Hits angles", 2000, -0.05, 199.95));
    getStatistics().getHisto1D("2Gamma_ThetaDiff")->SetXTitle("Hits theta diff [deg]");
    getStatistics().getHisto1D("2Gamma_ThetaDiff")->SetYTitle("Counts");

    getStatistics().createHistogram(new TH1F("2Gamma_DLOR", "Delta LOR distance", 100, -0.25, 49.75));
    getStatistics().getHisto1D("2Gamma_DLOR")->SetXTitle("Delta LOR [cm]");
    getStatistics().getHisto1D("2Gamma_DLOR")->SetYTitle("Counts");
    
    getStatistics().createHistogram(new TH1F("2Gamma_ScattTest", "Scatter test for annihilation hits", 500, -5.05, 44.95));
    getStatistics().getHisto1D("2Gamma_ScattTest")->SetXTitle("Scatter test [ns]");
    getStatistics().getHisto1D("2Gamma_ScattTest")->SetYTitle("Counts");

    getStatistics().createHistogram(new TH1F("2Anni_TimeDiff", "2 gamma annihilation Hits Time Difference", 200, -0.025, 9.975));
    getStatistics().getHisto1D("2Anni_TimeDiff")->SetXTitle("Time difference between 2 annihilation hits [ns]");
    getStatistics().getHisto1D("2Anni_TimeDiff")->SetYTitle("Counts");

    //1 deg bin can generate artifacts on the angle -> changing to 0.1 deg bin
    getStatistics().createHistogram(new TH1F("2Anni_ThetaDiff", "Annihilation Hits Theta Diff", 2000, -0.05, 199.95));
    getStatistics().getHisto1D("2Anni_ThetaDiff")->SetXTitle("Annihilation hits theta diff [deg]");
    getStatistics().getHisto1D("2Anni_ThetaDiff")->SetYTitle("Counts");

    getStatistics().createHistogram(new TH1F("2Anni_DLOR", "Delta LOR distance", 100, -0.25, 49.75));
    getStatistics().getHisto1D("2Anni_DLOR")->SetXTitle("Annihilation hits Delta LOR [cm]");
    getStatistics().getHisto1D("2Anni_DLOR")->SetYTitle("Counts");
    
    getStatistics().createHistogram(new TH1F("2Anni_ScattTest", "Scatter test for annihilation hits classified as 2G", 500, -5.05, 44.95));
    getStatistics().getHisto1D("2Anni_ScattTest")->SetXTitle("Scatter test [ns]");
    getStatistics().getHisto1D("2Anni_ScattTest")->SetYTitle("Counts");

    getStatistics().createHistogram(new TH2F("2Anni_XY", "Reconstructed XY position of annihilation point", 220, -55.25, 54.75, 220, -55.25, 54.75));
    getStatistics().getHisto2D("2Anni_XY")->SetXTitle("Annihilation point X [cm]");
    getStatistics().getHisto2D("2Anni_XY")->SetYTitle("Annihilation point Y [cm]");

    getStatistics().createHistogram(new TH1F("2Anni_Z", "Reconstructed Z position of annihilation point", 220, -55.25, 54.75));
    getStatistics().getHisto1D("2Anni_Z")->SetXTitle("Annihilation point Z [cm]");
    getStatistics().getHisto1D("2Anni_Z")->SetYTitle("Counts");
    
//---------------------------------------------------------------------------------------------------------------------
    
    getStatistics().createHistogram(new TH1F("2Gamma1Prompt_TimeDiff", "2 Gamma Hits Time Difference", 200, -0.025, 9.975));
    getStatistics().getHisto1D("2Gamma1Prompt_TimeDiff")->SetXTitle("Hits time difference [ns]");
    getStatistics().getHisto1D("2Gamma1Prompt_TimeDiff")->SetYTitle("Counts");

    //1 deg bin can generate artifacts on the angle -> changing to 0.1 deg bin
    getStatistics().createHistogram(new TH1F("2Gamma1Prompt_ThetaDiff", "2 Gamma Hits angles", 2000, -0.05, 199.95));
    getStatistics().getHisto1D("2Gamma1Prompt_ThetaDiff")->SetXTitle("Hits theta diff [deg]");
    getStatistics().getHisto1D("2Gamma1Prompt_ThetaDiff")->SetYTitle("Counts");
    
    getStatistics().createHistogram(new TH1F("2Gamma1Prompt_AnniPromptThetaDiff", "2 Gamma Hits angles", 2000, -0.05, 199.95));
    getStatistics().getHisto1D("2Gamma1Prompt_AnniPromptThetaDiff")->SetXTitle("Hits theta diff [deg]");
    getStatistics().getHisto1D("2Gamma1Prompt_AnniPromptThetaDiff")->SetYTitle("Counts");

    getStatistics().createHistogram(new TH1F("2Gamma1Prompt_DLOR", "Delta LOR distance", 100, -0.25, 49.75));
    getStatistics().getHisto1D("2Gamma1Prompt_DLOR")->SetXTitle("Delta LOR [cm]");
    getStatistics().getHisto1D("2Gamma1Prompt_DLOR")->SetYTitle("Counts");
    
    getStatistics().createHistogram(new TH1F("2Gamma1Prompt_ScattTest", "Scatter test for annihilation hits", 500, -5.05, 44.95));
    getStatistics().getHisto1D("2Gamma1Prompt_ScattTest")->SetXTitle("Scatter test [ns]");
    getStatistics().getHisto1D("2Gamma1Prompt_ScattTest")->SetYTitle("Counts");

    getStatistics().createHistogram(new TH1F("2Anni1Prompt_TimeDiff", "2 gamma annihilation Hits Time Difference", 200, -0.025, 9.975));
    getStatistics().getHisto1D("2Anni1Prompt_TimeDiff")->SetXTitle("Time difference between 2 annihilation hits [ns]");
    getStatistics().getHisto1D("2Anni1Prompt_TimeDiff")->SetYTitle("Counts");

    //1 deg bin can generate artifacts on the angle -> changing to 0.1 deg bin
    getStatistics().createHistogram(new TH1F("2Anni1Prompt_ThetaDiff", "Annihilation Hits Theta Diff", 2000, -0.05, 199.95));
    getStatistics().getHisto1D("2Anni1Prompt_ThetaDiff")->SetXTitle("Annihilation hits theta diff [deg]");
    getStatistics().getHisto1D("2Anni1Prompt_ThetaDiff")->SetYTitle("Counts");
    
    getStatistics().createHistogram(new TH1F("2Anni1Prompt_AnniPromptThetaDiff", "Annihilation Hits Theta Diff", 2000, -0.05, 199.95));
    getStatistics().getHisto1D("2Anni1Prompt_AnniPromptThetaDiff")->SetXTitle("Annihilation hits theta diff [deg]");
    getStatistics().getHisto1D("2Anni1Prompt_AnniPromptThetaDiff")->SetYTitle("Counts");

    getStatistics().createHistogram(new TH1F("2Anni1Prompt_DLOR", "Delta LOR distance", 100, -0.25, 49.75));
    getStatistics().getHisto1D("2Anni1Prompt_DLOR")->SetXTitle("Annihilation hits Delta LOR [cm]");
    getStatistics().getHisto1D("2Anni1Prompt_DLOR")->SetYTitle("Counts");
    
    getStatistics().createHistogram(new TH1F("2Anni_ScattTest", "Scatter test for annihilation hits classified as 2G", 500, -5.05, 44.95));
    getStatistics().getHisto1D("2Anni1Prompt_ScattTest")->SetXTitle("Scatter test [ns]");
    getStatistics().getHisto1D("2Anni1Prompt_ScattTest")->SetYTitle("Counts");

    getStatistics().createHistogram(new TH2F("2Anni1Prompt_XY", "Reconstructed XY position of annihilation point", 220, -55.25, 54.75, 220, -55.25, 54.75));
    getStatistics().getHisto2D("2Anni1Prompt_XY")->SetXTitle("Annihilation point X [cm]");
    getStatistics().getHisto2D("2Anni1Prompt_XY")->SetYTitle("Annihilation point Y [cm]");

    getStatistics().createHistogram(new TH1F("2Anni1Prompt_Z", "Reconstructed Z position of annihilation point", 220, -55.25, 54.75));
    getStatistics().getHisto1D("2Anni1Prompt_Z")->SetXTitle("Annihilation point Z [cm]");
    getStatistics().getHisto1D("2Anni1Prompt_Z")->SetYTitle("Counts");
    
    getStatistics().createHistogram(new TH1F("2Anni1Prompt_Lifetime", "Positron lifetime distribution", 4000, -200.05, 199.95));
    getStatistics().getHisto1D("2Anni1Prompt_Lifetime")->SetXTitle("Positron lifetime [ns]");
    getStatistics().getHisto1D("2Anni1Prompt_Lifetime")->SetYTitle("Counts");
    
//---------------------------------------------------------------------------------------------------------------------
    
    getStatistics().createHistogram(new TH2F("3Gamma_Thetas", "3 Gamma Thetas plot", 2500, -0.05, 249.95, 2000, -0.05, 199.95));
    getStatistics().getHisto2D("3Gamma_Thetas")->SetXTitle("Transformed thetas 1-2 [deg]");
    getStatistics().getHisto2D("3Gamma_Thetas")->SetYTitle("Transformed thetas 2-3 [deg]");

    getStatistics().createHistogram(new TH1F("3Gamma_PlaneDist", "3 Gamma Plane Distance to Center", 100, -0.25, 49.75));
    getStatistics().getHisto1D("3Gamma_PlaneDist")->SetXTitle("Distance [cm]");
    getStatistics().getHisto1D("3Gamma_PlaneDist")->SetYTitle("Counts");

    getStatistics().createHistogram(new TH1F("3Gamma_TimeDiff", "3 gamma last and first hit time difference", 200, -0.025, 9.975));
    getStatistics().getHisto1D("3Gamma_TimeDiff")->SetXTitle("Time difference [ns]");
    getStatistics().getHisto1D("3Gamma_TimeDiff")->SetYTitle("Counts");

    getStatistics().createHistogram(new TH1F("3Gamma_ScattTest", "Scatter test for every pair of the annihilation hits", 500, -5.05, 44.95));
    getStatistics().getHisto1D("3Gamma_ScattTest")->SetXTitle("Scatter test [ns]");
    getStatistics().getHisto1D("3Gamma_ScattTest")->SetYTitle("Counts");
    
    getStatistics().createHistogram(new TH2F("3Anni_Thetas", "3 Gamma Thetas plot after cut", 2500, -0.05, 249.95, 2000, -0.05, 199.95));
    getStatistics().getHisto2D("3Anni_Thetas")->SetXTitle("Transformed thetas 1-2 [deg]");
    getStatistics().getHisto2D("3Anni_Thetas")->SetYTitle("Transformed thetas 2-3 [deg]");
    
    getStatistics().createHistogram(
        new TH1F("3Anni_PlaneDist", "3 Gamma Annihilation Plane Distance to Center", 1100, -0.25, 49.75));
    getStatistics().getHisto1D("3Anni_PlaneDist")->SetXTitle("Distance [cm]");
    getStatistics().getHisto1D("3Anni_PlaneDist")->SetYTitle("Counts");

    getStatistics().createHistogram(
        new TH1F("3Anni_TimeDiff", "3 gamma Annihilation last and first hit time difference", 200, -0.025, 9.975));
    getStatistics().getHisto1D("3Anni_TimeDiff")->SetXTitle("Time difference [ns]");
    getStatistics().getHisto1D("3Anni_TimeDiff")->SetYTitle("Counts");
    
    getStatistics().createHistogram(new TH1F("3Anni_ScattTest", "Scatter test for every pair of the annihilation hits", 500, -5.05, 44.95));
    getStatistics().getHisto1D("3Anni_ScattTest")->SetXTitle("Scatter test [ns]");
    getStatistics().getHisto1D("3Anni_ScattTest")->SetYTitle("Counts");
  }
  return true;
}

bool EventCategorizerImaging::exec()
{
  vector<JPetEvent> events;
  if (auto timeWindow = dynamic_cast<const JPetTimeWindow* const>(fEvent))
  {
    uint n = timeWindow->getNumberOfEvents();
    for (uint i = 0; i < n; ++i)
    {
      const auto& event = dynamic_cast<const JPetEvent&>(timeWindow->operator[](i));
      if (event.getHits().size() > 1)
      {
        vector<JPetHit> hits = event.getHits();
        JPetEvent imagingEvent = imageReconstruction(hits);
        if (imagingEvent.getHits().size())
        {
          events.push_back(imagingEvent);
        }
      }
    }
  }
  else
  {
    return false;
  }
  if (events.size())
  {
    saveEvents(events);
  }
  events.clear();
  return true;
}

bool EventCategorizerImaging::terminate()
{
  INFO("Imaging streaming ended.");
  return true;
}

void EventCategorizerImaging::saveEvents(const vector<JPetEvent>& events)
{
  for (const auto& event : events)
  {
    fOutputEvents->add<JPetEvent>(event);
  }
}

JPetEvent EventCategorizerImaging::imageReconstruction(vector<JPetHit> hits)
{
  JPetEvent imagingEvent;
  uint promptIndex = -1;
  for (unsigned i = 0; i < hits.size(); i++)
  {
    double TOTofHit = EventCategorizerTools::calculateTOT(hits[i]);
    if (TOTofHit >= fMinAnnihilationTOT && TOTofHit <= fMaxAnnihilationTOT && fabs(hits[i].getPosZ()) < fMaxZPos)
    {
      imagingEvent.addHit(hits[i]);
    }
    if (TOTofHit >= fMinPromptTOT && TOTofHit <= fMaxPromptTOT && fabs(hits[i].getPosZ()) < fMaxZPos)
    {
      imagingEvent.addHit(hits[i]);
      if (promptIndex == -1)
        promptIndex = i;
      else
        promptIndex = -2;
    }
  }
  if (EventCategorizerTools::stream2Gamma(imagingEvent, getStatistics(), fSaveControlHistos, fBackToBackAngleWindow, fMaxTimeDiff, fScattTestValue))
  {
    imagingEvent.addEventType(JPetEventType::k2Gamma);
  }
  if (EventCategorizerTools::stream3Gamma(imagingEvent, getStatistics(), fSaveControlHistos, fDecayInto3MinAngle, fMaxTimeDiff,
                                          fMaxDistOfDecayPlaneFromCenter))
  {
    imagingEvent.addEventType(JPetEventType::k3Gamma);
  }
  if (promptIndex >= 0) {
    if (EventCategorizerTools::stream2GammaPlus1Prompt(imagingEvent, getStatistics(), fSaveControlHistos, fBackToBackAngleWindow, fMinAngleWindow, 
        fMaxTimeDiff, fScattTestValue, promptIndex))
    {
      imagingEvent.addEventType(JPetEventType::k2Gamma);
      imagingEvent.addEventType(JPetEventType::Prompt);
    }
  }
  return imagingEvent;
}
