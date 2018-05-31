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
 *  @file SignalFinder.cpp
 */

using namespace std;

#include <JPetOptionsTools/JPetOptionsTools.h>
#include <JPetWriter/JPetWriter.h>
#include "SignalFinderTools.h"
#include "SignalFinder.h"
#include <string>
#include <vector>
#include <map>

using namespace jpet_options_tools;

SignalFinder::SignalFinder(const char* name): JPetUserTask(name) {}

SignalFinder::~SignalFinder() {}

bool SignalFinder::init()
{
  INFO("Signal finding started.");
  fOutputEvents = new JPetTimeWindow("JPetRawSignal");

  // Reading values from the user options if available
  // Time window parameter for leading edge
  if (isOptionSet(fParams.getOptions(), kEdgeMaxTimeParamKey))
    fSigChEdgeMaxTime = getOptionAsFloat(fParams.getOptions(), kEdgeMaxTimeParamKey);
  else
    WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.",
  kEdgeMaxTimeParamKey.c_str(), fSigChEdgeMaxTime));
  // Time window parameter for leading-trailing comparison
  if (isOptionSet(fParams.getOptions(), kLeadTrailMaxTimeParamKey))
    fSigChLeadTrailMaxTime = getOptionAsFloat(fParams.getOptions(), kLeadTrailMaxTimeParamKey);
  else
    WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.",
  kLeadTrailMaxTimeParamKey.c_str(), fSigChLeadTrailMaxTime));
  // Getting bool for saving histograms
  if (isOptionSet(fParams.getOptions(), kSaveControlHistosParamKey))
    fSaveControlHistos = getOptionAsBool(fParams.getOptions(), kSaveControlHistosParamKey);

  // Creating control histograms
  if(fSaveControlHistos) initialiseHistograms();
  return true;
}

// SignalFinder execution method
bool SignalFinder::exec()
{

  // Getting the data from event in apropriate format
  if(auto timeWindow = dynamic_cast<const JPetTimeWindow* const>(fEvent)) {

  // Mapping method invocation
  map<int, vector<JPetSigCh>> sigChsPMMap = SignalFinderTools::getSigChsPMMapById(timeWindow);

  // Building signals method invocation
  vector<JPetRawSignal> allSignals = SignalFinderTools::buildAllSignals(
    sigChsPMMap, kNumOfThresholds, getStatistics(),
    fSigChEdgeMaxTime, fSigChLeadTrailMaxTime, fSaveControlHistos
  );

  // Saving method invocation
  saveRawSignals(allSignals);

  } else return false;
  return true;
}

// SignalFinder finish method
bool SignalFinder::terminate()
{
  INFO("Signal finding ended.");
  return true;
}

// Saving method
void SignalFinder::saveRawSignals(const vector<JPetRawSignal>& sigChVec)
{
  for (auto & sigCh : sigChVec) fOutputEvents->add<JPetRawSignal>(sigCh);
}

void SignalFinder::initialiseHistograms(){
  getStatistics().createHistogram(new TH1F(
    "remainig_leading_sig_ch_per_thr", "Remainig Leading Signal Channels",
    4, 0.5, 4.5));
  getStatistics().getHisto1D("remainig_leading_sig_ch_per_thr")
    ->GetXaxis()->SetTitle("Threshold Number");
  getStatistics().getHisto1D("remainig_leading_sig_ch_per_thr")
    ->GetYaxis()->SetTitle("Number of Signal Channels");

  getStatistics().createHistogram(new TH1F(
    "remainig_trailing_sig_ch_per_thr", "Remainig Trailing Signal Channels",
     4, 0.5, 4.5));
  getStatistics().getHisto1D("remainig_trailing_sig_ch_per_thr")
    ->GetXaxis()->SetTitle("Threshold Number");
  getStatistics().getHisto1D("remainig_trailing_sig_ch_per_thr")
    ->GetYaxis()->SetTitle("Number of Signal Channels");

  getStatistics().createHistogram(new TH1F(
    "lead_thr1_thr2_diff",
    "Time Difference between leading Signal Channels THR1 and THR2 in found signals",
    200, -fSigChEdgeMaxTime, fSigChEdgeMaxTime));
  getStatistics().getHisto1D("lead_thr1_thr2_diff")
    ->GetXaxis()->SetTitle("time diff [ps]");
  getStatistics().getHisto1D("lead_thr1_thr2_diff")
    ->GetYaxis()->SetTitle("Number of Signal Channels Pairs");

  getStatistics().createHistogram(new TH1F(
    "lead_thr1_thr3_diff",
    "Time Difference between leading Signal Channels THR1 and THR3 in found signals",
    200, -fSigChEdgeMaxTime, fSigChEdgeMaxTime));
  getStatistics().getHisto1D("lead_thr1_thr3_diff")
    ->GetXaxis()->SetTitle("time diff [ps]");
  getStatistics().getHisto1D("lead_thr1_thr3_diff")
    ->GetYaxis()->SetTitle("Number of Signal Channels Pairs");

  getStatistics().createHistogram(new TH1F(
    "lead_thr1_thr4_diff",
    "Time Difference between leading Signal Channels THR1 and THR4 in found signals",
    200, -fSigChEdgeMaxTime, fSigChEdgeMaxTime));
  getStatistics().getHisto1D("lead_thr1_thr4_diff")
    ->GetXaxis()->SetTitle("time diff [ps]");
  getStatistics().getHisto1D("lead_thr1_thr4_diff")
    ->GetYaxis()->SetTitle("Number of Signal Channels Pairs");

  getStatistics().createHistogram(new TH1F(
    "lead_trail_thr1_diff",
    "Time Difference between leading and trailing Signal Channels THR1 in found signals",
    200, 0.0, fSigChLeadTrailMaxTime));
  getStatistics().getHisto1D("lead_trail_thr1_diff")
    ->GetXaxis()->SetTitle("time diff [ps]");
  getStatistics().getHisto1D("lead_trail_thr1_diff")
    ->GetYaxis()->SetTitle("Number of Signal Channels Pairs");

  getStatistics().createHistogram(new TH1F(
    "lead_trail_thr2_diff",
    "Time Difference between leading and trailing Signal Channels THR2 in found signals",
    200, 0.0, fSigChLeadTrailMaxTime));
  getStatistics().getHisto1D("lead_trail_thr2_diff")
    ->GetXaxis()->SetTitle("time diff [ps]");
  getStatistics().getHisto1D("lead_trail_thr2_diff")
    ->GetYaxis()->SetTitle("Number of Signal Channels Pairs");

  getStatistics().createHistogram(new TH1F(
    "lead_trail_thr3_diff",
    "Time Difference between leading and trailing Signal Channels THR3 in found signals",
    200, 0.0, fSigChLeadTrailMaxTime));
  getStatistics().getHisto1D("lead_trail_thr3_diff")
    ->GetXaxis()->SetTitle("time diff [ps]");
  getStatistics().getHisto1D("lead_trail_thr3_diff")
    ->GetYaxis()->SetTitle("Number of Signal Channels Pairs");

  getStatistics().createHistogram(new TH1F(
    "lead_trail_thr4_diff",
    "Time Difference between leading and trailing Signal Channels THR4 in found signals",
    200, 0.0, fSigChLeadTrailMaxTime));
  getStatistics().getHisto1D("lead_trail_thr4_diff")
    ->GetXaxis()->SetTitle("time diff [ps]");
  getStatistics().getHisto1D("lead_trail_thr4_diff")
    ->GetYaxis()->SetTitle("Number of Signal Channels Pairs");
}
