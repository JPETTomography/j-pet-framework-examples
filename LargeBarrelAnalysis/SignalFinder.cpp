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
  // Reference detector photomultiplier identifier
  if (isOptionSet(fParams.getOptions(), kRefPMIDParamKey)) {
    fRefPMID = getOptionAsInt(fParams.getOptions(), kRefPMIDParamKey);
    WARNING("Signal Finder is ignoring Corrupted Signal Channels from the Refference detector as specified by the user (photomultiplier number + std::to_string(fRefPMID)");
  }
  else{
    WARNING("Signal Finder is ignoring Corrupted Signal Channels from the Refference detector(default photomultiplier number + std::to_string(fRefPMID)");
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
    fThresholdOrderings = SignalFinderTools::findThresholdOrders(getParamBank());
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
    auto& sigChByPM = SignalFinderTools::getSigChByPM(timeWindow, fUseCorruptedSigCh, fRefPMID);
    // Building signals
    auto allSignals = SignalFinderTools::buildAllSignals(
      sigChByPM, fSigChEdgeMaxTime, fSigChLeadTrailMaxTime,
      getStatistics(), fSaveControlHistos, fThresholdOrderings
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
  getStatistics().createHistogramWithAxes(
    new TH1D("unused_sigch_all", "Unused Signal Channels",
                                            8, 0.5, 8.5), "Signal label", "Number of SigChs");
  std::vector<std::pair<unsigned, std::string>> binLabels;
  binLabels.push_back(std::make_pair(1,"THR 1 Lead"));
  binLabels.push_back(std::make_pair(2,"THR 1 Trail"));
  binLabels.push_back(std::make_pair(3,"THR 2 Lead"));
  binLabels.push_back(std::make_pair(4,"THR 2 Trail"));
  binLabels.push_back(std::make_pair(5,"THR 3 Lead"));
  binLabels.push_back(std::make_pair(6,"THR 3 Trail"));
  binLabels.push_back(std::make_pair(7,"THR 4 Lead"));
  binLabels.push_back(std::make_pair(8,"THR 4 Trail"));
  getStatistics().setHistogramBinLabel("unused_sigch_all",
                                       getStatistics().AxisLabel::kXaxis, binLabels);

  getStatistics().createHistogramWithAxes(
    new TH1D("unused_sigch_good", "Unused Signal Channels with GOOD flag",
                                            8, 0.5, 8.5), "Signal label", "Number of SigChs");
  getStatistics().setHistogramBinLabel("unused_sigch_good",
                                       getStatistics().AxisLabel::kXaxis, binLabels);

  getStatistics().createHistogramWithAxes(
    new TH1D("unused_sigch_corr", "Unused Signal Channels with CORRUPTED flag",
                                            8, 0.5, 8.5), "Signal label", "Number of SigChs");
  getStatistics().setHistogramBinLabel("unused_sigch_corr",
                                       getStatistics().AxisLabel::kXaxis, binLabels);

  getStatistics().createHistogramWithAxes(
      new TH1D("lead_thr1_thr2_diff", "Time Difference between leading Signal Channels THR1 and THR2 in found signals",
                                                    2.0*fSigChEdgeMaxTime/100.0, -fSigChEdgeMaxTime+125.0, fSigChEdgeMaxTime-125.0),
                                                    "time diff [ps]", "Number of Signal Channels Pairs");
  getStatistics().createHistogramWithAxes(
      new TH1D("lead_thr1_thr3_diff", "Time Difference between leading Signal Channels THR1 and THR3 in found signals",
                                                    2.0*fSigChEdgeMaxTime/100.0, -fSigChEdgeMaxTime+125.0, fSigChEdgeMaxTime-125.0),
                                                    "time diff [ps]", "Number of Signal Channels Pairs");
  getStatistics().createHistogramWithAxes(
      new TH1D("lead_thr1_thr4_diff", "Time Difference between leading Signal Channels THR1 and THR4 in found signals",
                                                    2.0*fSigChEdgeMaxTime/100.0, -fSigChEdgeMaxTime+125.0, fSigChEdgeMaxTime-125.0),
                                                    "time diff [ps]", "Number of Signal Channels Pairs");
  getStatistics().createHistogramWithAxes(
      new TH1D("lead_trail_thr1_diff", "Time Difference between leading and trailing Signal Channels THR1 in found signals",
                                                    fSigChLeadTrailMaxTime/100.0, -125.0, fSigChLeadTrailMaxTime-125.0),
                                                    "time diff [ps]", "Number of Signal Channels Pairs");
  getStatistics().createHistogramWithAxes(
      new TH1D("lead_trail_thr2_diff", "Time Difference between leading and trailing Signal Channels THR2 in found signals",
                                                    fSigChLeadTrailMaxTime/100.0, -125.0, fSigChLeadTrailMaxTime-125.0),
                                                    "time diff [ps]", "Number of Signal Channels Pairs");
  getStatistics().createHistogramWithAxes(
      new TH1D("lead_trail_thr3_diff", "Time Difference between leading and trailing Signal Channels THR3 in found signals",
                                                    fSigChLeadTrailMaxTime/100.0, -125.0, fSigChLeadTrailMaxTime-125.0),
                                                    "time diff [ps]", "Number of Signal Channels Pairs");
  getStatistics().createHistogramWithAxes(
      new TH1D("lead_trail_thr4_diff", "Time Difference between leading and trailing Signal Channels THR4 in found signals",
                                                    fSigChLeadTrailMaxTime/100.0, -125.0, fSigChLeadTrailMaxTime-125.0),
                                                    "time diff [ps]", "Number of Signal Channels Pairs");

  getStatistics().createHistogramWithAxes(
    new TH1D("good_v_bad_raw_sigs", "Number of good and corrupted signals created",
                                            3, 0.5, 3.5), "Quality", "Number of Raw Signals");
  binLabels.clear();
  binLabels.push_back(std::make_pair(1,"GOOD"));
  binLabels.push_back(std::make_pair(2,"CORRUPTED"));
  binLabels.push_back(std::make_pair(3,"UNKNOWN"));
  getStatistics().setHistogramBinLabel("good_v_bad_raw_sigs",
                                       getStatistics().AxisLabel::kXaxis, binLabels);
}
