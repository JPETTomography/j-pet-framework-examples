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
  }

  // Main and ref scin IDs
  if (isOptionSet(fParams.getOptions(), kMainScinIDParamKey)) {
    fMainScinID = getOptionAsInt(fParams.getOptions(), kMainScinIDParamKey);
  }
  if (isOptionSet(fParams.getOptions(), kRefScinIDParamKey)) {
    fRefScinID = getOptionAsInt(fParams.getOptions(), kRefScinIDParamKey);
  }

  // Time walk corrections
  if (isOptionSet(fParams.getOptions(), kTimeWalkCorrAParamKey)) {
    fTimeWalkAParam = getOptionAsDouble(fParams.getOptions(), kTimeWalkCorrAParamKey);
  }
  if (isOptionSet(fParams.getOptions(), kTimeWalkCorrBParamKey)) {
    fTimeWalkBParam = getOptionAsDouble(fParams.getOptions(), kTimeWalkCorrBParamKey);
  }

  // ToT cut params
  if (isOptionSet(fParams.getOptions(), kToTCutMinParamKey)) {
    fToTCutMin = getOptionAsDouble(fParams.getOptions(), kToTCutMinParamKey);
  }
  if (isOptionSet(fParams.getOptions(), kToTCutMaxParamKey)) {
    fToTCutMax = getOptionAsDouble(fParams.getOptions(), kToTCutMaxParamKey);
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

  // Signal merging time parameter
  if (isOptionSet(fParams.getOptions(), kMergeSignalsTimeParamKey)) {
    fMergingTime = getOptionAsDouble(fParams.getOptions(), kMergeSignalsTimeParamKey);
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

    // Corrupted filter
    if(!fUseCorruptedHits && hit.getRecoFlag()==JPetHit::Corrupted){
      count++;
      continue;
    }

    // Checking, if following hits meets selection conditions
    // if not, moving interator
    unsigned int nextCount = 1;
    while(count+nextCount < nHits) {

      auto nextHit = dynamic_cast<const JPetHit&>(timeWindow.operator[](count+nextCount));

      // coincidence condition
      if (fabs(nextHit.getTime() - hit.getTime()) < fEventTimeWindow) {

        // different scin IDs conditioin
        if(hit.getScin().getID() == nextHit.getScin().getID()) {
          nextCount++;
          continue;
        }

        // Proper coincidence found - assigning main scin hit and ref scin hit
        JPetHit mainHit, refHit;
        if(hit.getScin().getID() == kMainScinIDParamKey) {
          mainHit = hit;
          refHit = nextHit;
        } else {
          mainHit = nextHit;
          refHit = hit;
        }

        auto hitStats = getStats(mainHit);
        auto multi = get<0>(hitStats);
        auto tdiff = get<1>(hitStats);
        auto tot = get<2>(hitStats);
        auto revToT = get<3>(hitStats);

        // Checking multi cut and ToT cut
        if(multi == 16 && tot>fToTCutMin && tot<fToTCutMax) {
          // Good coincidence, creating new event
          JPetEvent event;
          event.setEventType(JPetEventType::k2Gamma);
          event.setRecoFlag(JPetEvent::Good);
          event.addHit(hit);
          event.addHit(nextHit);
          eventVec.push_back(event);

          // TODO Fill histograms
          if(fSaveControlHistos){
            getStatistics().getHisto2D("tdiff_tot")->Fill(tdiff, revToT);
            getStatistics().getHisto2D("tdiff_tot_zoom")->Fill(tdiff, revToT);
            auto correction = (revToT-fTimeWalkBParam)/fTimeWalkAParam;
            getStatistics().getHisto2D("tdiff_tot_zoom_tw")->Fill(tdiff*fTimeWalkAParam+fTimeWalkBParam, revToT);

            // Offsets histograms
            plotOffsetHistograms(mainHit.getSignalA(), "A", "main");
            plotOffsetHistograms(mainHit.getSignalB(), "B", "main");
            plotOffsetHistograms(refHit.getSignalA(), "A", "ref");
            plotOffsetHistograms(refHit.getSignalA(), "B", "ref");
          }
        }
      } else {
        getStatistics().getHisto1D("hits_rejected_tdiff")
        ->Fill(fabs(nextHit.getTime() - hit.getTime()));
      }
      break;
    }
    // moving to next uninvestigated hit
    count+=nextCount;

    // Not needed now
    // if(fSaveControlHistos) {
    //   getStatistics().getHisto1D("hits_per_event_all")->Fill(event.getHits().size());
    //   if(event.getRecoFlag()==JPetEvent::Good){
    //     getStatistics().getHisto1D("good_vs_bad_events")->Fill(1);
    //   } else if(event.getRecoFlag()==JPetEvent::Corrupted){
    //     getStatistics().getHisto1D("good_vs_bad_events")->Fill(2);
    //   } else {
    //     getStatistics().getHisto1D("good_vs_bad_events")->Fill(3);
    //   }
    // }
    // if(event.getHits().size() >= fMinMultiplicity){
    //   eventVec.push_back(event);
    //   if(fSaveControlHistos) {
    //     getStatistics().getHisto1D("hits_per_event_selected")->Fill(event.getHits().size());
    //   }
    // }
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
    "tdiff_tot", "TDiff vs. TOT",
    200, fHistoTDiffMin, fHistoTDiffMax, 200, fHistoTOTMin, fHistoTOTMax
  ));
  getStatistics().getHisto2D("tdiff_tot")->GetXaxis()->SetTitle("Time difference [ps]");
  getStatistics().getHisto2D("tdiff_tot")->GetYaxis()->SetTitle("1/TOT_B-1/TOT_A [1/ps]");

  getStatistics().createHistogram(new TH2F(
    "tdiff_tot_zoom", "TDiff vs. TOT",
    200, fZoomTDiffMin, fZoomTDiffMax, 200, fZoomTOTMin, fZoomTOTMax
  ));
  getStatistics().getHisto2D("tdiff_tot_zoom")->GetXaxis()->SetTitle("Time difference [ps]");
  getStatistics().getHisto2D("tdiff_tot_zoom")->GetYaxis()->SetTitle("1/TOT_B-1/TOT_A [1/ps]");

  getStatistics().createHistogram(new TH2F(
    "tdiff_tot_zoom_tw", "TDiff vs. TOT with time walk correction",
    200, fHistoTDiffMin/2.0, fHistoTDiffMax/2.0, 200, fZoomTOTMin, fZoomTOTMax
  ));
  getStatistics().getHisto2D("tdiff_tot_zoom_tw")->GetXaxis()->SetTitle("Time difference [ps]");
  getStatistics().getHisto2D("tdiff_tot_zoom_tw")->GetYaxis()->SetTitle("1/TOT_B-1/TOT_A [1/ps]");

  // Histograms for matrix SiPMs synchronization
  // <side, main/ref, other SiPM>
  vector<tuple<string, string, int>> histoConfs = {
    {"A", "main", 2}, {"A", "main", 3}, {"A", "main", 4},
    {"A", "ref", 2}, {"A", "ref", 3}, {"A", "ref", 4},
    {"B", "main", 2}, {"B", "main", 3}, {"B", "main", 4},
    {"B", "ref", 2}, {"B", "ref", 3}, {"B", "ref", 4},
  };
  for(auto conf : histoConfs){
    getStatistics().createHistogram(new TH1F(
      Form("mtx_offsets_%s_%s_1_%d", ((string)get<0>(conf)).c_str(), ((string)get<1>(conf)).c_str(), get<2>(conf)),
      Form("Time offsets of SiPM 1 and %d on side %s of %s strip", get<2>(conf), ((string)get<0>(conf)).c_str(), ((string)get<1>(conf)).c_str()),
      200, -1.1*fMergingTime, 1.1*fMergingTime
    ));
    getStatistics().getHisto1D(Form("mtx_offsets_%s_%s_1_%d", ((string)get<0>(conf)).c_str(), ((string)get<1>(conf)).c_str(), get<2>(conf)))->GetXaxis()->SetTitle("Time difference [ps]");
    getStatistics().getHisto1D(Form("mtx_offsets_%s_%s_1_%d", ((string)get<0>(conf)).c_str(), ((string)get<1>(conf)).c_str(), get<2>(conf)))->GetYaxis()->SetTitle("Number of Raw Signal pairs");
  }
}

