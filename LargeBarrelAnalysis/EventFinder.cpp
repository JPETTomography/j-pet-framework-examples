/**
 *  @copyright Copyright 2016 The J-PET Framework Authors. All rights reserved.
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

bool EventFinder::init()
{
  INFO("Event finding started.");

  fOutputEvents = new JPetTimeWindow("JPetEvent");

  // Reading values from the user options if available
  // Event time
  if (isOptionSet(fParams.getOptions(), kEventTimeParamKey))
    fEventTimeWindow = getOptionAsDouble(fParams.getOptions(), kEventTimeParamKey);
  // Getting bool for saving histograms
  if (isOptionSet(fParams.getOptions(), kSaveControlHistosParamKey))
    fSaveControlHistos = getOptionAsBool(fParams.getOptions(), kSaveControlHistosParamKey);

  // Control histograms
  if (fSaveControlHistos)
  getStatistics().createHistogram(
    new TH1F("hits_per_event", "Number of Hits in Event", 20, 0.5, 20.5));
  getStatistics().getHisto1D("hits_per_event")
    ->GetXaxis()->SetTitle("Hits in Event");
  getStatistics().getHisto1D("hits_per_event")
    ->GetYaxis()->SetTitle("Number of Hits");
  return true;
}

bool EventFinder::exec()
{
  if (auto timeWindow = dynamic_cast<const JPetTimeWindow* const>(fEvent)) {
    saveEvents(buildEvents(*timeWindow));
  } else return false;
  return true;
}

bool EventFinder::terminate()
{
  INFO("Event fiding ended.");
  return true;
}

void EventFinder::saveEvents(const vector<JPetEvent>& events)
{
  for (const auto& event : events)
    fOutputEvents->add<JPetEvent>(event);
}

vector<JPetEvent> EventFinder::buildEvents(const JPetTimeWindow& timeWindow)
{
  vector<JPetEvent> eventVec;
  const unsigned int nHits = timeWindow.getNumberOfEvents();
  unsigned int count = 0;
  while(count<nHits){
    JPetEvent event;
    event.setEventType(JPetEventType::kUnknown);
    JPetHit hit = dynamic_cast<const JPetHit&>(timeWindow.operator[](count));
    event.addHit(hit);

    unsigned int nextCount = 1;
    while(count+nextCount<nHits){
      JPetHit nextHit = dynamic_cast<const JPetHit&>(timeWindow.operator[](count+nextCount));
      if (fabs(nextHit.getTime() - hit.getTime()) < fEventTimeWindow) {
        event.addHit(nextHit);
        nextCount++;
      } else break;
    }
    count+=nextCount;
    if(fSaveControlHistos)
      getStatistics().getHisto1D("hits_per_event")->Fill(event.getHits().size());
    eventVec.push_back(event);
  }
  return eventVec;
}
