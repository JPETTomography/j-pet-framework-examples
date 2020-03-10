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
 *  @file SignalFinder.cpp
 */

using namespace std;

#include <JPetOptionsTools/JPetOptionsTools.h>
#include <JPetTimeWindow/JPetTimeWindow.h>
#include <JPetWriter/JPetWriter.h>
#include "SignalFinderTools.h"
#include "SignalFinder.h"
#include <TRandom.h>
#include <utility>
#include <string>
#include <vector>

using namespace jpet_options_tools;

SignalFinder::SignalFinder(const char* name): JPetUserTask(name) {}

SignalFinder::~SignalFinder() {}

bool SignalFinder::init()
{
  INFO("Signal finding started.");
  fOutputEvents = new JPetTimeWindow("JPetRawSignal");

  // Reading values from the user options if available
  // Time window parameter for leading edge
  if (isOptionSet(fParams.getOptions(), kEdgeMaxTimeParamKey)) {
    fSigChEdgeMaxTime = getOptionAsFloat(fParams.getOptions(), kEdgeMaxTimeParamKey);
  } else {
    WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.",
      kEdgeMaxTimeParamKey.c_str(), fSigChEdgeMaxTime)
    );
  }

  // Time window parameter for leading-trailing comparison
  if (isOptionSet(fParams.getOptions(), kLeadTrailMaxTimeParamKey)) {
    fSigChLeadTrailMaxTime = getOptionAsFloat(fParams.getOptions(), kLeadTrailMaxTimeParamKey);
  } else {
    WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.",
      kLeadTrailMaxTimeParamKey.c_str(), fSigChLeadTrailMaxTime));
  }

  // Getting bool for saving histograms
  if (isOptionSet(fParams.getOptions(), kSaveControlHistosParamKey)) {
    fSaveControlHistos = getOptionAsBool(fParams.getOptions(), kSaveControlHistosParamKey);
  }

  // Creating control histograms
  if(fSaveControlHistos) { initialiseHistograms(); }
  return true;
}

bool SignalFinder::exec()
{
  // Getting the data from event in an apropriate format
  if(auto timeWindow = dynamic_cast<const JPetTimeWindow* const>(fEvent)) {
    // Distribute signal channels by PM IDs
    auto& sigChByPM = SignalFinderTools::getSigChByPM(timeWindow);
    // Building signals
    auto allSignals = SignalFinderTools::buildAllSignals(
      sigChByPM, fSigChEdgeMaxTime, fSigChLeadTrailMaxTime,
      kNumOfThresholds, getStatistics(), fSaveControlHistos
    );
    // Saving method invocation
    saveRawSignals(allSignals);
  } else { return false; }
  return true;
}

bool SignalFinder::terminate()
{
  INFO("Signal finding ended.");
  return true;
}

/**
 * Saving Raw Signals that have leading-trailing pairs,
 * otherwise filling histogram with incomplete signals
 */
void SignalFinder::saveRawSignals(const vector<JPetRawSignal>& rawSigVec)
{
  if(fSaveControlHistos){
    getStatistics().getHisto1D("rawsig_tslot")->Fill(rawSigVec.size());
  }
  for (auto & rawSig : rawSigVec) {
    auto leads = rawSig.getPoints(JPetSigCh::Leading, JPetRawSignal::ByThrValue);
    auto trails = rawSig.getPoints(JPetSigCh::Trailing, JPetRawSignal::ByThrValue);

    auto pmID = leads.at(0).getChannel().getPM().getID();

    if(leads.size()==trails.size()){

      fOutputEvents->add<JPetRawSignal>(rawSig);

      if(fSaveControlHistos && gRandom->Uniform()<fScalingFactor){

        getStatistics().getHisto1D("rawsig_per_pm")
        ->Fill(rawSig.getPM().getID());
        getStatistics().getHisto1D("rawsig_per_scin")
        ->Fill(rawSig.getPM().getScin().getID());
        getStatistics().getHisto1D("rawsig_multi")
        ->Fill(leads.size()+trails.size());

        for(auto& sigCh : leads){
          getStatistics().getHisto1D("rawsig_mtx_occ")
          ->Fill(sigCh.getChannel().getPM().getMatrixPosition());
          getStatistics().getHisto1D("rawsig_thr_occ")
          ->Fill(sigCh.getChannel().getThresholdNumber());

          for(auto& sigCh : trails){
            getStatistics().getHisto1D("rawsig_mtx_occ")
            ->Fill(sigCh.getChannel().getPM().getMatrixPosition());
            getStatistics().getHisto1D("rawsig_thr_occ")
            ->Fill(sigCh.getChannel().getThresholdNumber());
          }
        }

        if(leads.size() == 2){
          getStatistics().getHisto1D("lead_thr1_thr2_diff")
          ->Fill(leads.at(1).getTime()-leads.at(0).getTime());
        }

        if(trails.size() == 2){
          getStatistics().getHisto1D("trail_thr1_thr2_diff")
          ->Fill(trails.at(1).getTime()-trails.at(0).getTime());
        }
      }
    }
  }
}

