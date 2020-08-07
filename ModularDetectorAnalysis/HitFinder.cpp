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
 *  @file HitFinder.cpp
 */

using namespace std;

#include <JPetAnalysisTools/JPetAnalysisTools.h>
#include <JPetOptionsTools/JPetOptionsTools.h>
#include <JPetWriter/JPetWriter.h>

#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include "HitFinderTools.h"
#include "HitFinder.h"
#include <string>
#include <vector>
#include <map>

using namespace jpet_options_tools;

HitFinder::HitFinder(const char* name) : JPetUserTask(name) {}

HitFinder::~HitFinder() {}

bool HitFinder::init()
{
  INFO("Hit finding Started");
  fOutputEvents = new JPetTimeWindow("JPetHit");

  // Reading values from the user options if available
  // Allowed time difference between signals on A and B sides
  if (isOptionSet(fParams.getOptions(), kABTimeDiffParamKey)) {
    fABTimeDiff = getOptionAsDouble(fParams.getOptions(), kABTimeDiffParamKey);
  } else {
    WARNING(Form(
      "No value of the %s parameter provided by the user. Using default value of %lf.",
      kABTimeDiffParamKey.c_str(), fABTimeDiff
    ));
  }
  // Getting bool for saving histograms
  if (isOptionSet(fParams.getOptions(), kSaveControlHistosParamKey)) {
    fSaveControlHistos = getOptionAsBool(fParams.getOptions(), kSaveControlHistosParamKey);
  }

  // Parameters fof reference detector
  if (isOptionSet(fParams.getOptions(), kRefDetScinIDParamKey)) {
    fRefDetScinID = getOptionAsInt(fParams.getOptions(), kRefDetScinIDParamKey);
    INFO(Form("Using reference detector - scintillator ID: %d", fRefDetScinID));
  }

  if (isOptionSet(fParams.getOptions(), kMinScinIDParamKey)) {
    fMinScinID = getOptionAsInt(fParams.getOptions(), kMinScinIDParamKey);
  } else {
    fMinScinID = getParamBank().getScins().begin()->first;
  }

  if (isOptionSet(fParams.getOptions(), kMaxScinIDParamKey)) {
    fMaxScinID = getOptionAsInt(fParams.getOptions(), kMaxScinIDParamKey);
  } else {
    fMaxScinID = getParamBank().getScins().rbegin()->first;
  }

  // Control histograms
  if(fSaveControlHistos) { initialiseHistograms(); }

  return true;
}

bool HitFinder::exec()
{
  if (auto timeWindow = dynamic_cast<const JPetTimeWindow* const>(fEvent)) {
    auto signalsBySlot = HitFinderTools::getSignalsByScin(timeWindow);
    auto allHits = HitFinderTools::matchAllSignals(
      signalsBySlot, fABTimeDiff, fRefDetScinID,
      getStatistics(), fSaveControlHistos
    );
    saveHits(allHits);
  } else return false;
  return true;
}

bool HitFinder::terminate()
{
  if (isOptionSet(fParams.getOptions(), kOffestsFileParamKey) && fSaveControlHistos) {
    INFO("Hit finding - printing out offsets for Scins in matrices");
    fOffsetsFile = getOptionAsString(fParams.getOptions(), kOffestsFileParamKey);

    namespace pt = boost::property_tree;
    using namespace std;

    pt::ptree root;
    pt::ptree scin_node;

    for(int scinID = fMinScinID; scinID<=fMaxScinID; scinID++){
      int bin = scinID-fMinScinID+1;
      auto projX = getStatistics().getHisto2D("time_diff_per_scin")->ProjectionX("_px", bin, bin+1);
      scin_node.put(to_string(scinID), projX->GetMean());
    }

    root.add_child("scin_offsets", scin_node);
    pt::write_json(fOffsetsFile, root);
  }

  INFO("Hit finding ended");
  return true;
}

