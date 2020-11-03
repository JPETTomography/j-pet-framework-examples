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
    CalcTDiffAB(hit);
    if(hit.getRecoFlag() == JPetHit::Good) {
      event.setRecoFlag(JPetEvent::Good);
    } else if(hit.getRecoFlag() == JPetHit::Corrupted){
      event.setRecoFlag(JPetEvent::Corrupted);
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
        CalcTDiffAB(nextHit);
        nextCount++;
      } else { break; }
    }
    count+=nextCount;
    if(fSaveControlHistos) {
      getStatistics().fillHistogram("hits_per_event_all", event.getHits().size());
    }
    if(event.getHits().size() >= fMinMultiplicity){
      eventVec.push_back(event);
    }
  }
  return eventVec;
}

void EventFinder::initialiseHistograms(){
  getStatistics().createHistogramWithAxes(
    new TH1D("hits_per_event_all", "Number of Hits in an all Events", 20, 0.5, 20.5),
                                            "Hits in Event", "Number of Hits"
  );
  
  getStatistics().createHistogramWithAxes(
    new TH2D("TDiff_AB_vs_ID_thr1", "Time difference AB vs scintillator ID for threshold 1", 500, -24.75, 24.25, 200, -0.5 , 199.5),
                                            "Time difference AB for threshold 1 [ns]", "ID of the scintillator"
  );
  getStatistics().createHistogramWithAxes(
    new TH2D("TDiff_AB_vs_ID_thr2", "Time difference AB vs scintillator ID for threshold 2", 500, -24.75, 24.25, 200, -0.5 , 199.5),
                                            "Time difference AB for threshold 2 [ns]", "ID of the scintillator"
  );
  getStatistics().createHistogramWithAxes(
    new TH2D("TDiff_AB_vs_ID_thr3", "Time difference AB vs scintillator ID for threshold 3", 500, -24.75, 24.25, 200, -0.5 , 199.5),
                                            "Time difference AB for threshold 3 [ns]", "ID of the scintillator"
  );
  getStatistics().createHistogramWithAxes(
    new TH2D("TDiff_AB_vs_ID_thr4", "Time difference AB vs scintillator ID for threshold 4", 500, -24.75, 24.25, 200, -0.5 , 199.5),
                                            "Time difference AB for threshold 4 [ns]", "ID of the scintillator"
  );
}

void EventFinder::CalcTDiffAB( JPetHit Hit )
{
	double TDiff_AB = 0.;
	std::vector<JPetSigCh> sigALead = Hit.getSignalA().getRecoSignal().getRawSignal().getPoints(JPetSigCh::Leading, JPetRawSignal::ByThrNum);
	std::vector<JPetSigCh> sigBLead = Hit.getSignalB().getRecoSignal().getRawSignal().getPoints(JPetSigCh::Leading, JPetRawSignal::ByThrNum);
	
	if (sigALead.size() > 0 && sigBLead.size() > 0) {
		for (unsigned i = 0; i < sigALead.size() && i < sigBLead.size(); i++) {
			TDiff_AB = (sigBLead.at(i).getValue() - sigALead.at(i).getValue())/1000;
			switch(i) {
				case 0:
                    getStatistics().fillHistogram("TDiff_AB_vs_ID_thr1", TDiff_AB, Hit.getScintillator().getID());
					break;
				case 1:
					getStatistics().fillHistogram("TDiff_AB_vs_ID_thr2", TDiff_AB, Hit.getScintillator().getID());
					break;						  
				case 2:
					getStatistics().fillHistogram("TDiff_AB_vs_ID_thr3", TDiff_AB, Hit.getScintillator().getID());
					break;						  
				case 3:
					getStatistics().fillHistogram("TDiff_AB_vs_ID_thr4", TDiff_AB, Hit.getScintillator().getID());
					break;						  
			}
		}
	}
}
