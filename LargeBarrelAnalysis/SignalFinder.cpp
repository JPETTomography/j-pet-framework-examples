/**
 *  @copyright Copyright 2018 The J-PET Framework Authors. All rights reserved.
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
#include "SignalFinder.h"
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

  // Get bool for using corrupted Signal Channels
  if (isOptionSet(fParams.getOptions(), kUseCorruptedSigChParamKey)) {
    fUseCorruptedSigCh = getOptionAsBool(fParams.getOptions(), kUseCorruptedSigChParamKey);
    if(fUseCorruptedSigCh){
      WARNING("Signal Finder is using Corrupted Signal Channels, as set by the user");
    } else{
      WARNING("Signal Finder is NOT using Corrupted Signal Channels, as set by the user");
    }
  } else {
    WARNING("Signal Finder is not using Corrupted Signal Channels (default option)");
  }

  // Getting bool for saving histograms
  if (isOptionSet(fParams.getOptions(), kSaveControlHistosParamKey)) {
    fSaveControlHistos = getOptionAsBool(fParams.getOptions(), kSaveControlHistosParamKey);
  }

  // Check if the user requested ordering of thresholds by value
  if (isOptionSet(fParams.getOptions(), kOrderThresholdsByValueKey)) {
    fOrderThresholdsByValue = getOptionAsBool(fParams.getOptions(), kOrderThresholdsByValueKey);
  }
  if (fOrderThresholdsByValue){
    INFO("Threshold reordering was requested. Thresholds will be ordered by their values according to provided detector setup file.");
    fThreshldOrderings = SignalFinderTools::findThresholdOrders(getParamBank());
  }
    
  // Creating control histograms
  if(fSaveControlHistos) { initialiseHistograms(); }
  return true;
}

bool SignalFinder::exec()
{
  // Getting the data from event in an apropriate format
  if(auto timeWindow = dynamic_cast<const JPetTimeWindow* const>(fEvent)) {
    // Distribute signal channels by PM IDs and filter out Corrupted SigChs if requested
    auto& sigChByPM = SignalFinderTools::getSigChByPM(timeWindow, fUseCorruptedSigCh);
    // Building signals
    auto allSignals = SignalFinderTools::buildAllSignals(
      sigChByPM, kNumOfThresholds, fSigChEdgeMaxTime, fSigChLeadTrailMaxTime,
      getStatistics(), fSaveControlHistos, fThreshldOrderings
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

void SignalFinder::saveRawSignals(const vector<JPetRawSignal>& rawSigVec)
{
  for (auto & rawSig : rawSigVec) { fOutputEvents->add<JPetRawSignal>(rawSig); }
}

void SignalFinder::initialiseHistograms(){

  getStatistics().createHistogram(new TH1F(
    "unused_sigch_all", "Unused Signal Channels", 8, 0.5, 8.5
  ));
  getStatistics().getHisto1D("unused_sigch_all")->GetXaxis()->SetBinLabel(1,"THR 1 Lead");
  getStatistics().getHisto1D("unused_sigch_all")->GetXaxis()->SetBinLabel(2,"THR 1 Trail");
  getStatistics().getHisto1D("unused_sigch_all")->GetXaxis()->SetBinLabel(3,"THR 2 Lead");
  getStatistics().getHisto1D("unused_sigch_all")->GetXaxis()->SetBinLabel(4,"THR 2 Trail");
  getStatistics().getHisto1D("unused_sigch_all")->GetXaxis()->SetBinLabel(5,"THR 3 Lead");
  getStatistics().getHisto1D("unused_sigch_all")->GetXaxis()->SetBinLabel(6,"THR 3 Trail");
  getStatistics().getHisto1D("unused_sigch_all")->GetXaxis()->SetBinLabel(7,"THR 4 Lead");
  getStatistics().getHisto1D("unused_sigch_all")->GetXaxis()->SetBinLabel(8,"THR 4 Trail");
  getStatistics().getHisto1D("unused_sigch_all")->GetYaxis()->SetTitle("Number of SigChs");

  getStatistics().createHistogram(new TH1F(
    "unused_sigch_good", "Unused Signal Channels with GOOD flag", 8, 0.5, 8.5
  ));
  getStatistics().getHisto1D("unused_sigch_good")->GetXaxis()->SetBinLabel(1,"THR 1 Lead");
  getStatistics().getHisto1D("unused_sigch_good")->GetXaxis()->SetBinLabel(2,"THR 1 Trail");
  getStatistics().getHisto1D("unused_sigch_good")->GetXaxis()->SetBinLabel(3,"THR 2 Lead");
  getStatistics().getHisto1D("unused_sigch_good")->GetXaxis()->SetBinLabel(4,"THR 2 Trail");
  getStatistics().getHisto1D("unused_sigch_good")->GetXaxis()->SetBinLabel(5,"THR 3 Lead");
  getStatistics().getHisto1D("unused_sigch_good")->GetXaxis()->SetBinLabel(6,"THR 3 Trail");
  getStatistics().getHisto1D("unused_sigch_good")->GetXaxis()->SetBinLabel(7,"THR 4 Lead");
  getStatistics().getHisto1D("unused_sigch_good")->GetXaxis()->SetBinLabel(8,"THR 4 Trail");
  getStatistics().getHisto1D("unused_sigch_good")->GetYaxis()->SetTitle("Number of SigChs");

  getStatistics().createHistogram(new TH1F(
    "unused_sigch_corr", "Unused Signal Channels with CORRUPTED flag", 8, 0.5, 8.5
  ));
  getStatistics().getHisto1D("unused_sigch_corr")->GetXaxis()->SetBinLabel(1,"THR 1 Lead");
  getStatistics().getHisto1D("unused_sigch_corr")->GetXaxis()->SetBinLabel(2,"THR 1 Trail");
  getStatistics().getHisto1D("unused_sigch_corr")->GetXaxis()->SetBinLabel(3,"THR 2 Lead");
  getStatistics().getHisto1D("unused_sigch_corr")->GetXaxis()->SetBinLabel(4,"THR 2 Trail");
  getStatistics().getHisto1D("unused_sigch_corr")->GetXaxis()->SetBinLabel(5,"THR 3 Lead");
  getStatistics().getHisto1D("unused_sigch_corr")->GetXaxis()->SetBinLabel(6,"THR 3 Trail");
  getStatistics().getHisto1D("unused_sigch_corr")->GetXaxis()->SetBinLabel(7,"THR 4 Lead");
  getStatistics().getHisto1D("unused_sigch_corr")->GetXaxis()->SetBinLabel(8,"THR 4 Trail");
  getStatistics().getHisto1D("unused_sigch_corr")->GetYaxis()->SetTitle("Number of SigChs");

  getStatistics().createHistogram(new TH1F(
    "lead_thr1_thr2_diff", "Time Difference between leading Signal Channels THR1 and THR2 in found signals",
    200, -fSigChEdgeMaxTime, fSigChEdgeMaxTime)
  );
  getStatistics().getHisto1D("lead_thr1_thr2_diff")
    ->GetXaxis()->SetTitle("time diff [ps]");
  getStatistics().getHisto1D("lead_thr1_thr2_diff")
    ->GetYaxis()->SetTitle("Number of Signal Channels Pairs");

  getStatistics().createHistogram(new TH1F(
    "lead_thr1_thr3_diff", "Time Difference between leading Signal Channels THR1 and THR3 in found signals",
    200, -fSigChEdgeMaxTime, fSigChEdgeMaxTime)
  );
  getStatistics().getHisto1D("lead_thr1_thr3_diff")
    ->GetXaxis()->SetTitle("time diff [ps]");
  getStatistics().getHisto1D("lead_thr1_thr3_diff")
    ->GetYaxis()->SetTitle("Number of Signal Channels Pairs");

  getStatistics().createHistogram(new TH1F(
    "lead_thr1_thr4_diff", "Time Difference between leading Signal Channels THR1 and THR4 in found signals",
    200, -fSigChEdgeMaxTime, fSigChEdgeMaxTime)
  );
  getStatistics().getHisto1D("lead_thr1_thr4_diff")
    ->GetXaxis()->SetTitle("time diff [ps]");
  getStatistics().getHisto1D("lead_thr1_thr4_diff")
    ->GetYaxis()->SetTitle("Number of Signal Channels Pairs");

  getStatistics().createHistogram(new TH1F(
    "lead_trail_thr1_diff", "Time Difference between leading and trailing Signal Channels THR1 in found signals",
    200, 0.0, fSigChLeadTrailMaxTime)
  );
  getStatistics().getHisto1D("lead_trail_thr1_diff")
    ->GetXaxis()->SetTitle("time diff [ps]");
  getStatistics().getHisto1D("lead_trail_thr1_diff")
    ->GetYaxis()->SetTitle("Number of Signal Channels Pairs");

  getStatistics().createHistogram(new TH1F(
    "lead_trail_thr2_diff", "Time Difference between leading and trailing Signal Channels THR2 in found signals",
    200, 0.0, fSigChLeadTrailMaxTime)
  );
  getStatistics().getHisto1D("lead_trail_thr2_diff")
    ->GetXaxis()->SetTitle("time diff [ps]");
  getStatistics().getHisto1D("lead_trail_thr2_diff")
    ->GetYaxis()->SetTitle("Number of Signal Channels Pairs");

  getStatistics().createHistogram(new TH1F(
    "lead_trail_thr3_diff", "Time Difference between leading and trailing Signal Channels THR3 in found signals",
    200, 0.0, fSigChLeadTrailMaxTime)
  );
  getStatistics().getHisto1D("lead_trail_thr3_diff")
    ->GetXaxis()->SetTitle("time diff [ps]");
  getStatistics().getHisto1D("lead_trail_thr3_diff")
    ->GetYaxis()->SetTitle("Number of Signal Channels Pairs");

  getStatistics().createHistogram(new TH1F(
    "lead_trail_thr4_diff", "Time Difference between leading and trailing Signal Channels THR4 in found signals",
    200, 0.0, fSigChLeadTrailMaxTime)
  );
  getStatistics().getHisto1D("lead_trail_thr4_diff")
    ->GetXaxis()->SetTitle("time diff [ps]");
  getStatistics().getHisto1D("lead_trail_thr4_diff")
    ->GetYaxis()->SetTitle("Number of Signal Channels Pairs");

  getStatistics().createHistogram(new TH1F(
    "good_v_bad_raw_sigs", "Number of good and corrupted signals created",
    3, 0.5, 3.5
  ));
  getStatistics().getHisto1D("good_v_bad_raw_sigs")->GetXaxis()->SetBinLabel(1,"GOOD");
  getStatistics().getHisto1D("good_v_bad_raw_sigs")->GetXaxis()->SetBinLabel(2,"CORRUPTED");
  getStatistics().getHisto1D("good_v_bad_raw_sigs")->GetXaxis()->SetBinLabel(3,"UNKNOWN");
  getStatistics().getHisto1D("good_v_bad_raw_sigs")->GetYaxis()->SetTitle("Number of Raw Signals");
}