void SignalFinder::initialiseHistograms(){

  auto minPMID = getParamBank().getPMs().begin()->first;
  auto maxPMID = getParamBank().getPMs().rbegin()->first;
  auto minScinD = getParamBank().getScins().begin()->first;
  auto maxScinID = getParamBank().getScins().rbegin()->first;

  // Unused objects stats
  getStatistics().createHistogram(new TH1F(
    "unused_sigch_thr", "Unused Signal Channels per THR (downscaled)", 4, 0.5, 4.5
  ));
  getStatistics().getHisto1D("unused_sigch_thr")->GetXaxis()->SetBinLabel(1,"THR 1 Lead");
  getStatistics().getHisto1D("unused_sigch_thr")->GetXaxis()->SetBinLabel(2,"THR 1 Trail");
  getStatistics().getHisto1D("unused_sigch_thr")->GetXaxis()->SetBinLabel(3,"THR 2 Lead");
  getStatistics().getHisto1D("unused_sigch_thr")->GetXaxis()->SetBinLabel(4,"THR 2 Trail");
  getStatistics().getHisto1D("unused_sigch_thr")->GetYaxis()->SetTitle("Number of SigChs");

  getStatistics().createHistogram(new TH1F(
    "unused_sigch_pm", "Unused Signal Channels per SiPM",
    maxPMID-minPMID+1, minPMID-0.5, maxPMID+0.5
  ));
  getStatistics().getHisto1D("unused_sigch_pm")->GetXaxis()->SetTitle("SiPM ID");
  getStatistics().getHisto1D("unused_sigch_pm")->GetYaxis()->SetTitle("Number of Signal Channels");

  // Occupancies and multiplicities
  getStatistics().createHistogram(new TH1F(
    "rawsig_per_pm", "Raw Signals per SiPM",
    maxPMID-minPMID+1, minPMID-0.5, maxPMID+0.5
  ));
  getStatistics().getHisto1D("rawsig_per_pm")->GetXaxis()->SetTitle("SiPM ID");
  getStatistics().getHisto1D("rawsig_per_pm")->GetYaxis()->SetTitle("Number of Raw Signals");

  getStatistics().createHistogram(new TH1F(
    "rawsig_per_scin", "Raw Signals per scintillator",
    maxScinID-minScinD+1, minScinD-0.5, maxScinID+0.5
  ));
  getStatistics().getHisto1D("rawsig_per_scin")->GetXaxis()->SetTitle("Scin ID");
  getStatistics().getHisto1D("rawsig_per_scin")->GetYaxis()->SetTitle("Number of Raw Signals");

  getStatistics().createHistogram(new TH1F(
    "rawsig_thr_occ", "Thresholds occupation in createds Raw Signals",
    3, 0.5, 3.5
  ));
  getStatistics().getHisto1D("rawsig_thr_occ")->GetXaxis()->SetTitle("Threshold number");
  getStatistics().getHisto1D("rawsig_thr_occ")->GetYaxis()->SetTitle("Number of Signal Channels");

  getStatistics().createHistogram(new TH1F(
    "rawsig_mtx_occ", "Matrix SiPMs occupation in createds Raw Signals",
    5, -0.5, 4.5
  ));
  getStatistics().getHisto1D("rawsig_mtx_occ")->GetXaxis()->SetTitle("SiPM matrix position number");
  getStatistics().getHisto1D("rawsig_mtx_occ")->GetYaxis()->SetTitle("Number of Signal Channels");

  getStatistics().createHistogram(new TH1F(
    "rawsig_multi", "Raw Signal Multiplicity", 10, 0.5, 10.5
  ));
  getStatistics().getHisto1D("rawsig_multi")->GetXaxis()->SetTitle("Total number of SigChs in RawSig");
  getStatistics().getHisto1D("rawsig_multi")->GetYaxis()->SetTitle("Number of Signal Channels");

  getStatistics().createHistogram(new TH1F(
    "rawsig_tslot", "Number of Raw Signals in Time Window", 100, -0.5, 99.5
  ));
  getStatistics().getHisto1D("rawsig_tslot")->GetXaxis()->SetTitle("Number of Raw Signal in Time Window");
  getStatistics().getHisto1D("rawsig_tslot")->GetYaxis()->SetTitle("Number of Time Windows");


  // Time differences
  getStatistics().createHistogram(new TH1F(
    "lead_thr1_thr2_diff",
    "Time Difference between leading Signal Channels THR1 and THR2 in found signals",
    200, -fSigChEdgeMaxTime, fSigChEdgeMaxTime
  ));
  getStatistics().getHisto1D("lead_thr1_thr2_diff")
    ->GetXaxis()->SetTitle("time diff [ps]");
  getStatistics().getHisto1D("lead_thr1_thr2_diff")
    ->GetYaxis()->SetTitle("Number of Signal Channels Pairs");

  getStatistics().createHistogram(new TH1F(
    "trail_thr1_thr2_diff",
    "Time Difference between trailing Signal Channels THR1 and THR2 in found signals",
    200, -fSigChEdgeMaxTime, fSigChEdgeMaxTime
  ));
  getStatistics().getHisto1D("trail_thr1_thr2_diff")
  ->GetXaxis()->SetTitle("time diff [ps]");
  getStatistics().getHisto1D("trail_thr1_thr2_diff")
  ->GetYaxis()->SetTitle("Number of Signal Channels Pairs");

  getStatistics().createHistogram(new TH1F(
    "lead_trail_thr1_diff",
    "Time Difference between leading and trailing Signal Channels THR1 in found signals",
    200, 0.0, fSigChLeadTrailMaxTime
  ));
  getStatistics().getHisto1D("lead_trail_thr1_diff")
    ->GetXaxis()->SetTitle("time diff [ps]");
  getStatistics().getHisto1D("lead_trail_thr1_diff")
    ->GetYaxis()->SetTitle("Number of Signal Channels Pairs");

  getStatistics().createHistogram(new TH1F(
    "lead_trail_thr2_diff",
    "Time Difference between leading and trailing Signal Channels THR2 in found signals",
    200, 0.0, fSigChLeadTrailMaxTime
  ));
  getStatistics().getHisto1D("lead_trail_thr2_diff")
    ->GetXaxis()->SetTitle("time diff [ps]");
  getStatistics().getHisto1D("lead_trail_thr2_diff")
    ->GetYaxis()->SetTitle("Number of Signal Channels Pairs");
}