void HitFinder::saveHits(const std::vector<JPetHit>& hits)
{
  auto sortedHits = JPetAnalysisTools::getHitsOrderedByTime(hits);
  for (auto& hit : sortedHits) {
    fOutputEvents->add<JPetHit>(hit);

    if(fSaveControlHistos) {
      auto multi = hit.getSignalA().getRawSignals().size() + hit.getSignalB().getRawSignals().size();

      getStatistics().getHisto2D("time_diff_per_scin")->Fill(hit.getTimeDiff(), hit.getScin().getID());
      getStatistics().getHisto2D("hit_pos_XY")->Fill(hit.getPosY(), hit.getPosX());
      getStatistics().getHisto1D("hit_sig_multi")->Fill(multi);
      getStatistics().getHisto1D("hit_sig_multi")->Fill(multi);

      auto scinID = hit.getScin().getID();
      if(scinID>=fMinScinID && scinID<=fMaxScinID){
        getStatistics().getHisto1D(Form("hit_sig_multi_scin_%d", scinID))->Fill(multi);
        getStatistics().getHisto1D(Form("hit_tdiff_scin_%d_m_%d", hit.getScin().getID(), ((int) multi)))
        ->Fill(hit.getTimeDiff());
        if(hit.getEnergy()!=0.0){
          getStatistics().getHisto1D(Form("hit_tot_scin_%d_m_%d", hit.getScin().getID(), ((int) multi)))
          ->Fill(hit.getEnergy()/((double) multi));
        }
        // stats.getHisto2D("hit_pos_per_scin")->Fill(hit.getPosZ(), hit.getScin().getID());
      }
    }

  }
}

