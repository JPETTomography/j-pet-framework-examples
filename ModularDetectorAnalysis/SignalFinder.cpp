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
 *  @file SignalFinder.cpp
 */

#include "SignalFinder.h"
#include "SignalFinderTools.h"
#include <JPetOptionsTools/JPetOptionsTools.h>
#include <JPetTimeWindow/JPetTimeWindow.h>
#include <JPetWriter/JPetWriter.h>
#include <TRandom.h>
#include <boost/property_tree/json_parser.hpp>
#include <string>
#include <utility>
#include <vector>

using namespace std;
using namespace jpet_options_tools;
namespace pt = boost::property_tree;

SignalFinder::SignalFinder(const char* name) : JPetUserTask(name) {}

SignalFinder::~SignalFinder() {}

bool SignalFinder::init()
{
  INFO("Signal finding started.");
  fOutputEvents = new JPetTimeWindow("JPetPMSignal");

  // Reading values from the user options if available
  // Time window parameter for leading edge
  if (isOptionSet(fParams.getOptions(), kEdgeMaxTimeParamKey))
  {
    fChSigEdgeMaxTime = getOptionAsDouble(fParams.getOptions(), kEdgeMaxTimeParamKey);
  }
  else
  {
    WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.", kEdgeMaxTimeParamKey.c_str(), fChSigEdgeMaxTime));
  }

  // Time window parameter for leading-trailing comparison
  if (isOptionSet(fParams.getOptions(), kLeadTrailMaxTimeParamKey))
  {
    fChSigLeadTrailMaxTime = getOptionAsDouble(fParams.getOptions(), kLeadTrailMaxTimeParamKey);
  }
  else
  {
    WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.", kLeadTrailMaxTimeParamKey.c_str(),
                 fChSigLeadTrailMaxTime));
  }

  // For plotting TOT histograms
  if (isOptionSet(fParams.getOptions(), kTOTHistoUpperLimitParamKey))
  {
    fTOTHistoUpperLimit = getOptionAsDouble(fParams.getOptions(), kTOTHistoUpperLimitParamKey);
  }

  if (isOptionSet(fParams.getOptions(), kTOTCalculationTypeParamKey))
  {
    switch ((getOptionAsString(fParams.getOptions(), kTOTCalculationTypeParamKey)) {
    case "simple":
      fTOTCalcType = kSimplified;
      break;
    case "rectangular":
      fTOTCalcType = kThresholdRectangular;
      break;
    case "trapeze":
      fTOTCalcType = kThresholdTrapeze;
      break;
    default:
      WARNING("Unrecognized name for method of calculating TOT provided: use simple, rectangular of trapeze.");
      break;
    }
  }

  // Get bool for using corrupted Signal Channels
  if (isOptionSet(fParams.getOptions(), kUseCorruptedChSigParamKey))
  {
    fUseCorruptedChSig = getOptionAsBool(fParams.getOptions(), kUseCorruptedChSigParamKey);
    if (fUseCorruptedChSig)
    {
      WARNING("Signal Finder is using Corrupted Channel Signals, as set by the user");
    }
    else
    {
      WARNING("Signal Finder is NOT using Corrupted Channel Signals, as set by the user");
    }
  }
  else
  {
    WARNING("Signal Finder is not using Corrupted Channel Signals (default option)");
  }

  // Getting bool for saving histograms
  if (isOptionSet(fParams.getOptions(), kSaveControlHistosParamKey))
  {
    fSaveControlHistos = getOptionAsBool(fParams.getOptions(), kSaveControlHistosParamKey);
  }

  // Reading file with offsets to property tree - SiPM calibration per matrix
  if (isOptionSet(fParams.getOptions(), kConstantsFileParamKey))
  {
    pt::read_json(getOptionAsString(fParams.getOptions(), kConstantsFileParamKey), fConstansTree);
  }

  // Creating control histograms
  if (fSaveControlHistos)
  {
    initialiseHistograms();
  }
  return true;
}

bool SignalFinder::exec()
{
  // Getting the data from event in an apropriate format
  if (auto timeWindow = dynamic_cast<const JPetTimeWindow* const>(fEvent))
  {
    // Distribute signal channels by PM IDs
    auto& chSigsByPM = SignalFinderTools::getChannelSignalsByPM(timeWindow, fUseCorruptedChannelSignals);
    // Building signals
    auto allSignals = SignalFinderTools::buildAllSignals(chSigsByPM, fChannelSignalsEdgeMaxTime, fChannelSignalsLeadTrailMaxTime, kNumOfThresholds,
                                                         getStatistics(), fSaveControlHistos, fConstansTree);
    // Saving method invocation
    savePMSignals(allSignals);
  }
  else
  {
    return false;
  }
  return true;
}

