/**
 *  @copyright Copyright 2020 The J-PET Framework Authors. All rights reserved.
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
 *  @file EventFinder.cpp
 */

using namespace std;

#include <JPetOptionsTools/JPetOptionsTools.h>
#include <JPetWriter/JPetWriter.h>
#include "EventCategorizerTools.h"
#include "EventFinder.h"
#include <iostream>

using namespace jpet_options_tools;

EventFinder::EventFinder(const char* name): JPetUserTask(name) {}

EventFinder::~EventFinder() {}

bool EventFinder::init()
{
  INFO("Event finding started.");

  fOutputEvents = new JPetTimeWindow("JPetEvent");

  // Reading values from the user options if available
  // Getting bool for using corrupted hits
  if (isOptionSet(fParams.getOptions(), kUseCorruptedHitsParamKey)) {
    fUseCorruptedHits = getOptionAsBool(fParams.getOptions(), kUseCorruptedHitsParamKey);
    if(fUseCorruptedHits){
      WARNING("Event Finder is using Corrupted Hits, as set by the user");
    } else{
      WARNING("Event Finder is NOT using Corrupted Hits, as set by the user");
    }
  } else {
    WARNING("Event Finder is not using Corrupted Hits (default option)");
  }
  // Event time window
  if (isOptionSet(fParams.getOptions(), kEventTimeParamKey)) {
    fEventTimeWindow = getOptionAsFloat(fParams.getOptions(), kEventTimeParamKey);
  } else {
    WARNING(Form(
      "No value of the %s parameter provided by the user. Using default value of %lf.",
      kEventTimeParamKey.c_str(), fEventTimeWindow
    ));
  }
  // Minimum number of hits in an event to save an event
  if (isOptionSet(fParams.getOptions(), kEventMinMultiplicity)) {
    fMinMultiplicity = getOptionAsInt(fParams.getOptions(), kEventMinMultiplicity);
  } else {
    WARNING(Form(
      "No value of the %s parameter provided by the user. Using default value of %d.",
      kEventMinMultiplicity.c_str(), fMinMultiplicity
    ));
  }
  // Getting bool for saving histograms
  if (isOptionSet(fParams.getOptions(), kSaveControlHistosParamKey)){
    fSaveControlHistos = getOptionAsBool(fParams.getOptions(), kSaveControlHistosParamKey);
  }

  if (isOptionSet(fParams.getOptions(), kABTimeDiffParamKey)) {
    fABTimeDiff = getOptionAsFloat(fParams.getOptions(), kABTimeDiffParamKey);
  }

  // Parameters fof reference detector
  if (isOptionSet(fParams.getOptions(), kRefDetScinIDParamKey)) {
    fRefDetScinID = getOptionAsInt(fParams.getOptions(), kRefDetScinIDParamKey);
    INFO(Form("Using reference detector - scintillator ID: %d", fRefDetScinID));
  }

  // Initialize histograms
  if (fSaveControlHistos) { initialiseHistograms(); }
  return true;
}

bool EventFinder::exec()
{
  vector<JPetEvent> eventVec;
  if (auto timeWindow = dynamic_cast<const JPetTimeWindow* const>(fEvent)) {
    saveEvents(buildEvents(*timeWindow));

    // saveEvents(eventVec);
  } else { return false; }
  return true;
}

bool EventFinder::terminate()
{
  INFO("Event fiding ended.");
  return true;
}

void EventFinder::saveEvents(const vector<JPetEvent>& events)
{
  for (const auto& event : events){
    fOutputEvents->add<JPetEvent>(event);
  }
}

/**
 * Main method of building Events - Hit in the Time slot are groupped
 * within time parameter, that can be set by the user
 */
