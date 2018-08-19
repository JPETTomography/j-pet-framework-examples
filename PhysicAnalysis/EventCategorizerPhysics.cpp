/**
 *  @copyright Copyright 2018 The J-PET Framework Authors. All rights reserved.
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
 *  @file EventCategorizerPhysics.cpp
 */

#include "../LargeBarrelAnalysis/EventCategorizerTools.h"
#include <JPetOptionsTools/JPetOptionsTools.h>
#include "EventCategorizerPhysics.h"
#include <JPetWriter/JPetWriter.h>
#include <iostream>

using namespace jpet_options_tools;

using namespace std;

EventCategorizerPhysics::EventCategorizerPhysics(const char* name): JPetUserTask(name) {}

bool EventCategorizerPhysics::init()
{
  INFO("Physics streaming started.");

  fOutputEvents = new JPetTimeWindow("JPetEvent");

  if (isOptionSet(fParams.getOptions(), kMinAnnihilationParamKey)) {
    fMinAnnihilationTOT = getOptionAsFloat(fParams.getOptions(), kMinAnnihilationParamKey);
  } else {
    WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.", kMinAnnihilationParamKey.c_str(), fMinAnnihilationTOT));
  }
  if (isOptionSet(fParams.getOptions(), kMaxAnnihilationParamKey)) {
    fMaxAnnihilationTOT = getOptionAsFloat(fParams.getOptions(), kMaxAnnihilationParamKey);
  } else {
    WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.", kMaxAnnihilationParamKey.c_str(), fMaxAnnihilationTOT));
  }
  if (isOptionSet(fParams.getOptions(), kMinDeexcitationParamKey)) {
    fMinDeexcitationTOT = getOptionAsFloat(fParams.getOptions(), kMinDeexcitationParamKey);
  } else {
    WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.", kMinDeexcitationParamKey.c_str(), fMinDeexcitationTOT));
  }
  if (isOptionSet(fParams.getOptions(), kMaxDeexcitationParamKey)) {
    fMaxDeexcitationTOT = getOptionAsFloat(fParams.getOptions(), kMaxDeexcitationParamKey);
  } else {
    WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.", kMaxDeexcitationParamKey.c_str(), fMaxDeexcitationTOT));
  }
  if (isOptionSet(fParams.getOptions(), kMaxZPosParamKey)) {
    fMaxZPos = getOptionAsFloat(fParams.getOptions(), kMaxZPosParamKey);
  } else {
    WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.", kMaxZPosParamKey.c_str(), fMaxZPos));
  }
  if (isOptionSet(fParams.getOptions(), kMaxDistOfDecayPlaneFromCenterParamKey)) {
    fMaxDistOfDecayPlaneFromCenter = getOptionAsFloat(fParams.getOptions(), kMaxDistOfDecayPlaneFromCenterParamKey);
  } else {
    WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.", kMaxDistOfDecayPlaneFromCenterParamKey.c_str(), fMaxDistOfDecayPlaneFromCenter));
  }
  if (isOptionSet(fParams.getOptions(), kMaxTimeDiffParamKey)) {
    fMaxTimeDiff = getOptionAsFloat(fParams.getOptions(), kMaxTimeDiffParamKey);
  } else {
    WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.", kMaxTimeDiffParamKey.c_str(), fMaxTimeDiff));
  }
  if (isOptionSet(fParams.getOptions(), kBackToBackAngleWindowParamKey)) {
    fBackToBackAngleWindow = getOptionAsFloat(fParams.getOptions(), kBackToBackAngleWindowParamKey);
  } else {
    WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.", kBackToBackAngleWindowParamKey.c_str(), fBackToBackAngleWindow));
  }
  if (isOptionSet(fParams.getOptions(), kDecayInto3MinAngleParamKey)) {
    fDecayInto3MinAngle = getOptionAsFloat(fParams.getOptions(), kDecayInto3MinAngleParamKey);
  } else {
    WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.", kDecayInto3MinAngleParamKey.c_str(), fDecayInto3MinAngle));
  }

  if (fSaveControlHistos) {
    getStatistics().createHistogram(
      new TH1F("AllHitTOT", "TOT of all Hits in physics stream", 200, -0.5, 99.5)
    );
    getStatistics().getHisto1D("AllHitTOT")->SetXTitle("TOT [ns]");
    getStatistics().getHisto1D("AllHitTOT")->SetYTitle("Number of hits");

    getStatistics().createHistogram(
      new TH1F("AnnihHitsNumber", "Number of Annihilation Hits in Event", 50, -0.5, 49.5)
    );
    getStatistics().getHisto1D("AnnihHitsNumber")->SetXTitle("Number of Annihilation Hits in a Event");
    getStatistics().getHisto1D("AnnihHitsNumber")->SetYTitle("Counts");

    getStatistics().createHistogram(
      new TH1F("DeexHitsNumber", "Number of Deexcitation Hits in Event", 50, -0.5, 49.5)
    );
    getStatistics().getHisto1D("DeexHitsNumber")->SetXTitle("Number of Deexcitation Hits in Event");
    getStatistics().getHisto1D("DeexHitsNumber")->SetYTitle("Counts");

    getStatistics().createHistogram(
      new TH1F("DeexAnnihTimeDiff", "Deexcitation-Annihilation Hits Time Difference", 200, -195.5, 195.5)
    );
    getStatistics().getHisto1D("DeexAnnihTimeDiff")->SetXTitle("Time difference between deexcitation and annihilation hits [ns]");
    getStatistics().getHisto1D("DeexAnnihTimeDiff")->SetYTitle("Counts");

    getStatistics().createHistogram(
      new TH1F("2Gamma_TimeDiff", "2 Gamma Hits Time Difference", 200, 0.0, 100.0)
    );
    getStatistics().getHisto1D("2Gamma_TimeDiff")->SetXTitle("Hits time difference [ns]");
    getStatistics().getHisto1D("2Gamma_TimeDiff")->SetYTitle("Counts");

    getStatistics().createHistogram(
      new TH1F("2Gamma_ThetaDiff", "2 Gamma Hits angles", 360, -0.5, 179.5)
    );
    getStatistics().getHisto1D("2Gamma_ThetaDiff")->SetXTitle("Hits theta diff [deg]");
    getStatistics().getHisto1D("2Gamma_ThetaDiff")->SetYTitle("Counts");

    getStatistics().createHistogram(
      new TH1F("2Gamma_DLOR", "Delta LOR distance", 100, -0.5, 49.5)
    );
    getStatistics().getHisto1D("2Gamma_DLOR")->SetXTitle("Delta LOR [cm]");
    getStatistics().getHisto1D("2Gamma_DLOR")->SetYTitle("Counts");

    getStatistics().createHistogram(
      new TH1F("2Annih_TimeDiff", "2 gamma annihilation Hits Time Difference", 200, 0.0, fMaxTimeDiff/1000.0)
    );
    getStatistics().getHisto1D("2Annih_TimeDiff")->SetXTitle("Time difference between 2 annihilation hits [ns]");
    getStatistics().getHisto1D("2Annih_TimeDiff")->SetYTitle("Counts");

    getStatistics().createHistogram(
      new TH1F("2Annih_ThetaDiff", "Annihilation Hits Theta Diff",
      (int) 4*fBackToBackAngleWindow, 180.-fBackToBackAngleWindow, 180.+fBackToBackAngleWindow)
    );
    getStatistics().getHisto1D("2Annih_ThetaDiff")->SetXTitle("Annihilation hits theta diff [deg]");
    getStatistics().getHisto1D("2Annih_ThetaDiff")->SetYTitle("Counts");

    getStatistics().createHistogram(
      new TH1F("2Annih_DLOR", "Delta LOR distance", 100, -0.5, 49.5)
    );
    getStatistics().getHisto1D("2Annih_ThetaDiff")->SetXTitle("Annihilation hits Delta LOR [cm]");
    getStatistics().getHisto1D("2Annih_ThetaDiff")->SetYTitle("Counts");

    getStatistics().createHistogram(
      new TH2F("2Annih_XY", "Reconstructed XY position of annihilation point", 220, -54.5, 54.5, 220, -54.5, 54.5)
    );
    getStatistics().getHisto2D("2Annih_XY")->SetXTitle("Annihilation point X [cm]");
    getStatistics().getHisto2D("2Annih_XY")->SetYTitle("Annihilation point Y [cm]");

    getStatistics().createHistogram(
      new TH1F("2Annih_Z", "Reconstructed Z position of annihilation point", 220, -54.5, 54.5)
    );
    getStatistics().getHisto1D("2Annih_Z")->SetXTitle("Annihilation point Z [cm]");
    getStatistics().getHisto1D("2Annih_Z")->SetYTitle("Counts");

    getStatistics().createHistogram(
      new TH2F("3GammaThetas", "3 Gamma Thetas plot", 251, -0.5, 250.5, 201, -0.5, 200.5)
    );
    getStatistics().getHisto2D("3GammaThetas")->SetXTitle("Transformed thetas 1-2 [deg]");
    getStatistics().getHisto2D("3GammaThetas")->SetYTitle("Transformed thetas 2-3 [deg]");

    getStatistics().createHistogram(
      new TH1F("3GammaPlaneDist", "3 Gamma Plane Distance to Center", 200, 0.0, 50.0)
    );
    getStatistics().getHisto1D("3GammaPlaneDist")->SetXTitle("Distance [cm]");
    getStatistics().getHisto1D("3GammaPlaneDist")->SetYTitle("Counts");

    getStatistics().createHistogram(
      new TH1F("3GammaTimeDiff", "3 gamma last and first hit time difference", 200, 0.0, 20.0)
    );
    getStatistics().getHisto1D("3GammaTimeDiff")->SetXTitle("Time difference [ns]");
    getStatistics().getHisto1D("3GammaTimeDiff")->SetYTitle("Counts");

    getStatistics().createHistogram(
      new TH1F("3AnnihPlaneDist", "3 Gamma Annihilation Plane Distance to Center",
      100, 0.0, fMaxDistOfDecayPlaneFromCenter)
    );
    getStatistics().getHisto1D("3AnnihPlaneDist")->SetXTitle("Distance [cm]");
    getStatistics().getHisto1D("3AnnihPlaneDist")->SetYTitle("Counts");

    getStatistics().createHistogram(
      new TH1F("3AnnihTimeDiff", "3 gamma Annihilation last and first hit time difference", 200, 0.0, fMaxTimeDiff/1000.0)
    );
    getStatistics().getHisto1D("3AnnihTimeDiff")->SetXTitle("Time difference [ns]");
    getStatistics().getHisto1D("3AnnihTimeDiff")->SetYTitle("Counts");
  }
  return true;
}

