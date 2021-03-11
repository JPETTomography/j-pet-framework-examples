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

#include <boost/property_tree/json_parser.hpp>

#include <JPetOptionsTools/JPetOptionsTools.h>
#include <JPetTimeWindow/JPetTimeWindow.h>
#include <JPetWriter/JPetWriter.h>

#include <TRandom.h>

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
  fOutputEvents = new JPetTimeWindow("JPetRawSignal");

  // Reading values from the user options if available
  // Time window parameter for leading edge
  if (isOptionSet(fParams.getOptions(), kEdgeMaxTimeParamKey))
  {
    fSigChEdgeMaxTime = getOptionAsDouble(fParams.getOptions(), kEdgeMaxTimeParamKey);
  }
  else
  {
    WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.", kEdgeMaxTimeParamKey.c_str(), fSigChEdgeMaxTime));
  }

  // Time window parameter for leading-trailing comparison
  if (isOptionSet(fParams.getOptions(), kLeadTrailMaxTimeParamKey))
  {
    fSigChLeadTrailMaxTime = getOptionAsDouble(fParams.getOptions(), kLeadTrailMaxTimeParamKey);
  }
  else
  {
    WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.", kLeadTrailMaxTimeParamKey.c_str(),
                 fSigChLeadTrailMaxTime));
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
    auto& sigChByPM = SignalFinderTools::getSigChByPM(timeWindow);
    // Building signals
    auto allSignals = SignalFinderTools::buildAllSignals(sigChByPM, fSigChEdgeMaxTime, fSigChLeadTrailMaxTime, kNumOfThresholds, getStatistics(),
                                                         fSaveControlHistos, fConstansTree);
    // Saving method invocation
    saveRawSignals(allSignals);
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
 * Saving Raw Signals that have leading-trailing pairs,
 * otherwise filling histogram with incomplete signals
 */
void SignalFinder::saveRawSignals(const vector<JPetRawSignal>& rawSigVec)
{
  if (fSaveControlHistos)
  {
    getStatistics().getHisto1D("rawsig_tslot")->Fill(rawSigVec.size());
  }

  for (auto& rawSig : rawSigVec)
  {
    auto leads = rawSig.getPoints(JPetSigCh::Leading, JPetRawSignal::ByThrValue);
    auto trails = rawSig.getPoints(JPetSigCh::Trailing, JPetRawSignal::ByThrValue);

    // Saving only signals with lead-trail pair on threshold
    if (leads.size() == trails.size())
    {
      fOutputEvents->add<JPetRawSignal>(rawSig);

      if (fSaveControlHistos && gRandom->Uniform() < fScalingFactor)
      {
        auto pmID = rawSig.getPM().getID();
        getStatistics().getHisto1D("rawsig_pm")->Fill(pmID);
        getStatistics().getHisto1D("rawsig_multi")->Fill(leads.size() + trails.size());
      }
    }
  }
}

void SignalFinder::initialiseHistograms()
{

  auto minPMID = getParamBank().getPMs().begin()->first;
  auto maxPMID = getParamBank().getPMs().rbegin()->first;

  // Unused objects stats
  getStatistics().createHistogram(new TH1F("unused_sigch_thr", "Unused Signal Channels per THR (downscaled)", 5, 0.5, 5.5));
  getStatistics().getHisto1D("unused_sigch_thr")->GetXaxis()->SetBinLabel(1, "THR 1 Lead");
  getStatistics().getHisto1D("unused_sigch_thr")->GetXaxis()->SetBinLabel(2, "THR 1 Trail");
  getStatistics().getHisto1D("unused_sigch_thr")->GetXaxis()->SetBinLabel(3, "THR 2 Lead");
  getStatistics().getHisto1D("unused_sigch_thr")->GetXaxis()->SetBinLabel(4, "THR 2 Trail");
  getStatistics().getHisto1D("unused_sigch_thr")->GetXaxis()->SetBinLabel(5, "  ");
  getStatistics().getHisto1D("unused_sigch_thr")->GetYaxis()->SetTitle("Number of SigChs");

  getStatistics().createHistogram(
      new TH1F("unused_sigch_pm", "Unused Signal Channels per SiPM", maxPMID - minPMID + 1, minPMID - 0.5, maxPMID + 0.5));
  getStatistics().getHisto1D("unused_sigch_pm")->GetXaxis()->SetTitle("SiPM ID");
  getStatistics().getHisto1D("unused_sigch_pm")->GetYaxis()->SetTitle("Number of Signal Channels");

  // Occupancies and multiplicities
  getStatistics().createHistogram(new TH1F("rawsig_pm", "Raw Signals per SiPM", maxPMID - minPMID + 1, minPMID - 0.5, maxPMID + 0.5));
  getStatistics().getHisto1D("rawsig_pm")->GetXaxis()->SetTitle("SiPM ID");
  getStatistics().getHisto1D("rawsig_pm")->GetYaxis()->SetTitle("Number of Raw Signals");

  getStatistics().createHistogram(new TH1F("rawsig_multi", "Raw Signal Multiplicity", 6, 0.5, 6.5));
  getStatistics().getHisto1D("rawsig_multi")->GetXaxis()->SetTitle("Total number of SigChs in RawSig");
  getStatistics().getHisto1D("rawsig_multi")->GetYaxis()->SetTitle("Number of Signal Channels");

  getStatistics().createHistogram(new TH1F("rawsig_tslot", "Number of Raw Signals in Time Window", 200, 0.5, 400.5));
  getStatistics().getHisto1D("rawsig_tslot")->GetXaxis()->SetTitle("Number of Raw Signal in Time Window");
  getStatistics().getHisto1D("rawsig_tslot")->GetYaxis()->SetTitle("Number of Time Windows");

  for (int thr = 0; thr < kNumOfThresholds; ++thr)
  {
    getStatistics().createHistogram(new TH2F(Form("tot_sipm_id_thr%d", thr + 1),
                                             Form("SiPM Signal Time over Threshold per SiPM ID for THR %d", thr + 1), maxPMID - minPMID + 1,
                                             minPMID - 0.5, maxPMID + 0.5, 200, 0.0, fSigChLeadTrailMaxTime));
    getStatistics().getHisto2D(Form("tot_sipm_id_thr%d", thr + 1))->GetXaxis()->SetTitle("SiPM ID");
    getStatistics().getHisto2D(Form("tot_sipm_id_thr%d", thr + 1))->GetYaxis()->SetTitle("TOT [ps]");
  }

  getStatistics().createHistogram(new TH2F("tot_sipm_id_sum", "SiPM Signal Time over Threshold per SiPM ID", maxPMID - minPMID + 1, minPMID - 0.5,
                                           maxPMID + 0.5, 200, 0.0, kNumOfThresholds * fSigChLeadTrailMaxTime));
  getStatistics().getHisto2D("tot_sipm_id_sum")->GetXaxis()->SetTitle("SiPM ID");
  getStatistics().getHisto2D("tot_sipm_id_sum")->GetYaxis()->SetTitle("TOT [ps]");
}