vector<JPetEvent> EventFinder::buildEvents(const JPetTimeWindow& timeWindow)
{
  vector<JPetEvent> eventVec;
  const unsigned int nHits = timeWindow.getNumberOfEvents();
  unsigned int count = 0;
  while(count<nHits-2){
    auto firstHit = dynamic_cast<const JPetHit&>(timeWindow.operator[](count));
    auto secondHit = dynamic_cast<const JPetHit&>(timeWindow.operator[](count+1));

    if (fabs(secondHit.getTime() - firstHit.getTime()) < fEventTimeWindow
    && secondHit.getScin().getSlot().getID() != firstHit.getScin().getSlot().getID()) {
      JPetEvent event;
      event.setEventType(JPetEventType::k2Gamma);
      event.addHit(firstHit);
      event.addHit(secondHit);
      eventVec.push_back(event);

      if(fSaveControlHistos) {
        if(firstHit.getScin().getID()==fRefDetScinID){
          auto multiRef = firstHit.getSignalB().getRawSignals().size();
          getStatistics().getHisto1D("ref_pm_hits_tdiff")
          ->Fill(firstHit.getTimeDiff());
          getStatistics().getHisto1D("ref_pm_hits_tot")
          ->Fill(firstHit.getEnergy()/((float) multiRef));

          auto multiSecond = secondHit.getSignalA().getRawSignals().size()
            + secondHit.getSignalB().getRawSignals().size();
          getStatistics().getHisto1D(
            Form("hit_tdiff_scin_%d_m_%d", secondHit.getScin().getID(), ((int) multiSecond))
          )->Fill(secondHit.getTimeDiff());
          getStatistics().getHisto1D(
            Form("hit_tot_scin_%d_m_%d", secondHit.getScin().getID(), ((int) multiSecond))
          )->Fill(secondHit.getEnergy()/((float) multiSecond));

        } else if(secondHit.getScin().getID()==fRefDetScinID){
          auto multiRef = secondHit.getSignalB().getRawSignals().size();
          getStatistics().getHisto1D("ref_pm_hits_tdiff")
          ->Fill(secondHit.getTimeDiff());
          getStatistics().getHisto1D("ref_pm_hits_tot")
          ->Fill(secondHit.getEnergy()/((float) multiRef));

          auto multiFirst = firstHit.getSignalA().getRawSignals().size()
            + firstHit.getSignalB().getRawSignals().size();
          getStatistics().getHisto1D(
            Form("hit_tdiff_scin_%d_m_%d", firstHit.getScin().getID(), ((int) multiFirst))
          )->Fill(firstHit.getTimeDiff());
          getStatistics().getHisto1D(
            Form("hit_tot_scin_%d_m_%d", firstHit.getScin().getID(), ((int) multiFirst))
          )->Fill(firstHit.getEnergy()/((float) multiFirst));

        } else {
          auto multiFirst = firstHit.getSignalA().getRawSignals().size()
            + firstHit.getSignalB().getRawSignals().size();
          auto multiSecond = secondHit.getSignalA().getRawSignals().size()
            + secondHit.getSignalB().getRawSignals().size();

          getStatistics().getHisto1D(
            Form("hit_tdiff_scin_%d_m_%d", firstHit.getScin().getID(), ((int) multiFirst))
          )->Fill(firstHit.getTimeDiff());
          getStatistics().getHisto1D(
            Form("hit_tot_scin_%d_m_%d", firstHit.getScin().getID(), ((int) multiFirst))
          )->Fill(firstHit.getEnergy()/((float) multiFirst));

          getStatistics().getHisto1D(
            Form("hit_tdiff_scin_%d_m_%d", secondHit.getScin().getID(), ((int) multiSecond))
          )->Fill(secondHit.getTimeDiff());
          getStatistics().getHisto1D(
            Form("hit_tot_scin_%d_m_%d", secondHit.getScin().getID(), ((int) multiSecond))
          )->Fill(secondHit.getEnergy()/((float) multiSecond));
        }
      }
      count += 2;
      if(count >= nHits-2) { break; }
    } else {
      count++;
      if(count >= nHits-2) { break; }
    }
  }

  // const unsigned int nHits = timeWindow.getNumberOfEvents();
  // unsigned int count = 0;
  // while(count<nHits){
  //   auto hit = dynamic_cast<const JPetHit&>(timeWindow.operator[](count));
  //   if(!fUseCorruptedHits && hit.getRecoFlag()==JPetHit::Corrupted){
  //     count++;
  //     continue;
  //   }
  //   // Creating new event with the first hit
  //   JPetEvent event;
  //   event.setEventType(JPetEventType::kUnknown);
  //   event.addHit(hit);
  //   if(hit.getRecoFlag() == JPetHit::Good) {
  //     event.setRecoFlag(JPetEvent::Good);
  //   } else if(hit.getRecoFlag() == JPetHit::Corrupted){
  //     event.setRecoFlag(JPetEvent::Corrupted);
  //   }
  //   // Checking, if following hits fulfill time window condition,
  //   // then moving interator
  //   unsigned int nextCount = 1;
  //   while(count+nextCount < nHits){
  //     auto nextHit = dynamic_cast<const JPetHit&>(timeWindow.operator[](count+nextCount));
  //     if (fabs(nextHit.getTime() - hit.getTime()) < fEventTimeWindow) {
  //       if(nextHit.getRecoFlag() == JPetHit::Corrupted) {
  //         event.setRecoFlag(JPetEvent::Corrupted);
  //       }
  //       event.addHit(nextHit);
  //       nextCount++;
  //     } else { break; }
  //   }
  //   count+=nextCount;
  //   if(fSaveControlHistos) {
  //     getStatistics().getHisto1D("hits_per_event_all")->Fill(event.getHits().size());
  //     if(event.getRecoFlag()==JPetEvent::Good){
  //       getStatistics().getHisto1D("good_vs_bad_events")->Fill(1);
  //     } else if(event.getRecoFlag()==JPetEvent::Corrupted){
  //       getStatistics().getHisto1D("good_vs_bad_events")->Fill(2);
  //     } else {
  //       getStatistics().getHisto1D("good_vs_bad_events")->Fill(3);
  //     }
  //   }
  //   if(event.getHits().size() >= fMinMultiplicity){
  //     eventVec.push_back(event);
  //     if(fSaveControlHistos) {
  //       getStatistics().getHisto1D("hits_per_event_selected")->Fill(event.getHits().size());
  //     }
  //   }
  // }
  return eventVec;
}