void HitFinder::initialiseHistograms(){

  getStatistics().createHistogram(new TH1F(
    "hits_per_time_slot", "Number of Hits in Time Window", 40, -0.5, 40.5
  ));
  getStatistics().getHisto1D("hits_per_time_slot")->GetXaxis()->SetTitle("Hits in Time Slot");
  getStatistics().getHisto1D("hits_per_time_slot")->GetYaxis()->SetTitle("Number of Time Slots");

  getStatistics().createHistogram(new TH2F(
    "time_diff_per_scin", "Signals Time Difference per Scintillator ID",
    200, -1.1 * fABTimeDiff, 1.1 * fABTimeDiff,
    fMaxScinID-fMinScinID+1, fMinScinID-0.5, fMaxScinID+0.5
  ));
  getStatistics().getHisto2D("time_diff_per_scin")->GetXaxis()->SetTitle("A-B time difference [ps]");
  getStatistics().getHisto2D("time_diff_per_scin")->GetYaxis()->SetTitle("ID of Scintillator");

  getStatistics().createHistogram(new TH2F(
    "hit_pos_XY", "Hit Position XY projection", 121, -60.5, 60.5, 121, -60.5, 60.5
  ));
  getStatistics().getHisto2D("hit_pos_XY")->GetXaxis()->SetTitle("Y [cm]");
  getStatistics().getHisto2D("hit_pos_XY")->GetYaxis()->SetTitle("X [cm]");

  // getStatistics().createHistogram(new TH2F(
  //   "hit_pos_per_scin", "Hit Position per Scintillator ID",
  //   200, -50.0, 50.0, fMaxScinID-fMinScinID+1, fMinScinID-0.5, fMaxScinID+0.5
  // ));
  // getStatistics().getHisto2D("hit_pos_per_scin")
  // ->GetXaxis()->SetTitle("Hit z position [cm]");
  // getStatistics().getHisto2D("hit_pos_per_scin")
  // ->GetYaxis()->SetTitle("ID of Scintillator");

  // Multiplicity of signals in Hits
  getStatistics().createHistogram(new TH1F(
    "hit_sig_multi", "Number of signals from SiPMs in created hit", 10, -0.5, 9.5
  ));
  getStatistics().getHisto1D("hit_sig_multi")->GetXaxis()->SetTitle("Number of signals");
  getStatistics().getHisto1D("hit_sig_multi")->GetYaxis()->SetTitle("Number of Hits");

  // Time diff and TOT per scin per multi
  for(int scinID=fMinScinID; scinID<=fMaxScinID; scinID++){

    getStatistics().createHistogram(new TH1F(
      Form("hit_sig_multi_scin_%d", scinID),
      Form("Number of signals from SiPMs in created hit for scin %d", scinID),
      10, -0.5, 9.5
    ));
    getStatistics().getHisto1D(Form("hit_sig_multi_scin_%d", scinID))->GetXaxis()->SetTitle("Number of signals");
    getStatistics().getHisto1D(Form("hit_sig_multi_scin_%d", scinID))->GetYaxis()->SetTitle("Number of Hits");

    for(int multi = 2; multi <=8; multi++){
      getStatistics().createHistogram(new TH1F(
        Form("hit_tdiff_scin_%d_m_%d", scinID, multi),
        Form("Hit time difference, scin %d,  multiplicity %d", scinID, multi),
        200, -1.1 * fABTimeDiff, 1.1 * fABTimeDiff
      ));
      getStatistics().getHisto1D(Form("hit_tdiff_scin_%d_m_%d", scinID, multi))
      ->GetXaxis()->SetTitle("Time difference [ps]");
      getStatistics().getHisto1D(Form("hit_tdiff_scin_%d_m_%d", scinID, multi))
      ->GetYaxis()->SetTitle("Number of Hits");

      getStatistics().createHistogram(new TH1F(
        Form("hit_tot_scin_%d_m_%d", scinID, multi),
        Form("Hit TOT divided by multiplicity, scin %d multi %d", scinID, multi),
        200, 0.0, 400000.0
      ));
      getStatistics().getHisto1D(Form("hit_tot_scin_%d_m_%d", scinID, multi))
      ->GetXaxis()->SetTitle("Time over Threshold [ps]");
      getStatistics().getHisto1D(Form("hit_tot_scin_%d_m_%d", scinID, multi))
      ->GetYaxis()->SetTitle("Number of Hits");
    }
  }

  // Reference detector hits - signal B only
  getStatistics().createHistogram(new TH1F(
    "ref_hits_per_time_slot", "Number of Reference Detector Hits in Time Window", 40, -0.5, 40.5
  ));
  getStatistics().getHisto1D("ref_hits_per_time_slot")->GetXaxis()->SetTitle("Hits in Time Slot");
  getStatistics().getHisto1D("ref_hits_per_time_slot")->GetYaxis()->SetTitle("Number of Time Slots");

  getStatistics().createHistogram(new TH1F(
    "ref_hit_signalB_tot", "RefDet hits TOT of signal B", 200, 0.0, 400000.0
  ));
  getStatistics().getHisto1D("ref_hit_signalB_tot")->GetXaxis()->SetTitle("TOT [ps]");
  getStatistics().getHisto1D("ref_hit_signalB_tot")->GetYaxis()->SetTitle("Number of hits");

  // Unused sigals stats
  getStatistics().createHistogram(new TH1F(
    "remain_signals_per_scin", "Number of Unused Signals in Scintillator",
    fMaxScinID-fMinScinID+1, fMinScinID-0.5, fMaxScinID+0.5
  ));
  getStatistics().getHisto1D("remain_signals_per_scin")->GetXaxis()->SetTitle("ID of Scintillator");
  getStatistics().getHisto1D("remain_signals_per_scin")->GetYaxis()->SetTitle("Number of Unused Signals in Scintillator");

  getStatistics().createHistogram(new TH1F(
    "remain_signals_tdiff", "Time Diff of an unused signal and the consecutive one",
    200, fABTimeDiff, 75000.0+fABTimeDiff
  ));
  getStatistics().getHisto1D("remain_signals_tdiff")->GetXaxis()->SetTitle("Time difference [ps]");
  getStatistics().getHisto1D("remain_signals_tdiff")->GetYaxis()->SetTitle("Number of Signals");
}
