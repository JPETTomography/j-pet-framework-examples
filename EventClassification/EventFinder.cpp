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

  if (isOptionSet(fParams.getOptions(), kABTimeDiffParamKey)) {
    fABTimeDiff = getOptionAsFloat(fParams.getOptions(), kABTimeDiffParamKey);
  } else {
    WARNING(Form(
      "No value of the %s parameter provided by the user. Using default value of %lf.",
      kABTimeDiffParamKey.c_str(), fABTimeDiff
    ));
  }

  if (isOptionSet(fParams.getOptions(), kTOTCutMinParamKey)) {
    fTOTCutMin = getOptionAsFloat(fParams.getOptions(), kTOTCutMinParamKey);
  } else {
    WARNING(Form(
      "No value of the %s parameter provided by the user. Using default value of %lf.",
      kTOTCutMinParamKey.c_str(), fTOTCutMin
    ));
  }

  if (isOptionSet(fParams.getOptions(), kTOTCutMaxParamKey)) {
    fTOTCutMax = getOptionAsFloat(fParams.getOptions(), kTOTCutMaxParamKey);
  } else {
    WARNING(Form(
      "No value of the %s parameter provided by the user. Using default value of %lf.",
      kTOTCutMaxParamKey.c_str(), fTOTCutMax
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
  vector<JPetEvent> eventVec;
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
        nextCount++;

        if(fSaveControlHistos){
          if(hit.getScin().getID() != nextHit.getScin().getID()){
            // Coincidental hits
            auto multiHit =
            hit.getSignalA().getRawSignals().size()
            + hit.getSignalB().getRawSignals().size();
            auto multiNextHit =
            nextHit.getSignalA().getRawSignals().size()
            + nextHit.getSignalB().getRawSignals().size();

            // TOTs divided by multipliity
            auto totHit = hit.getEnergy()/((float) multiHit);
            auto totNext = nextHit.getEnergy()/((float) multiNextHit);
            auto revTOTHit = reverseTOTDiff(hit, multiHit);
            auto revTOTNext = reverseTOTDiff(nextHit, multiNextHit);

            // all hits
            getStatistics().getHisto1D("coin_hit_sig_multi")->Fill(multiHit);
            getStatistics().getHisto1D("coin_hit_sig_multi")->Fill(multiNextHit);

            getStatistics().getHisto1D("coin_hits_tdiff")
            ->Fill(hit.getTimeDiff());
            getStatistics().getHisto1D("coin_hits_tdiff")
            ->Fill(nextHit.getTimeDiff());

            getStatistics().getHisto1D("coin_hits_tdiff_recalc")
            ->Fill(recalculateTimeDiff(hit));
            getStatistics().getHisto1D("coin_hits_tdiff_recalc")
            ->Fill(recalculateTimeDiff(nextHit));

            getStatistics().getHisto1D("coin_hits_tot")->Fill(totHit);
            getStatistics().getHisto1D("coin_hits_tot")->Fill(totNext);
            getStatistics().getHisto1D("coin_hits_tot_reverse")->Fill(revTOTHit);
            getStatistics().getHisto1D("coin_hits_tot_reverse")->Fill(revTOTNext);

            // Per scin ID
            getStatistics().getHisto1D(Form(
              "coin_tdiff_scin_%d", hit.getScin().getID()
            ))->Fill(hit.getTimeDiff());
            getStatistics().getHisto1D(Form(
              "coin_tdiff_scin_%d", nextHit.getScin().getID()
            ))->Fill(nextHit.getTimeDiff());

            getStatistics().getHisto1D(Form(
              "coin_tdiff_scin_%d_recalc", hit.getScin().getID()
            ))->Fill(recalculateTimeDiff(hit));
            getStatistics().getHisto1D(Form(
              "coin_tdiff_scin_%d_recalc", nextHit.getScin().getID()
            ))->Fill(recalculateTimeDiff(nextHit));

            getStatistics().getHisto1D(Form(
              "coin_tot_scin_%d", hit.getScin().getID()
            ))->Fill(totHit);
            getStatistics().getHisto1D(Form(
              "coin_tot_scin_%d", nextHit.getScin().getID()
            ))->Fill(totNext);

            getStatistics().getHisto1D(Form(
              "coin_tot_scin_%d_reverse", hit.getScin().getID()
            ))->Fill(revTOTHit);
            getStatistics().getHisto1D(Form(
              "coin_tot_scin_%d_reverse", nextHit.getScin().getID()
            ))->Fill(revTOTNext);

            getStatistics().getHisto2D(Form("tdiff_tot_scin_%d", hit.getScin().getID()))
            ->Fill(hit.getTimeDiff(), totHit);
            getStatistics().getHisto2D(Form("tdiff_tot_scin_%d", nextHit.getScin().getID()))
            ->Fill(nextHit.getTimeDiff(), totNext);

            getStatistics().getHisto2D(Form("tdiff_tot_scin_%d_reverse", hit.getScin().getID()))
            ->Fill(hit.getTimeDiff(), totHit);
            getStatistics().getHisto2D(Form("tdiff_tot_scin_%d_reverse", nextHit.getScin().getID()))
            ->Fill(nextHit.getTimeDiff(), totNext);

            getStatistics().getHisto2D(Form("tdiff_tot_scin_%d_recalc", hit.getScin().getID()))
            ->Fill(recalculateTimeDiff(hit), totHit);
            getStatistics().getHisto2D(Form("tdiff_tot_scin_%d_recalc", nextHit.getScin().getID()))
            ->Fill(recalculateTimeDiff(nextHit), totNext);

            getStatistics().getHisto2D(Form("tdiff_tot_scin_%d_recalc_reverse", hit.getScin().getID()))
            ->Fill(recalculateTimeDiff(hit), revTOTHit);
            getStatistics().getHisto2D(Form("tdiff_tot_scin_%d_recalc_reverse", nextHit.getScin().getID()))
            ->Fill(recalculateTimeDiff(nextHit), revTOTNext);

            // Per scin ID and multiplicity
            getStatistics().getHisto1D(Form(
              "coin_tdiff_scin_%d_m_%d",
              hit.getScin().getID(), ((int) multiHit)
            ))->Fill(hit.getTimeDiff());
            getStatistics().getHisto1D(Form(
              "coin_tdiff_scin_%d_m_%d",
              nextHit.getScin().getID(), ((int) multiNextHit)
            ))->Fill(nextHit.getTimeDiff());

            getStatistics().getHisto1D(Form(
              "coin_tdiff_scin_%d_m_%d_recalc",
              hit.getScin().getID(), ((int) multiHit)
            ))->Fill(recalculateTimeDiff(hit));
            getStatistics().getHisto1D(Form(
              "coin_tdiff_scin_%d_m_%d_recalc",
              nextHit.getScin().getID(), ((int) multiNextHit)
            ))->Fill(recalculateTimeDiff(nextHit));

            getStatistics().getHisto1D(Form(
              "coin_tot_scin_%d_m_%d",
              hit.getScin().getID(), ((int) multiHit)
            ))->Fill(totHit);
            getStatistics().getHisto1D(Form(
              "coin_tot_scin_%d_m_%d",
              nextHit.getScin().getID(), ((int) multiNextHit)
            ))->Fill(totNext);

            getStatistics().getHisto1D(Form(
              "coin_tot_scin_%d_m_%d_reverse",
              hit.getScin().getID(), ((int) multiHit)
            ))->Fill(revTOTHit);
            getStatistics().getHisto1D(Form(
              "coin_tot_scin_%d_m_%d_reverse",
              nextHit.getScin().getID(), ((int) multiNextHit)
            ))->Fill(revTOTNext);

            getStatistics().getHisto2D(Form(
              "tdiff_tot_scin_%d_m_%d", hit.getScin().getID(), ((int) multiHit)
            ))->Fill(hit.getTimeDiff(), totHit);
            getStatistics().getHisto2D(Form(
              "tdiff_tot_scin_%d_m_%d", nextHit.getScin().getID(), ((int) multiNextHit)
            ))->Fill(nextHit.getTimeDiff(), totNext);

            getStatistics().getHisto2D(Form(
              "tdiff_tot_scin_%d_m_%d_reverse", hit.getScin().getID(), ((int) multiHit)
            ))->Fill(hit.getTimeDiff(), revTOTHit);
            getStatistics().getHisto2D(Form(
              "tdiff_tot_scin_%d_m_%d_reverse", nextHit.getScin().getID(), ((int) multiNextHit)
            ))->Fill(nextHit.getTimeDiff(), revTOTNext);

            getStatistics().getHisto2D(Form(
              "tdiff_tot_scin_%d_m_%d_recalc", hit.getScin().getID(), ((int) multiHit)
            ))->Fill(recalculateTimeDiff(hit), hit.getEnergy()/((float) multiHit));
            getStatistics().getHisto2D(Form(
              "tdiff_tot_scin_%d_m_%d_recalc", nextHit.getScin().getID(), ((int) multiNextHit)
            ))->Fill(recalculateTimeDiff(nextHit), nextHit.getEnergy()/((float) multiNextHit));

            getStatistics().getHisto2D(Form(
              "tdiff_tot_scin_%d_m_%d_recalc_reverse", hit.getScin().getID(), ((int) multiHit)
            ))->Fill(recalculateTimeDiff(hit), revTOTHit);
            getStatistics().getHisto2D(Form(
              "tdiff_tot_scin_%d_m_%d_recalc_reverse", nextHit.getScin().getID(), ((int) multiNextHit)
            ))->Fill(recalculateTimeDiff(nextHit), revTOTNext);
          }
        }
      } else {
        getStatistics().getHisto1D("hits_rejected_tdiff")
        ->Fill(fabs(nextHit.getTime() - hit.getTime()));
        break;
      }
    }
    count+=nextCount;
    if(fSaveControlHistos) {
      getStatistics().getHisto1D("hits_per_event_all")->Fill(event.getHits().size());
      if(event.getRecoFlag()==JPetEvent::Good){
        getStatistics().getHisto1D("good_vs_bad_events")->Fill(1);
      } else if(event.getRecoFlag()==JPetEvent::Corrupted){
        getStatistics().getHisto1D("good_vs_bad_events")->Fill(2);
      } else {
        getStatistics().getHisto1D("good_vs_bad_events")->Fill(3);
      }
    }
    if(event.getHits().size() >= fMinMultiplicity){
      eventVec.push_back(event);
      if(fSaveControlHistos) {
        getStatistics().getHisto1D("hits_per_event_selected")->Fill(event.getHits().size());
      }
    }
  }
  return eventVec;
}