bool EventCategorizerPhysics::exec()
{
  vector<JPetEvent> events;
  if (auto timeWindow = dynamic_cast<const JPetTimeWindow* const>(fEvent)) {
    uint n = timeWindow->getNumberOfEvents();
    for (uint i = 0; i < n; ++i) {
      const auto& event = dynamic_cast<const JPetEvent&>(timeWindow->operator[](i));
      vector<JPetHit> hits = event.getHits();
      JPetEvent physicEvent = physicsAnalysis(hits);
      if (physicEvent.getHits().size()) { events.push_back(physicEvent); }
    }
  } else {
    return false;
  }
  if (events.size()) { saveEvents(events); }
  events.clear();
  return true;
}

bool EventCategorizerPhysics::terminate()
{
  INFO("Physics streaming ended.");
  return true;
}

void EventCategorizerPhysics::saveEvents(const vector<JPetEvent>& events)
{
  for (const auto& event : events) {
    fOutputEvents->add<JPetEvent>(event);
  }
}

JPetEvent EventCategorizerPhysics::physicsAnalysis(vector<JPetHit> hits)
{
  JPetEvent physicEvent;
  JPetEvent annihilationHits;
  JPetEvent deexcitationHits;

  for (unsigned i = 0; i < hits.size(); i++) {
    if (fabs(hits[i].getPosZ()) < fMaxZPos) {
      double TOTofHit = EventCategorizerTools::calculateTOT(hits[i]);
      if (fSaveControlHistos) {
        getStatistics().getHisto1D("AllHitTOT")->Fill(TOTofHit / 1000.);
      }
      if (TOTofHit >= fMinAnnihilationTOT && TOTofHit <= fMaxAnnihilationTOT) {
        physicEvent.addHit(hits[i]);
        annihilationHits.addHit(hits[i]);
      }
      if (TOTofHit >= fMinDeexcitationTOT && TOTofHit <= fMaxDeexcitationTOT) {
        physicEvent.addHit(hits[i]);
        deexcitationHits.addHit(hits[i]);
      }
    }
  }
  if (fSaveControlHistos) {
    getStatistics().getHisto1D("AnnihHitsNumber")->Fill(annihilationHits.getHits().size());
    getStatistics().getHisto1D("DeexHitsNumber")->Fill(deexcitationHits.getHits().size());
  }
  if (deexcitationHits.getHits().size() > 0) {
    if (physicEvent.isOnlyTypeOf(JPetEventType::kUnknown)){
      physicEvent.setEventType(JPetEventType::kPrompt);
    } else {
      physicEvent.addEventType(JPetEventType::kPrompt);
    }
    if (annihilationHits.getHits().size() > 0) {
      getStatistics().getHisto1D("DeexAnnihTimeDiff")->Fill(
        annihilationHits.getHits().at(0).getTime() - deexcitationHits.getHits().at(0).getTime()
      );
    }
  }
  if (EventCategorizerTools::stream2Gamma(
    annihilationHits, getStatistics(), fSaveControlHistos,
    fBackToBackAngleWindow, fMaxTimeDiff)
  ) {
    if (physicEvent.isOnlyTypeOf(JPetEventType::kUnknown)) {
      physicEvent.setEventType(JPetEventType::k2Gamma);
    } else {
      physicEvent.addEventType(JPetEventType::k2Gamma);
    }
  }
  if (EventCategorizerTools::stream3Gamma(
    annihilationHits, getStatistics(), fSaveControlHistos,
    fDecayInto3MinAngle, fMaxTimeDiff, fMaxDistOfDecayPlaneFromCenter)
  ) {
    if (physicEvent.isOnlyTypeOf(JPetEventType::kUnknown)){
      physicEvent.setEventType(JPetEventType::k3Gamma);
    } else {
      physicEvent.addEventType(JPetEventType::k3Gamma);
    }
  }
  return physicEvent;
}
