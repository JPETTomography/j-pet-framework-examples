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
 *  @file EventFinder.cpp
 */

using namespace std;

#include <JPetOptionsTools/JPetOptionsTools.h>
#include <JPetWriter/JPetWriter.h>
#include "EventFinder.h"
#include <iostream>

using namespace jpet_options_tools;

/**
 * Constructor
 */
EventFinder::EventFinder(const char* name): JPetUserTask(name) {}

/**
 * Destructor
 */
EventFinder::~EventFinder() {}

/**
 * Initialize Event Finder
 */
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

  // Initialize histograms
  if (fSaveControlHistos) { initialiseHistograms(); }
  return true;
}

/**
 * Execute Event Finder
 */
bool EventFinder::exec()
{
  if (auto timeWindow = dynamic_cast<const JPetTimeWindow* const>(fEvent)) {
    saveEvents(buildEvents(*timeWindow));
  } else { return false; }
  return true;
}

/**
 * Terminate Event Finder
 */
bool EventFinder::terminate()
{
  INFO("Event fiding ended.");
  return true;
}

/**
 * Saving method
 */
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
  while(count<nHits){
    bool isCorrupted = false;
    JPetEvent event;
    event.setEventType(JPetEventType::kUnknown);
    auto hit = dynamic_cast<const JPetHit&>(timeWindow.operator[](count));
    if(hit.getRecoFlag() == JPetHit::Corrupted) { isCorrupted = true; }
    event.addHit(hit);
    unsigned int nextCount = 1;
    while(count+nextCount<nHits){
      auto nextHit = dynamic_cast<const JPetHit&>(timeWindow.operator[](count+nextCount));
      if (fabs(nextHit.getTime() - hit.getTime()) < fEventTimeWindow) {
        if(nextHit.getRecoFlag() == JPetHit::Corrupted) { isCorrupted = true; }
        event.addHit(nextHit);
        nextCount++;
      } else { break; }
    }
    count+=nextCount;
    if(event.getHits().size()>=fMinMultiplicity){
      if(isCorrupted) {
        event.setRecoFlag(JPetEvent::Corrupted);
        getStatistics().getHisto1D("good_vs_bad_events")->Fill(2);
      } else {
        event.setRecoFlag(JPetEvent::Good);
        getStatistics().getHisto1D("good_vs_bad_events")->Fill(1);
      }
      eventVec.push_back(event);
      if(fSaveControlHistos) {
        getStatistics().getHisto1D("hits_per_event")->Fill(event.getHits().size());
      }
    }
  }
  return eventVec;
}

/**
 * Init histograms
 */
void EventFinder::initialiseHistograms(){
  getStatistics().createHistogram(
    new TH1F("hits_per_event", "Number of Hits in Event", 20, 0.5, 20.5)
  );
  getStatistics().getHisto1D("hits_per_event")->GetXaxis()->SetTitle("Hits in Event");
  getStatistics().getHisto1D("hits_per_event")->GetYaxis()->SetTitle("Number of Hits");

  getStatistics().createHistogram(
    new TH1F("good_vs_bad_events", "Number of good and corrupted Events created", 2, 0.5, 2.5)
  );
  getStatistics().getHisto1D("good_vs_bad_events")->GetXaxis()->SetBinLabel(1,"GOOD");
  getStatistics().getHisto1D("good_vs_bad_events")->GetXaxis()->SetBinLabel(2,"CORRUPTED");
  getStatistics().getHisto1D("good_vs_bad_events")->GetYaxis()->SetTitle("Number of Events");
}