bool SignalFinder::terminate()
{
  INFO("Signal finding ended.");
  return true;
}

/**
 * Saving PM Signals that have leading-trailing pairs,
 * otherwise filling histogram with incomplete signals
 */
void SignalFinder::savePMSignals(const vector<JPetPMSignal>& pmSigVec)
{
  if (pmSigVec.size() > 0 && fSaveControlHistos)
  {
    getStatistics().fillHistogram("pmsig_tslot", pmSigVec.size());
  }

  for (auto& pmSig : pmSigVec)
  {
    auto leads = pmSig.getPoints(JPetChannelSignal::Leading, JPetPMSignal::ByThrValue);
    auto trails = pmSig.getPoints(JPetChannelSignal::Trailing, JPetPMSignal::ByThrValue);
    if (fSaveControlHistos && gRandom->Uniform() < fScalingFactor)
    {
      getStatistics().fillHistogram("pmsig_multi", leads.size() + trails.size());
    }

    // Saving only signals with lead-trail pair on threshold
    if (leads.size() == trails.size())
    {
      fOutputEvents->add<JPetPMSignal>(pmSig);
      if (fSaveControlHistos && gRandom->Uniform() < fScalingFactor)
      {
        auto pmID = pmSig.getPM().getID();
        getStatistics().fillHistogram("pmsig_sipm", pmID);
      }
    }
  }
}

void SignalFinder::initialiseHistograms()
{
  auto minPMID = getParamBank().getPMs().begin()->first;
  auto maxPMID = getParamBank().getPMs().rbegin()->first;

  vector<pair<unsigned, string>> binLabels = {make_pair(1, "THR 1 Lead"), make_pair(2, "THR 1 Trail"), make_pair(3, "THR 2 Lead"),
                                              make_pair(4, "THR 2 Trail"), make_pair(5, " ")};
  getStatistics().createHistogramWithAxes(new TH1D("unused_chsig_thr", "Unused Channel Signals per THR (downscaled)", 5, 0.5, 5.5), " ",
                                          "Number of Channel Signals");
  getStatistics().setHistogramBinLabel("unused_chsig_thr", getStatistics().AxisLabel::kXaxis, binLabels);

  getStatistics().createHistogramWithAxes(
      new TH1D("unused_chsig_sipm", "Unused Signal Channels per SiPM", maxPMID - minPMID + 1, minPMID - 0.5, maxPMID + 0.5), "SiPM ID",
      "Number of Channel Signals");

  // Occupancies and multiplicities
  getStatistics().createHistogramWithAxes(new TH1D("pmsig_sipm", "PM Signals per SiPM", maxPMID - minPMID + 1, minPMID - 0.5, maxPMID + 0.5),
                                          "SiPM ID", "Number of PM Signals");

  getStatistics().createHistogramWithAxes(new TH1D("pmsig_multi", "PM Signal Multiplicity", 6, 0.5, 6.5), "Total number of ChSigs in PMSig",
                                          "Number of Signal Channels");

  getStatistics().createHistogramWithAxes(new TH1D("pmsig_tslot", "Number of PM Signals in Time Window", 100, 0.5, 100.5),
                                          "Number of PM Signal in Time Window", "Number of Time Windows");

  getStatistics().createHistogramWithAxes(new TH1D("pmsig_tslot", "Number of PM Signals in Time Window", 100, 0.5, 100.5),
                                          "Number of PM Signal in Time Window", "Number of Time Windows");

  // TOT of signals
  getStatistics().createHistogramWithAxes(new TH2D("tot_sipm_id", "SiPM Signal Time over Threshold per SiPM ID", maxPMID - minPMID + 1, minPMID - 0.5,
                                                   maxPMID + 0.5, 200, 0.0, fTOTHistoUpperLimit),
                                          "SiPM ID", "TOT [ps]");

  getStatistics().createHistogramWithAxes(new TH2D("tot_sipm_id_norm", "SiPM Signal Time over Threshold per SiPM ID normalised",
                                                   maxPMID - minPMID + 1, minPMID - 0.5, maxPMID + 0.5, 200, 0.0,
                                                   double totUppLimit = 50.0 * fChSigLeadTrailMaxTime;),
                                          "SiPM ID", "TOT [ps]");
}
