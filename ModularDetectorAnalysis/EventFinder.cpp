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

#include "EventFinder.h"
#include "EventCategorizerTools.h"
#include <Hits/JPetMCRecoHit/JPetMCRecoHit.h>
#include <Hits/JPetRecoHit/JPetRecoHit.h>
#include <JPetOptionsTools/JPetOptionsTools.h>
#include <JPetRawMCHit/JPetRawMCHit.h>
#include <JPetWriter/JPetWriter.h>
#include <iostream>

using namespace jpet_options_tools;

EventFinder::EventFinder(const char* name) : JPetUserTask(name) {}

EventFinder::~EventFinder() {}

bool EventFinder::init()
{
  INFO("Event finding started.");

  fOutputEvents = new JPetTimeWindow("JPetEvent");

  // Reading values from the user options if available
  // Getting bool for using corrupted hits
  if (isOptionSet(fParams.getOptions(), kUseCorruptedHitsParamKey))
  {
    fUseCorruptedHits = getOptionAsBool(fParams.getOptions(), kUseCorruptedHitsParamKey);
    if (fUseCorruptedHits)
    {
      WARNING("Event Finder is using Corrupted Hits, as set by the user");
    }
    else
    {
      WARNING("Event Finder is NOT using Corrupted Hits, as set by the user");
    }
  }
  else
  {
    WARNING("Event Finder is not using Corrupted Hits (default option)");
  }

  // Event time window
  if (isOptionSet(fParams.getOptions(), kEventTimeParamKey))
  {
    fEventTimeWindow = getOptionAsDouble(fParams.getOptions(), kEventTimeParamKey);
  }

  // Minimum number of hits in an event to save an event
  if (isOptionSet(fParams.getOptions(), kEventMinMultiplicity))
  {
    fMinMultiplicity = getOptionAsInt(fParams.getOptions(), kEventMinMultiplicity);
  }
  else
  {
    WARNING(Form("No value of the %s parameter provided by the user. Using default value of %d.", kEventMinMultiplicity.c_str(), fMinMultiplicity));
  }

  // Getting bool for saving histograms
  if (isOptionSet(fParams.getOptions(), kSaveControlHistosParamKey))
  {
    fSaveControlHistos = getOptionAsBool(fParams.getOptions(), kSaveControlHistosParamKey);
  }

  // Initialize histograms
  if (fSaveControlHistos)
  {
    initialiseHistograms();
  }
  return true;
}

bool EventFinder::exec()
{
  vector<JPetEvent> eventVec;
  if (auto timeWindow = dynamic_cast<const JPetTimeWindow* const>(fEvent))
  {
    saveEvents(buildEvents(*timeWindow));
  }
  else
  {
    return false;
  }
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
  {
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

  while (count < nHits)
  {
    auto hit = dynamic_cast<const JPetBaseHit*>(&timeWindow.operator[](count));

    // If Event contains hits of reco class, then check corrupted data filter
    if (dynamic_cast<const JPetRecoHit*>(hit))
    {
      if (!fUseCorruptedHits && dynamic_cast<const JPetRecoHit*>(hit)->getRecoFlag() == JPetRecoHit::Corrupted)
      {
        count++;
        continue;
      }
    }

    // Creating new event with the first hit
    JPetEvent event;
    event.setEventType(JPetEventType::kUnknown);
    event.addHit(hit);

    // If hit is reco class, then check set corrupted data flag approptiately
    if (dynamic_cast<const JPetRecoHit*>(hit))
    {
      if (dynamic_cast<const JPetRecoHit*>(hit)->getRecoFlag() == JPetRecoHit::Good)
      {
        event.setRecoFlag(JPetEvent::Good);
      }
      else if (dynamic_cast<const JPetRecoHit*>(hit)->getRecoFlag() == JPetRecoHit::Corrupted)
      {
        event.setRecoFlag(JPetEvent::Corrupted);
      }
    }

    // If this is a Monte Carlo generated hit, set flag to MC
    if (dynamic_cast<const JPetMCRecoHit*>(hit) || dynamic_cast<const JPetRawMCHit*>(hit))
    {
      event.setRecoFlag(JPetEvent::MC);
    }

    // Checking, if following hits fulfill time window condition, then moving the interator
    unsigned int nextCount = 1;
    while (count + nextCount < nHits)
    {
      auto nextHit = dynamic_cast<const JPetBaseHit*>(&timeWindow.operator[](count + nextCount));
      auto tDiff = fabs(nextHit->getTime() - hit->getTime());
      getStatistics().fillHistogram("event_hits_tdiff_all", tDiff);
      if (tDiff < fEventTimeWindow)
      {
        // Reco flag check
        if (dynamic_cast<const JPetRecoHit*>(nextHit))
        {
          if (dynamic_cast<const JPetRecoHit*>(nextHit)->getRecoFlag() == JPetRecoHit::Corrupted)
          {
            event.setRecoFlag(JPetEvent::Corrupted);
          }
        }
        event.addHit(nextHit);
        nextCount++;
      }
      else
      {
        if (fSaveControlHistos)
        {
          getStatistics().fillHistogram("event_hits_tdiff_rejected", tDiff);
        }
        break;
      }
    }
    count += nextCount;
    if (fSaveControlHistos)
    {
      getStatistics().fillHistogram("event_multi_all", event.getHits().size());
      if (event.getRecoFlag() == JPetEvent::Good)
      {
        getStatistics().fillHistogram("reco_flags_events", 1);
      }
      else if (event.getRecoFlag() == JPetEvent::Corrupted)
      {
        getStatistics().fillHistogram("reco_flags_events", 2);
      }
      else if (event.getRecoFlag() == JPetEvent::MC)
      {
        getStatistics().fillHistogram("reco_flags_events", 3);
      }
      else
      {
        getStatistics().fillHistogram("reco_flags_events", 4);
      }
    }

    if (event.getHits().size() >= fMinMultiplicity)
    {
      eventVec.push_back(event);
      if (fSaveControlHistos)
      {
        getStatistics().fillHistogram("event_multi_selected", event.getHits().size());
      }
    }
  }
  return eventVec;
}

void EventFinder::initialiseHistograms()
{
  getStatistics().createHistogramWithAxes(new TH1D("event_hits_tdiff_all", "Time difference of consecutive hits", 200, 0.0, 200000.0),
                                          "Time difference [ps]", "Number of Hit Pairs");

  getStatistics().createHistogramWithAxes(new TH1D("event_hits_tdiff_rejected", "Time difference of consecutive unmatched hits", 200, 0.0, 200000.0),
                                          "Time difference [ps]", "Number of Hit Pairs");

  getStatistics().createHistogramWithAxes(new TH1D("event_multi_all", "Number of Hits in all Events", 20, 0.5, 20.5), "Hits in Event",
                                          "Number of Hits");

  getStatistics().createHistogramWithAxes(new TH1D("event_multi_selected", "Number of Hits in selected Events", 20, 0.5, 20.5), "Hits in Event",
                                          "Number of Hits");

  getStatistics().createHistogramWithAxes(new TH1D("reco_flags_events", "Reconstruction flags of created events", 5, 0.5, 5.5), " ",
                                          "Number of Channel Signals");
  vector<pair<unsigned, string>> binLabels = {make_pair(1, "GOOD"), make_pair(2, "CORRUPTED"), make_pair(3, "MC"), make_pair(4, "UNKNOWN"),
                                              make_pair(5, "")};
  getStatistics().setHistogramBinLabel("reco_flags_events", getStatistics().AxisLabel::kXaxis, binLabels);
}
