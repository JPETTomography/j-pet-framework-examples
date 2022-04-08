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
 *  @file EventFinder.cpp
 */

using namespace std;

#include <JPetOptionsTools/JPetOptionsTools.h>
#include <JPetWriter/JPetWriter.h>
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
  // Number of thresholds for which TDiffAB histograms should be filled
  if (isOptionSet(fParams.getOptions(), kNmbOfThresholdsParamKey)) {
    fNmbOfThresholds = getOptionAsInt(fParams.getOptions(), kNmbOfThresholdsParamKey);
  } else {
    WARNING(Form(
      "No value of the %s parameter provided by the user. Using default value of %d.",
      kNmbOfThresholdsParamKey.c_str(), fNmbOfThresholds
    ));
  }
  
  // Initialize histograms
  if (fSaveControlHistos) { initialiseHistograms(); }
  return true;
}

bool EventFinder::exec()
{
  if (auto timeWindow = dynamic_cast<const JPetTimeWindow* const>(fEvent)) {
    saveEvents(buildEvents(*timeWindow));
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
  while(count<nHits){
    auto hit = dynamic_cast<const JPetHit&>(timeWindow.operator[](count));
    if(!fUseCorruptedHits && hit.getRecoFlag()==JPetHit::Corrupted){
      count++;
      continue;
    }
    // Creating new event with the first hit
    JPetEvent event;
    event.setEventType(JPetEventType::kUnknown);
    event.addHit(hit);
    if(hit.getRecoFlag() == JPetHit::Good) {
      event.setRecoFlag(JPetEvent::Good);
    } else if(hit.getRecoFlag() == JPetHit::Corrupted){
      event.setRecoFlag(JPetEvent::Corrupted);
    }
    if (fSaveControlHistos) {
      PlotTDiffAB(hit);
    }
    // Checking, if following hits fulfill time window condition,
    // then moving interator 
    unsigned int nextCount = 1;
    while(count+nextCount < nHits){
      auto nextHit = dynamic_cast<const JPetHit&>(timeWindow.operator[](count+nextCount));
      if (fabs(nextHit.getTime() - hit.getTime()) < fEventTimeWindow) {
        if(nextHit.getRecoFlag() == JPetHit::Corrupted) {
          event.setRecoFlag(JPetEvent::Corrupted);
        }
        event.addHit(nextHit);
        if (fSaveControlHistos) {
          PlotTDiffAB(nextHit);
        }
        nextCount++;
      } else { break; }
    }
    count+=nextCount;
    if(fSaveControlHistos) {
      getStatistics().fillHistogram("hits_per_event_all", event.getHits().size());
      if(event.getRecoFlag()==JPetEvent::Good){
        getStatistics().fillHistogram("good_vs_bad_events", 1);
      } else if(event.getRecoFlag()==JPetEvent::Corrupted){
        getStatistics().fillHistogram("good_vs_bad_events", 2);
      } else {
        getStatistics().fillHistogram("good_vs_bad_events", 3);
      }
    }
    if(event.getHits().size() >= fMinMultiplicity){
      eventVec.push_back(event);
      if(fSaveControlHistos) {
        getStatistics().fillHistogram("hits_per_event_selected", event.getHits().size());
      }
    }
  }
  return eventVec;
}

void EventFinder::initialiseHistograms(){
  getStatistics().createHistogramWithAxes(
    new TH1D("hits_per_event_all", "Number of Hits in an all Events", 20, 0.5, 20.5),
                                            "Hits in Event", "Number of Hits");

  getStatistics().createHistogramWithAxes(
    new TH1D("hits_per_event_selected", "Number of Hits in selected Events (min. multiplicity)",
                                            20, fMinMultiplicity-0.5, fMinMultiplicity+19.5),
                                            "Hits in Event", "Number of Hits");

  getStatistics().createHistogramWithAxes(
    new TH1D("good_vs_bad_events", "Number of good and corrupted Events created",
                                            3, 0.5, 3.5), "Quality", "Number of Events");
  std::vector<std::pair<unsigned, std::string>> binLabels;
  binLabels.push_back(std::make_pair(1,"GOOD"));
  binLabels.push_back(std::make_pair(2,"CORRUPTED"));
  binLabels.push_back(std::make_pair(3,"UNKNOWN"));
  getStatistics().setHistogramBinLabel("good_vs_bad_events",
                                       getStatistics().AxisLabel::kXaxis, binLabels);
  
  auto minScinID = getParamBank().getScintillators().begin()->first;
  auto maxScinID = getParamBank().getScintillators().rbegin()->first;
//minScinID for LargeBarrel is 1 and max is 193 -> histogram should have range from 0.5 to 192.5 -> 193 bins
  for (unsigned thrNum=1; thrNum<=fNmbOfThresholds; thrNum++) {
    getStatistics().createHistogramWithAxes(
        new TH2D(Form("TDiff_AB_vs_ID_thr%d", thrNum), Form("Time difference AB vs scintillator ID for threshold %d", thrNum), 500, -24750, 24250, 
                 maxScinID-minScinID+1, minScinID-0.5, maxScinID+0.5), "Time difference AB [ps]", "ID of the scintillator"
    );
  }
}

void EventFinder::PlotTDiffAB(JPetHit Hit)
{
  double TDiff_AB = 0.;
  std::map<int, double> sigALead = Hit.getSignalA().getRecoSignal().getRawSignal().getTimesVsThresholdNumber(JPetSigCh::Leading);
  std::map<int, double> sigBLead = Hit.getSignalB().getRecoSignal().getRawSignal().getTimesVsThresholdNumber(JPetSigCh::Leading);
  unsigned ScintID = Hit.getScintillator().getID();

  if (sigALead.size()>0 && sigBLead.size()>0) {
    for (unsigned i=1; i<=sigALead.size() && i<=sigBLead.size(); i++) {
      if (sigBLead.find(i) != sigBLead.end() || sigALead.find(i) != sigALead.end()) {
        TDiff_AB = (sigBLead.find(i)->second - sigALead.find(i)->second);
        getStatistics().fillHistogram(Form("TDiff_AB_vs_ID_thr%d", i), TDiff_AB, ScintID);
      }
    }
  }
}