void EventFinder::initialiseHistograms(){

  getStatistics().createHistogram(new TH1F(
    "ref_pm_hits_tdiff",
    "RefDet hits signals time diff",
    200, -1.1 * fABTimeDiff, 1.1 * fABTimeDiff
  ));
  getStatistics().getHisto1D("ref_pm_hits_tdiff")->GetXaxis()->SetTitle("Time difference [ps]");
  getStatistics().getHisto1D("ref_pm_hits_tdiff")->GetYaxis()->SetTitle("Number of hits");

  getStatistics().createHistogram(new TH1F(
    "ref_pm_hits_tot",
    "RefDet hits TOT",
    200, 0.0, 375000.0
  ));
  getStatistics().getHisto1D("ref_pm_hits_tot")->GetXaxis()->SetTitle("TOT [ps]");
  getStatistics().getHisto1D("ref_pm_hits_tot")->GetYaxis()->SetTitle("Number of hits");

  auto minScinID = getParamBank().getScins().begin()->first;
  auto maxScinID = 230;

  // Time diff and TOT per scin per multi
  for(int scinID = minScinID; scinID<= maxScinID; scinID++){
    for(int multi = 2; multi <=8; multi++){

      getStatistics().createHistogram(new TH1F(
        Form("hit_tdiff_scin_%d_m_%d", scinID, multi),
        Form("Module 1 Hit time difference, scin %d,  multiplicity %d", scinID, multi),
        200, -1.1 * fABTimeDiff, 1.1 * fABTimeDiff
      ));
      getStatistics().getHisto1D(Form("hit_tdiff_scin_%d_m_%d", scinID, multi))
      ->GetXaxis()->SetTitle("Time difference [ps]");
      getStatistics().getHisto1D(Form("hit_tdiff_scin_%d_m_%d", scinID, multi))
      ->GetYaxis()->SetTitle("Number of Hits");

      getStatistics().createHistogram(new TH1F(
        Form("hit_tot_scin_%d_m_%d", scinID, multi),
        Form("Module 1  Hit TOT divided by multiplicity, scin %d multi %d", scinID, multi),
        200, 0.0, 375000.0
      ));
      getStatistics().getHisto1D(Form("hit_tot_scin_%d_m_%d", scinID, multi))
      ->GetXaxis()->SetTitle("Time over Threshold [ps]");
      getStatistics().getHisto1D(Form("hit_tot_scin_%d_m_%d", scinID, multi))
      ->GetYaxis()->SetTitle("Number of Hits");
    }
  }

  ////////////////////////////////////// old
  getStatistics().createHistogram(
    new TH1F("hits_per_event_all", "Number of Hits in an all Events", 20, 0.5, 20.5)
  );
  getStatistics().getHisto1D("hits_per_event_all")->GetXaxis()->SetTitle("Hits in Event");
  getStatistics().getHisto1D("hits_per_event_all")->GetYaxis()->SetTitle("Number of Hits");

  getStatistics().createHistogram(
    new TH1F("hits_per_event_selected", "Number of Hits in selected Events (min. multiplicity)",
    20, fMinMultiplicity-0.5, fMinMultiplicity+19.5)
  );
  getStatistics().getHisto1D("hits_per_event_selected")->GetXaxis()->SetTitle("Hits in Event");
  getStatistics().getHisto1D("hits_per_event_selected")->GetYaxis()->SetTitle("Number of Hits");

  getStatistics().createHistogram(
    new TH1F("good_vs_bad_events", "Number of good and corrupted Events created", 3, 0.5, 3.5)
  );
  getStatistics().getHisto1D("good_vs_bad_events")->GetXaxis()->SetBinLabel(1,"GOOD");
  getStatistics().getHisto1D("good_vs_bad_events")->GetXaxis()->SetBinLabel(2,"CORRUPTED");
  getStatistics().getHisto1D("good_vs_bad_events")->GetXaxis()->SetBinLabel(3,"UNKNOWN");
  getStatistics().getHisto1D("good_vs_bad_events")->GetYaxis()->SetTitle("Number of Events");
}