/**
 * Gets stats for THR 1 and 2, returns average
 * @return <multi, tdiff, tot/multi, rev_tot>
 */
tuple<int, double, double, double> EventFinder::getStats(const JPetHit& hit)
{
  auto statsTHR1 = getStatsPerTHR(hit, 1);
  auto statsTHR2 = getStatsPerTHR(hit, 2);

  auto multi = get<0>(statsTHR1)+get<0>(statsTHR2);
  auto avTDiff = (get<1>(statsTHR1)+get<1>(statsTHR2))/2.0;
  auto avToT = (get<2>(statsTHR1)+get<2>(statsTHR2))/((double) get<0>(statsTHR1)+get<0>(statsTHR2));
  auto avRevToT = (get<3>(statsTHR1)+get<3>(statsTHR2))/2.0;

  return make_tuple(multi, avTDiff, avToT, avRevToT);
}

/**
 * @return <multi, tdiff, tot, rev_tot>
 */
tuple<int, double, double, double> EventFinder::getStatsPerTHR(const JPetHit& hit, int thrNum)
{
  int multi = 0;
  double timeA = 0.0;
  double timeB = 0.0;
  double totA = 0.0;
  double totB = 0.0;
  double revToTA = 0.0;
  double revToTB = 0.0;

  for(auto signalA : hit.getSignalA().getRawSignals()){
    auto leads = signalA.second.getTimesVsThresholdNumber(JPetSigCh::Leading);
    auto trails = signalA.second.getTimesVsThresholdNumber(JPetSigCh::Trailing);
    if(leads.find(thrNum)!=leads.end() && trails.find(thrNum)!=trails.end()){
      multi++;
      timeA += leads.at(thrNum);
      totA += trails.at(thrNum)-leads.at(thrNum);
      revToTA += 1.0/(trails.at(thrNum)-leads.at(thrNum));
    }
  }

  for(auto signalB : hit.getSignalB().getRawSignals()){
    auto leads = signalB.second.getTimesVsThresholdNumber(JPetSigCh::Leading);
    auto trails = signalB.second.getTimesVsThresholdNumber(JPetSigCh::Trailing);
    if(leads.find(thrNum)!=leads.end() && trails.find(thrNum)!=trails.end()){
      multi++;
      timeB += leads.at(thrNum);
      totB += trails.at(thrNum)-leads.at(thrNum);
      revToTB += 1.0/(trails.at(thrNum)-leads.at(thrNum));
    }
  }

  timeA = timeA/((float) hit.getSignalA().getRawSignals().size());
  timeB = timeB/((float) hit.getSignalB().getRawSignals().size());

  return make_tuple(multi, timeB-timeA, totB-totA, revToTB-revToTA);
}

/**
 * Returning time of leading Signal Channel on the first threshold from Raw Signal
 */
double EventFinder::getRawSigBaseTime(const JPetRawSignal& rawSig)
{
  return rawSig.getPoints(JPetSigCh::Leading, JPetRawSignal::ByThrValue).at(0).getTime();
}

void EventFinder::plotOffsetHistograms(const JPetMatrixSignal& mtxSig, string side, string type)
{
  auto rawSigMap = mtxSig.getRawSignals();

  // Looking for first
  auto search1 = rawSigMap.find(1);
  if (search1 != rawSigMap.end()) {
    auto t1 = getRawSigBaseTime(rawSigMap.at(1));
    for(int i=2;i<5;i++){
      auto searchI = rawSigMap.find(i);
      if (searchI != rawSigMap.end()) {
        auto tI = getRawSigBaseTime(rawSigMap.at(i));
        getStatistics().getHisto1D(Form("mtx_offsets_%s_%s_1_%d", side.c_str(), type.c_str(), i))->Fill(tI-t1);
      }
    }
  }
}