void EventFinder::initialiseHistograms(){

  getStatistics().createHistogram(
    new TH1F("hits_rejected_tdiff", "Rej tdiff",
    200, 0.0, 500000.0)
  );
  getStatistics().getHisto1D("hits_rejected_tdiff")->GetXaxis()->SetTitle("Time difference [ps]");
  getStatistics().getHisto1D("hits_rejected_tdiff")->GetYaxis()->SetTitle("Number of Hit Pairs");

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

  //////////////////////////////////////////////////////////////////////////////
  getStatistics().createHistogram(new TH1F(
    "coin_hit_sig_multi", "Number of signals from SiPMs in pairs of coincidental hits",
    10, -0.5, 9.5
  ));
  getStatistics().getHisto1D("coin_hit_sig_multi")
  ->GetXaxis()->SetTitle("Number of SiPM signals");
  getStatistics().getHisto1D("coin_hit_sig_multi")
  ->GetYaxis()->SetTitle("Number of Hits");

  getStatistics().createHistogram(new TH1F(
    "coin_hits_tdiff", "Coincidental hits time difference",
    200, -1.1 * fABTimeDiff, 1.1 * fABTimeDiff
  ));
  getStatistics().getHisto1D("coin_hits_tdiff")
  ->GetXaxis()->SetTitle("Time difference [ps]");
  getStatistics().getHisto1D("coin_hits_tdiff")
  ->GetYaxis()->SetTitle("Number of Hit Pairs");

  getStatistics().createHistogram(new TH1F(
    "coin_hits_tdiff_recalc", "Coincidental hits recalcualted time difference",
    200, -1.1 * fABTimeDiff, 1.1 * fABTimeDiff
  ));
  getStatistics().getHisto1D("coin_hits_tdiff_recalc")
  ->GetXaxis()->SetTitle("Time difference [ps]");
  getStatistics().getHisto1D("coin_hits_tdiff_recalc")
  ->GetYaxis()->SetTitle("Number of Hit Pairs");

  getStatistics().createHistogram(new TH1F(
    "coin_hits_tot", "Coincidental hits TOT",
    200, 0.0, 375000.0
  ));
  getStatistics().getHisto1D("coin_hits_tot")
  ->GetXaxis()->SetTitle("Time over Threshold [ps]");
  getStatistics().getHisto1D("coin_hits_tot")
  ->GetYaxis()->SetTitle("Number of Hit Pairs");

  getStatistics().createHistogram(new TH1F(
    "coin_hits_tot_reverse", "Coincidental hits - signal reversed TOT A-B difference",
    200, -0.0002, 0.0002
  ));
  getStatistics().getHisto1D("coin_hits_tot_reverse")
  ->GetXaxis()->SetTitle("1/TOT_B-1/TOT_A [1/ps]");
  getStatistics().getHisto1D("coin_hits_tot_reverse")
  ->GetYaxis()->SetTitle("Number of Hit Pairs");

  auto minScinID = getParamBank().getScins().begin()->first;
  auto maxScinID = 215;

  // Time diff and TOT per scin per multi
  for(int scinID = minScinID; scinID <= maxScinID; scinID++){
    if(scinID != 201 && scinID != 213) { continue; }

    getStatistics().createHistogram(new TH1F(
      Form("coin_tdiff_scin_%d", scinID),
      Form("Hits coincidence, time difference, scin %d", scinID),
      200, -1.1 * fABTimeDiff, 1.1 * fABTimeDiff
    ));
    getStatistics().getHisto1D(Form("coin_tdiff_scin_%d", scinID))
    ->GetXaxis()->SetTitle("Time difference [ps]");
    getStatistics().getHisto1D(Form("coin_tdiff_scin_%d", scinID))
    ->GetYaxis()->SetTitle("Number of Hits");

    getStatistics().createHistogram(new TH1F(
      Form("coin_tdiff_scin_%d_recalc", scinID),
      Form("Hits coincidence, recalcualted time difference, scin %d", scinID),
      200, -1.1 * fABTimeDiff, 1.1 * fABTimeDiff
    ));
    getStatistics().getHisto1D(Form("coin_tdiff_scin_%d_recalc", scinID))
    ->GetXaxis()->SetTitle("Time difference [ps]");
    getStatistics().getHisto1D(Form("coin_tdiff_scin_%d_recalc", scinID))
    ->GetYaxis()->SetTitle("Number of Hits");

    getStatistics().createHistogram(new TH1F(
      Form("coin_tot_scin_%d", scinID),
      Form("Hits coincidence, TOT divided by multiplicity, scin %d", scinID),
      200, 0.0, 375000.0
    ));
    getStatistics().getHisto1D(Form("coin_tot_scin_%d", scinID))
    ->GetXaxis()->SetTitle("Time over Threshold [ps]");
    getStatistics().getHisto1D(Form("coin_tot_scin_%d", scinID))
    ->GetYaxis()->SetTitle("Number of Hits");

    getStatistics().createHistogram(new TH1F(
      Form("coin_tot_scin_%d_reverse", scinID),
      Form("Hits coincidence, TOT divided by multiplicity, scin %d", scinID),
      200, -0.0002, 0.0002
    ));
    getStatistics().getHisto1D(Form("coin_tot_scin_%d_reverse", scinID))
    ->GetXaxis()->SetTitle("1/TOT_B-1/TOT_A [1/ps]");
    getStatistics().getHisto1D(Form("coin_tot_scin_%d_reverse", scinID))
    ->GetYaxis()->SetTitle("Number of Hits");

    getStatistics().createHistogram(new TH2F(
      Form("tdiff_tot_scin_%d", scinID),
      Form("Hits coincidence, time difference vs. TOT divided by multiplicity, scin %d",  scinID),
      200, -1.1 * fABTimeDiff, 1.1 * fABTimeDiff, 200, 0.0, 375000.0
    ));
    getStatistics().getHisto2D(Form("tdiff_tot_scin_%d", scinID))
    ->GetXaxis()->SetTitle("Time difference [ps]");
    getStatistics().getHisto2D(Form("tdiff_tot_scin_%d", scinID))
    ->GetYaxis()->SetTitle("Time over Threshold [ps]");

    getStatistics().createHistogram(new TH2F(
      Form("tdiff_tot_scin_%d_recalc", scinID),
      Form("Hits coincidence, recalcualted time difference vs. TOT divided by multiplicity, scin %d",  scinID),
      200, -1.1 * fABTimeDiff, 1.1 * fABTimeDiff, 200, 0.0, 375000.0
    ));
    getStatistics().getHisto2D(Form("tdiff_tot_scin_%d_recalc", scinID))
    ->GetXaxis()->SetTitle("Time difference [ps]");
    getStatistics().getHisto2D(Form("tdiff_tot_scin_%d_recalc", scinID))
    ->GetYaxis()->SetTitle("Time over Threshold [ps]");

    getStatistics().createHistogram(new TH2F(
      Form("tdiff_tot_scin_%d_reverse", scinID),
      Form("Hits coincidence, time difference vs. TOT reversed, scin %d",  scinID),
      200, -1.1 * fABTimeDiff, 1.1 * fABTimeDiff, 200, -0.0002, 0.0002
    ));
    getStatistics().getHisto2D(Form("tdiff_tot_scin_%d_reverse", scinID))
    ->GetXaxis()->SetTitle("Time difference [ps]");
    getStatistics().getHisto2D(Form("tdiff_tot_scin_%d_reverse", scinID))
    ->GetYaxis()->SetTitle("1/TOT_B-1/TOT_A [1/ps]");

    getStatistics().createHistogram(new TH2F(
      Form("tdiff_tot_scin_%d_recalc_reverse", scinID),
      Form("Hits coincidence, recalcualted time difference vs. TOT reversed, scin %d",  scinID),
      200, -1.1 * fABTimeDiff, 1.1 * fABTimeDiff, 200, -0.0002, 0.0002
    ));
    getStatistics().getHisto2D(Form("tdiff_tot_scin_%d_recalc", scinID))
    ->GetXaxis()->SetTitle("Time difference [ps]");
    getStatistics().getHisto2D(Form("tdiff_tot_scin_%d_recalc", scinID))
    ->GetYaxis()->SetTitle("1/TOT_B-1/TOT_A [1/ps]");

    for(int multi = 2; multi <=8; multi++){

      getStatistics().createHistogram(new TH1F(
        Form("coin_tdiff_scin_%d_m_%d", scinID, multi),
        Form("Hits coincidence, time difference, scin %d, multi %d", scinID, multi),
        200, -1.1 * fABTimeDiff, 1.1 * fABTimeDiff
      ));
      getStatistics().getHisto1D(Form("coin_tdiff_scin_%d_m_%d", scinID, multi))
      ->GetXaxis()->SetTitle("Time difference [ps]");
      getStatistics().getHisto1D(Form("coin_tdiff_scin_%d_m_%d", scinID, multi))
      ->GetYaxis()->SetTitle("Number of Hits");

      getStatistics().createHistogram(new TH1F(
        Form("coin_tdiff_scin_%d_m_%d_recalc", scinID, multi),
        Form("Hits coincidence, recalcualted time difference, scin %d, multi %d", scinID, multi),
        200, -1.1 * fABTimeDiff, 1.1 * fABTimeDiff
      ));
      getStatistics().getHisto1D(Form("coin_tdiff_scin_%d_m_%d_recalc", scinID, multi))
      ->GetXaxis()->SetTitle("Time difference [ps]");
      getStatistics().getHisto1D(Form("coin_tdiff_scin_%d_m_%d_recalc", scinID, multi))
      ->GetYaxis()->SetTitle("Number of Hits");

      getStatistics().createHistogram(new TH1F(
        Form("coin_tot_scin_%d_m_%d", scinID, multi),
        Form("Hits coincidence, TOT divided by multiplicity, scin %d multi %d", scinID, multi),
        200, 0.0, 375000.0
      ));
      getStatistics().getHisto1D(Form("coin_tot_scin_%d_m_%d", scinID, multi))
      ->GetXaxis()->SetTitle("Time over Threshold [ps]");
      getStatistics().getHisto1D(Form("coin_tot_scin_%d_m_%d", scinID, multi))
      ->GetYaxis()->SetTitle("Number of Hits");

      getStatistics().createHistogram(new TH1F(
        Form("coin_tot_scin_%d_m_%d_reverse", scinID, multi),
        Form("Hits coincidence, TOT reversed, scin %d multi %d", scinID, multi),
        200, -0.0002, 0.0002
      ));
      getStatistics().getHisto1D(Form("coin_tot_scin_%d_m_%d_reverse", scinID, multi))
      ->GetXaxis()->SetTitle("1/TOT_B-1/TOT_A [1/ps]");
      getStatistics().getHisto1D(Form("coin_tot_scin_%d_m_%d_reverse", scinID, multi))
      ->GetYaxis()->SetTitle("Number of Hits");

      getStatistics().createHistogram(new TH2F(
        Form("tdiff_tot_scin_%d_m_%d", scinID, multi),
        Form("Hits coincidence, time difference vs. TOT divided by multiplicity, scin %d multi %d",
        scinID, multi),
        200, -1.1 * fABTimeDiff, 1.1 * fABTimeDiff, 200, 0.0, 375000.0
      ));
      getStatistics().getHisto2D(Form("tdiff_tot_scin_%d_m_%d", scinID, multi))
      ->GetXaxis()->SetTitle("Time difference [ps]");
      getStatistics().getHisto2D(Form("tdiff_tot_scin_%d_m_%d", scinID, multi))
      ->GetYaxis()->SetTitle("Time over Threshold [ps]");

      getStatistics().createHistogram(new TH2F(
        Form("tdiff_tot_scin_%d_m_%d_reverse", scinID, multi),
        Form("Hits coincidence, time difference vs. TOT reversed, scin %d multi %d",
        scinID, multi),
        200, -1.1 * fABTimeDiff, 1.1 * fABTimeDiff, 200, -0.0002, 0.0002
      ));
      getStatistics().getHisto2D(Form("tdiff_tot_scin_%d_m_%d", scinID, multi))
      ->GetXaxis()->SetTitle("Time difference [ps]");
      getStatistics().getHisto2D(Form("tdiff_tot_scin_%d_m_%d", scinID, multi))
      ->GetYaxis()->SetTitle("1/TOT_B-1/TOT_A [1/ps]");

      getStatistics().createHistogram(new TH2F(
        Form("tdiff_tot_scin_%d_m_%d_recalc", scinID, multi),
        Form("Hits coincidence, recalcualted time difference vs. TOT divided by multiplicity, scin %d multi %d",
        scinID, multi),
        200, -1.1 * fABTimeDiff, 1.1 * fABTimeDiff, 200, 0.0, 375000.0
      ));
      getStatistics().getHisto2D(Form("tdiff_tot_scin_%d_m_%d_recalc", scinID, multi))
      ->GetXaxis()->SetTitle("Time difference [ps]");
      getStatistics().getHisto2D(Form("tdiff_tot_scin_%d_m_%d_recalc", scinID, multi))
      ->GetYaxis()->SetTitle("Time over Threshold [ps]");

      getStatistics().createHistogram(new TH2F(
        Form("tdiff_tot_scin_%d_m_%d_recalc_reverse", scinID, multi),
        Form("Hits coincidence, recalcualted time difference vs. TOT reversed, scin %d multi %d",
        scinID, multi),
        200, -1.1 * fABTimeDiff, 1.1 * fABTimeDiff, 200, -0.0002, 0.0002
      ));
      getStatistics().getHisto2D(Form("tdiff_tot_scin_%d_m_%d_recalc_reverse", scinID, multi))
      ->GetXaxis()->SetTitle("Time difference [ps]");
      getStatistics().getHisto2D(Form("tdiff_tot_scin_%d_m_%d_recalc_reverse", scinID, multi))
      ->GetYaxis()->SetTitle("1/TOT_B-1/TOT_A [1/ps]");
    }
  }

  getStatistics().createHistogram(new TH1F(
    "good_vs_bad_events", "Number of good and corrupted Events created", 3, 0.5, 3.5
  ));
  getStatistics().getHisto1D("good_vs_bad_events")->GetXaxis()->SetBinLabel(1, "GOOD");
  getStatistics().getHisto1D("good_vs_bad_events")->GetXaxis()->SetBinLabel(2, "CORRUPTED");
  getStatistics().getHisto1D("good_vs_bad_events")->GetXaxis()->SetBinLabel(3, "UNKNOWN");
  getStatistics().getHisto1D("good_vs_bad_events")->GetYaxis()->SetTitle("Number of Events");
}

