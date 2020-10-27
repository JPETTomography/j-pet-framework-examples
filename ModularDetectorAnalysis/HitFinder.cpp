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

  // Control histograms
  if(fSaveControlHistos) { initialiseHistograms(); }

  return true;
}

bool HitFinder::exec()
{
  if (auto timeWindow = dynamic_cast<const JPetTimeWindow* const>(fEvent)) {
    auto signalsBySlot = HitFinderTools::getSignalsByScin(timeWindow);
    auto allHits = HitFinderTools::matchAllSignals(
      signalsBySlot, fABTimeDiff, -1, getStatistics(), fSaveControlHistos
    );
    if(allHits.size()>0) { saveHits(allHits); }
  } else return false;
  return true;
}

bool HitFinder::terminate()
{
  // if (isOptionSet(fParams.getOptions(), kOffestsFileParamKey) && fSaveControlHistos) {
  //   INFO("Hit finding - printing out offsets for Scins in matrices");
  //   fOffsetsFile = getOptionAsString(fParams.getOptions(), kOffestsFileParamKey);
  //
  //   namespace pt = boost::property_tree;
  //   using namespace std;
  //
  //   pt::ptree root;
  //   pt::ptree scin_node;
  //
  //   for(int scinID = fMinScinID; scinID<=fMaxScinID; scinID++){
  //     int bin = scinID-fMinScinID+1;
  //     auto projX = getStatistics().getHisto2D("time_diff_scin")->ProjectionX("_px", bin, bin+1);
  //     scin_node.put(to_string(scinID), projX->GetMean());
  //   }
  //   root.add_child("scin_offsets", scin_node);
  //
  //   // Merging used calibration with new one - iteration alike
  //   if (isOptionSet(fParams.getOptions(), kScinCalibFileParamKey)) {
  //     auto scinCalibFileName = getOptionAsString(fParams.getOptions(), kScinCalibFileParamKey);
  //
  //     pt::ptree rootOld;
  //     pt::read_json(scinCalibFileName, rootOld);
  //
  //     pt::ptree new_root;
  //     pt::ptree new_scin_node;
  //
  //     for(int scinID = fMinScinID; scinID<=fMaxScinID; scinID++){
  //       double oldOffset = rootOld.get("scin_offsets."+to_string(scinID), 0.0);
  //       double newOffset = root.get("scin_offsets."+to_string(scinID), 0.0);
  //       new_scin_node.put(to_string(scinID), oldOffset+newOffset);
  //     }
  //     new_root.add_child("scin_offsets", new_scin_node);
  //     pt::write_json(fOffsetsFile, new_root);
  //
  //   }else{
  //     pt::write_json(fOffsetsFile, root);
  //   }
  // }

  INFO("Hit finding ended");
  return true;
}

void HitFinder::saveHits(const std::vector<JPetHit>& hits)
{
  auto sortedHits = JPetAnalysisTools::getHitsOrderedByTime(hits);

  if (fSaveControlHistos) { getStatistics().getHisto1D("hits_tslot")->Fill(hits.size()); }

  for (auto& hit : sortedHits) {
    fOutputEvents->add<JPetHit>(hit);

    if(fSaveControlHistos) {
      int multi = hit.getSignalA().getRawSignals().size() + hit.getSignalB().getRawSignals().size();
      int scinID = hit.getScin().getID();

      getStatistics().getHisto2D("hit_pos_XY")->Fill(hit.getPosY(), hit.getPosX());

      getStatistics().getHisto1D("hit_multi")->Fill(multi);
      getStatistics().getHisto2D("hit_multi_scin")->Fill(multi, scinID);

      getStatistics().getHisto2D("hit_tdiff_scin")->Fill(hit.getTimeDiff(), scinID);
      getStatistics().getHisto2D(Form("hit_tdiff_scin_m_%d", multi))->Fill(hit.getTimeDiff(), scinID);

      if(hit.getEnergy() != 0.0){
        getStatistics().getHisto2D(Form("hit_tot_scin_m_%d", multi))
        ->Fill(hit.getEnergy()/((double) multi), scinID);
      }
    }
  }
}

