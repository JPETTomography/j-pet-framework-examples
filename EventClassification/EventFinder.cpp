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
    fEventTimeWindow = getOptionAsDouble(fParams.getOptions(), kEventTimeParamKey);
  } else {
    WARNING(Form(
      "No value of the %s parameter provided by the user. Using default value of %lf.",
      kEventTimeParamKey.c_str(), fEventTimeWindow
    ));
  }

  // Time walk corrections
  if (isOptionSet(fParams.getOptions(), kTimeWalkCorrATHR1ParamKey)) {
    fTimeWalkAParamTHR1 = getOptionAsDouble(fParams.getOptions(), kTimeWalkCorrATHR1ParamKey);
  }
  if (isOptionSet(fParams.getOptions(), kTimeWalkCorrBTHR1ParamKey)) {
    fTimeWalkBParamTHR1 = getOptionAsDouble(fParams.getOptions(), kTimeWalkCorrBTHR1ParamKey);
  }

  if (isOptionSet(fParams.getOptions(), kTimeWalkCorrATHR2ParamKey)) {
    fTimeWalkAParamTHR2 = getOptionAsDouble(fParams.getOptions(), kTimeWalkCorrATHR2ParamKey);
  }
  if (isOptionSet(fParams.getOptions(), kTimeWalkCorrBTHR2ParamKey)) {
    fTimeWalkBParamTHR2 = getOptionAsDouble(fParams.getOptions(), kTimeWalkCorrBTHR2ParamKey);
  }

  if (isOptionSet(fParams.getOptions(), kTimeWalkCorrAAVParamKey)) {
    fTimeWalkAParamAV = getOptionAsDouble(fParams.getOptions(), kTimeWalkCorrAAVParamKey);
  }
  if (isOptionSet(fParams.getOptions(), kTimeWalkCorrBAVParamKey)) {
    fTimeWalkBParamAV = getOptionAsDouble(fParams.getOptions(), kTimeWalkCorrBAVParamKey);
  }

  // Limits of histograms
  if (isOptionSet(fParams.getOptions(), kHistoTDiffMinParamKey)) {
    fHistoTDiffMin = getOptionAsDouble(fParams.getOptions(), kHistoTDiffMinParamKey);
  }
  if (isOptionSet(fParams.getOptions(), kHistoTDiffMaxParamKey)) {
    fHistoTDiffMax = getOptionAsDouble(fParams.getOptions(), kHistoTDiffMaxParamKey);
  }

  if (isOptionSet(fParams.getOptions(), kHistoTOTMinParamKey)) {
    fHistoTOTMin = getOptionAsDouble(fParams.getOptions(), kHistoTOTMinParamKey);
  }
  if (isOptionSet(fParams.getOptions(), kHistoTOTMaxParamKey)) {
    fHistoTOTMax = getOptionAsDouble(fParams.getOptions(), kHistoTOTMaxParamKey);
  }

  // Limits of zoomed histograms
  if (isOptionSet(fParams.getOptions(), kZoomTDiffMinParamKey)) {
    fZoomTDiffMin = getOptionAsDouble(fParams.getOptions(), kZoomTDiffMinParamKey);
  }
  if (isOptionSet(fParams.getOptions(), kZoomTDiffMaxParamKey)) {
    fZoomTDiffMax = getOptionAsDouble(fParams.getOptions(), kZoomTDiffMaxParamKey);
  }

  if (isOptionSet(fParams.getOptions(), kZoomTOTMinParamKey)) {
    fZoomTOTMin = getOptionAsDouble(fParams.getOptions(), kZoomTOTMinParamKey);
  }
  if (isOptionSet(fParams.getOptions(), kZoomTOTMaxParamKey)) {
    fZoomTOTMax = getOptionAsDouble(fParams.getOptions(), kZoomTOTMaxParamKey);
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
          if(hit.getScin().getID() == 213){
            // Coincidental hits
            auto hitTHR1 = getStatsPerTHR(hit, 1);
            auto hitTHR2 = getStatsPerTHR(hit, 2);

            if(hitTHR1.first == 8) {
              getStatistics().getHisto2D("tdiff_tot_thr_1")
              ->Fill(hitTHR1.second.first, hitTHR1.second.second);
              getStatistics().getHisto2D("tdiff_tot_thr_1_z")
              ->Fill(hitTHR1.second.first, hitTHR1.second.second);

              auto correction = fTimeWalkAParamTHR1*hitTHR1.second.second+fTimeWalkBParamTHR1;
              getStatistics().getHisto2D("tdiff_tot_thr_1_z_tw")
              ->Fill(
                hitTHR1.second.first-correction,
                hitTHR1.second.second
              );
            }

            if(hitTHR2.first == 8) {
              getStatistics().getHisto2D("tdiff_tot_thr_2")
              ->Fill(hitTHR2.second.first, hitTHR2.second.second);
              getStatistics().getHisto2D("tdiff_tot_thr_2_z")
              ->Fill(hitTHR2.second.first, hitTHR2.second.second);

              auto correction = fTimeWalkAParamTHR2*hitTHR2.second.second+fTimeWalkBParamTHR2;
              getStatistics().getHisto2D("tdiff_tot_thr_2_z_tw")
              ->Fill(
                hitTHR2.second.first-correction,
                hitTHR2.second.second
              );
            }

            if(hitTHR1.first+hitTHR2.first == 16) {
              auto avTdiff = (hitTHR1.second.first+hitTHR2.second.first)/2.0;
              auto avTOT = (hitTHR1.second.second+hitTHR2.second.second)/2.0;
              getStatistics().getHisto2D("tdiff_tot_av_z")->Fill(avTdiff, avTOT);

              auto correction = fTimeWalkAParamAV*avTOT+fTimeWalkBParamAV;
              getStatistics().getHisto2D("tdiff_tot_av_z_tw")
              ->Fill(avTdiff-correction, avTOT);
            }

          }else if(nextHit.getScin().getID() == 213){
            auto nextTHR1 = getStatsPerTHR(hit, 1);
            auto nextTHR2 = getStatsPerTHR(hit, 2);

            if(nextTHR1.first == 8) {
              getStatistics().getHisto2D("tdiff_tot_thr_1")
              ->Fill(nextTHR1.second.first, nextTHR1.second.second);
              getStatistics().getHisto2D("tdiff_tot_thr_1_z")
              ->Fill(nextTHR1.second.first, nextTHR1.second.second);

              auto correction = (nextTHR1.second.second-fTimeWalkBParamTHR1)/fTimeWalkAParamTHR1;
              getStatistics().getHisto2D("tdiff_tot_thr_1_z_tw")
              ->Fill(
                nextTHR1.second.first-correction,
                nextTHR1.second.second
              );
            }

            if(nextTHR2.first == 8) {
              getStatistics().getHisto2D("tdiff_tot_thr_2")
              ->Fill(nextTHR2.second.first, nextTHR2.second.second);
              getStatistics().getHisto2D("tdiff_tot_thr_2_z")
              ->Fill(nextTHR2.second.first, nextTHR2.second.second);

              auto correction = (nextTHR2.second.second-fTimeWalkBParamTHR2)/fTimeWalkAParamTHR2;
              getStatistics().getHisto2D("tdiff_tot_thr_2_z_tw")
              ->Fill(
                nextTHR2.second.first-correction,
                nextTHR2.second.second
              );
            }

            if(nextTHR1.first+nextTHR2.first == 16) {
              auto avTdiff = (nextTHR1.second.first+nextTHR2.second.first)/2.0;
              auto avTOT = (nextTHR1.second.second+nextTHR2.second.second)/2.0;
              getStatistics().getHisto2D("tdiff_tot_av_z")->Fill(avTdiff, avTOT);
              auto correction = (avTOT-fTimeWalkBParamAV)/fTimeWalkAParamAV;
              getStatistics().getHisto2D("tdiff_tot_av_z_tw")
              ->Fill(avTdiff*fTimeWalkAParamAV+fTimeWalkBParamAV, avTOT);
            }
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
    new TH1F("hits_rejected_tdiff", "Time difference of consecutive unmatched hits",
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

  getStatistics().createHistogram(new TH1F(
    "good_vs_bad_events", "Number of good and corrupted Events created", 3, 0.5, 3.5
  ));
  getStatistics().getHisto1D("good_vs_bad_events")->GetXaxis()->SetBinLabel(1, "GOOD");
  getStatistics().getHisto1D("good_vs_bad_events")->GetXaxis()->SetBinLabel(2, "CORRUPTED");
  getStatistics().getHisto1D("good_vs_bad_events")->GetXaxis()->SetBinLabel(3, "UNKNOWN");
  getStatistics().getHisto1D("good_vs_bad_events")->GetYaxis()->SetTitle("Number of Events");

  //////////////////////////////////////////////////////////////////////////////
  getStatistics().createHistogram(new TH2F(
    "tdiff_tot_thr_1", "THR 1 TDiff vs. TOT, scin 213, multi 8",
    200, fHistoTDiffMin, fHistoTDiffMax, 200, fHistoTOTMin, fHistoTOTMax
  ));
  getStatistics().getHisto2D("tdiff_tot_thr_1")
  ->GetXaxis()->SetTitle("Time difference [ps]");
  getStatistics().getHisto2D("tdiff_tot_thr_1")
  ->GetYaxis()->SetTitle("1/TOT_B-1/TOT_A [1/ps]");

  getStatistics().createHistogram(new TH2F(
    "tdiff_tot_thr_2", "THR 2 TDiff vs. TOT, scin 213, multi 8",
    200, fHistoTDiffMin, fHistoTDiffMax, 200, fHistoTOTMin, fHistoTOTMax
  ));
  getStatistics().getHisto2D("tdiff_tot_thr_2")
  ->GetXaxis()->SetTitle("Time difference [ps]");
  getStatistics().getHisto2D("tdiff_tot_thr_2")
  ->GetYaxis()->SetTitle("1/TOT_B-1/TOT_A [1/ps]");

  getStatistics().createHistogram(new TH2F(
    "tdiff_tot_thr_1_z", "THR 1 TDiff vs. TOT, scin 213, multi 8",
    200, fZoomTDiffMin, fZoomTDiffMax, 200, fZoomTOTMin, fZoomTOTMax
  ));
  getStatistics().getHisto2D("tdiff_tot_thr_1_z")
  ->GetXaxis()->SetTitle("Time difference [ps]");
  getStatistics().getHisto2D("tdiff_tot_thr_1_z")
  ->GetYaxis()->SetTitle("1/TOT_B-1/TOT_A [1/ps]");

  getStatistics().createHistogram(new TH2F(
    "tdiff_tot_thr_2_z", "THR 2 TDiff vs. TOT, scin 213, multi 8",
    200, fZoomTDiffMin, fZoomTDiffMax, 200, fZoomTOTMin, fZoomTOTMax
  ));
  getStatistics().getHisto2D("tdiff_tot_thr_2_z")
  ->GetXaxis()->SetTitle("Time difference [ps]");
  getStatistics().getHisto2D("tdiff_tot_thr_2_z")
  ->GetYaxis()->SetTitle("1/TOT_B-1/TOT_A [1/ps]");

  getStatistics().createHistogram(new TH2F(
    "tdiff_tot_thr_1_z_tw", "THR 1 TDiff vs. TOT, scin 213, multi 8, time walk correction",
    200, fHistoTDiffMin/2.0, fHistoTDiffMax/2.0, 200, fZoomTOTMin, fZoomTOTMax
  ));
  getStatistics().getHisto2D("tdiff_tot_thr_1_z_tw")
  ->GetXaxis()->SetTitle("Time difference [ps]");
  getStatistics().getHisto2D("tdiff_tot_thr_1_z_tw")
  ->GetYaxis()->SetTitle("1/TOT_B-1/TOT_A [1/ps]");

  getStatistics().createHistogram(new TH2F(
    "tdiff_tot_thr_2_z_tw", "THR 2 TDiff vs. TOT, scin 213, multi 8, time walk correction",
    200, fHistoTDiffMin/2.0, fHistoTDiffMax/2.0, 200, fZoomTOTMin, fZoomTOTMax
  ));
  getStatistics().getHisto2D("tdiff_tot_thr_2_z_tw")
  ->GetXaxis()->SetTitle("Time difference [ps]");
  getStatistics().getHisto2D("tdiff_tot_thr_2_z_tw")
  ->GetYaxis()->SetTitle("1/TOT_B-1/TOT_A [1/ps]");

  // For average of two THRs
  getStatistics().createHistogram(new TH2F(
    "tdiff_tot_av_z", "THR average TDiff vs. TOT, scin 213, multi 16",
    200, fZoomTDiffMin, fZoomTDiffMax, 200, fZoomTOTMin, fZoomTOTMax
  ));
  getStatistics().getHisto2D("tdiff_tot_av_z")
  ->GetXaxis()->SetTitle("Time difference [ps]");
  getStatistics().getHisto2D("tdiff_tot_av_z")
  ->GetYaxis()->SetTitle("1/TOT_B-1/TOT_A [1/ps]");

  getStatistics().createHistogram(new TH2F(
    "tdiff_tot_av_z_tw", "THR average TDiff vs. TOT, scin 213, multi 16, time walk correction",
    200, fHistoTDiffMin/2.0, fHistoTDiffMax/2.0, 200, fZoomTOTMin, fZoomTOTMax
  ));
  getStatistics().getHisto2D("tdiff_tot_av_z_tw")
  ->GetXaxis()->SetTitle("Time difference [ps]");
  getStatistics().getHisto2D("tdiff_tot_av_z_tw")
  ->GetYaxis()->SetTitle("1/TOT_B-1/TOT_A [1/ps]");
}

/**
 * @return <multi <tdiff, rev_tot>>
 */
pair<int, pair<double, double>> EventFinder::getStatsPerTHR(const JPetHit& hit, int thrNum)
{
  int multi = 0;
  float timeA = 0.0;
  float timeB = 0.0;
  float totA = 0.0;
  float totB = 0.0;

  for(auto signalA : hit.getSignalA().getRawSignals()){
    auto leads = signalA.second.getTimesVsThresholdNumber(JPetSigCh::Leading);
    auto trails = signalA.second.getTimesVsThresholdNumber(JPetSigCh::Trailing);
    if(leads.find(thrNum)!=leads.end() && trails.find(thrNum)!=trails.end()){
      multi++;
      timeA += leads.at(thrNum);
      totA += 1.0/(trails.at(thrNum)-leads.at(thrNum));
    }
  }

  for(auto signalB : hit.getSignalB().getRawSignals()){
    auto leads = signalB.second.getTimesVsThresholdNumber(JPetSigCh::Leading);
    auto trails = signalB.second.getTimesVsThresholdNumber(JPetSigCh::Trailing);
    if(leads.find(thrNum)!=leads.end() && trails.find(thrNum)!=trails.end()){
      multi++;
      timeB += leads.at(thrNum);
      totB += 1.0/(trails.at(thrNum)-leads.at(thrNum));
    }
  }

  timeA = timeA/((float) hit.getSignalA().getRawSignals().size());
  timeB = timeB/((float) hit.getSignalB().getRawSignals().size());

  auto tPair = make_pair(timeB-timeA, totB-totA);
  return make_pair(multi, tPair);
}