float EventFinder::recalculateTimeDiff(const JPetHit& hit)
{
  auto signalARaws = hit.getSignalA().getRawSignals();
  auto signalBRaws = hit.getSignalB().getRawSignals();

  float timeA = 0.0;
  float timeB = 0.0;

  for(auto rawSig : signalARaws){
    timeA += rawSig.second.getPoints(JPetSigCh::Leading, JPetRawSignal::ByThrValue).at(0).getTime();
  }

  for(auto rawSig : signalBRaws){
    timeB += rawSig.second.getPoints(JPetSigCh::Leading, JPetRawSignal::ByThrValue).at(0).getTime();
  }

  timeA = timeA/((float) signalARaws.size());
  timeB = timeB/((float) signalBRaws.size());
  return timeB - timeA;
}

float EventFinder::reverseTOTDiff(const JPetHit& hit, int multi)
{
  double totA = 0.0;
  double totB = 0.0;

  for(auto rawSig: hit.getSignalA().getRawSignals()){
    auto sigALead = rawSig.second.getPoints(JPetSigCh::Leading, JPetRawSignal::ByThrNum);
    auto sigATrail = rawSig.second.getPoints(JPetSigCh::Trailing, JPetRawSignal::ByThrNum);
    if (sigALead.size() > 0 && sigATrail.size() > 0){
      for (unsigned i = 0; i < sigALead.size() && i < sigATrail.size(); i++){
        totA += (sigATrail.at(i).getTime() - sigALead.at(i).getTime());
      }
    }
  }

  for(auto rawSig: hit.getSignalB().getRawSignals()){
    auto sigBLead = rawSig.second.getPoints(JPetSigCh::Leading, JPetRawSignal::ByThrNum);
    auto sigBTrail = rawSig.second.getPoints(JPetSigCh::Trailing, JPetRawSignal::ByThrNum);
    if (sigBLead.size() > 0 && sigBTrail.size() > 0){
      for (unsigned i = 0; i < sigBLead.size() && i < sigBTrail.size(); i++){
        totB += (sigBTrail.at(i).getTime() - sigBLead.at(i).getTime());
      }
    }
  }

  totA = totA/((float) multi);
  totB = totB/((float) multi);

  if(totA != 0.0 && totB != 0.0){
    return 1/totB - 1/totA;
  } else {
    return 0.0;
  }
}