void HitFinder::initialiseHistograms(){

  auto minScinID = getParamBank().getScins().begin()->first;
  auto maxScinID = getParamBank().getScins().rbegin()->first;

  getStatistics().createHistogram(new TH1F(
    "hits_tslot", "Number of Hits in Time Window", 40, 0.5, 40.5
  ));
  getStatistics().getHisto1D("hits_tslot")->GetXaxis()->SetTitle("Hits in Time Slot");
  getStatistics().getHisto1D("hits_tslot")->GetYaxis()->SetTitle("Number of Time Slots");

  getStatistics().createHistogram(new TH2F(
    "hit_pos_XY", "Hit Position XY projection", 101, -50.5, 50.5, 101, -50.5, 50.5
  ));
  getStatistics().getHisto2D("hit_pos_XY")->GetXaxis()->SetTitle("Y [cm]");
  getStatistics().getHisto2D("hit_pos_XY")->GetYaxis()->SetTitle("X [cm]");

  // getStatistics().createHistogram(new TH2F(
  //   "hit_pos_scin", "Hit Position per Scintillator ID",
  //   200, -50.0, 50.0, fMaxScinID-fMinScinID+1, fMinScinID-0.5, fMaxScinID+0.5
  // ));
  // getStatistics().getHisto2D("hit_pos_scin")->GetXaxis()->SetTitle("Hit z position [cm]");
  // getStatistics().getHisto2D("hit_pos_scin")->GetYaxis()->SetTitle("ID of Scintillator");

  getStatistics().createHistogram(new TH2F(
    "hit_tdiff_scin", "Hit Time Difference per Scintillator ID",
    200, -1.1 * fABTimeDiff, 1.1 * fABTimeDiff,
    maxScinID-minScinID+1, minScinID-0.5, maxScinID+0.5
  ));
  getStatistics().getHisto2D("hit_tdiff_scin")->GetXaxis()->SetTitle("A-B time difference [ps]");
  getStatistics().getHisto2D("hit_tdiff_scin")->GetYaxis()->SetTitle("Scintillator ID");

  getStatistics().createHistogram(new TH1F(
    "hit_multi", "Number of signals from SiPMs in created hit", 12, -0.5, 11.5
  ));
  getStatistics().getHisto1D("hit_multi")->GetXaxis()->SetTitle("Number of signals");
  getStatistics().getHisto1D("hit_multi")->GetYaxis()->SetTitle("Number of Hits");

  getStatistics().createHistogram(new TH2F(
    "hit_multi_scin", "Number of signals from SiPMs in created Hit per Scin",
    12, -0.5, 11.5, maxScinID-minScinID+1, minScinID-0.5, maxScinID+0.5
  ));
  getStatistics().getHisto2D("hit_multi_scin")->GetXaxis()->SetTitle("A-B time difference [ps]");
  getStatistics().getHisto2D("hit_multi_scin")->GetYaxis()->SetTitle("Scintillator ID");

  // Time diff and TOT per multiplicity
  for(int multi = 2; multi <=8; multi++){
    getStatistics().createHistogram(new TH2F(
      Form("hit_tdiff_scin_m_%d", multi),
      Form("Hit time difference per scin, multiplicity %d", multi),
      200, -1.1 * fABTimeDiff, 1.1 * fABTimeDiff, maxScinID-minScinID+1, minScinID-0.5, maxScinID+0.5
    ));
    getStatistics().getHisto2D(Form("hit_tdiff_scin_m_%d", multi))->GetXaxis()->SetTitle("A-B time difference [ps]");
    getStatistics().getHisto2D(Form("hit_tdiff_scin_m_%d", multi))->GetYaxis()->SetTitle("Scintillator ID");

    getStatistics().createHistogram(new TH2F(
      Form("hit_tot_scin_m_%d", multi),
      Form("Hit TOT divided by multiplicity, multiplicity %d", multi),
      200, 0.0, 400000.0, maxScinID-minScinID+1, minScinID-0.5, maxScinID+0.5
    ));
    getStatistics().getHisto2D(Form("hit_tot_scin_m_%d", multi))->GetXaxis()->SetTitle("Time over Threshold [ps]");
    getStatistics().getHisto2D(Form("hit_tot_scin_m_%d", multi))->GetYaxis()->SetTitle("Scintillator ID");
  }

  // Unused sigals stats
  getStatistics().createHistogram(new TH1F(
    "remain_signals_scin", "Number of Unused Signals in Scintillator",
    maxScinID-minScinID+1, minScinID-0.5, maxScinID+0.5
  ));
  getStatistics().getHisto1D("remain_signals_scin")->GetXaxis()->SetTitle("ID of Scintillator");
  getStatistics().getHisto1D("remain_signals_scin")->GetYaxis()->SetTitle("Number of Unused Signals in Scintillator");

  getStatistics().createHistogram(new TH1F(
    "remain_signals_tdiff", "Time Diff of an unused signal and the consecutive one",
    200, fABTimeDiff, 5.0*fABTimeDiff
  ));
  getStatistics().getHisto1D("remain_signals_tdiff")->GetXaxis()->SetTitle("Time difference [ps]");
  getStatistics().getHisto1D("remain_signals_tdiff")->GetYaxis()->SetTitle("Number of Signals");
}
