/**
 *  @copyright Copyright 2019 The J-PET Framework Authors. All rights reserved.
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
 *  @file SignalTransformer.cpp
 */

#include "SignalTransformerTools.h"
#include "JPetWriter/JPetWriter.h"
#include "SignalTransformer.h"

using namespace jpet_options_tools;

SignalTransformer::SignalTransformer(const char* name): JPetUserTask(name) {}

SignalTransformer::~SignalTransformer() {}

bool SignalTransformer::init()
{
  INFO("Signal transforming started: Raw to Matrix Signal");
  fOutputEvents = new JPetTimeWindow("JPetMatrixSignal");
  // Getting bool for saving histograms
  if (isOptionSet(fParams.getOptions(), kSaveControlHistosParamKey)) {
    fSaveControlHistos = getOptionAsBool(fParams.getOptions(), kSaveControlHistosParamKey);
  }

  // Signal merging time parameter
  if (isOptionSet(fParams.getOptions(), kMergeSignalsTimeParamKey)) {
    fMergingTime = getOptionAsFloat(fParams.getOptions(), kMergeSignalsTimeParamKey);
  } else {
    WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.",
      kMergeSignalsTimeParamKey.c_str(), fMergingTime)
    );
  }

  // Control histograms
  if(fSaveControlHistos) { initialiseHistograms(); }
  return true;
}

bool SignalTransformer::exec()
{
  // Getting the data from event in an apropriate format
  if(auto timeWindow = dynamic_cast<const JPetTimeWindow* const>(fEvent)) {
    // Distribute Raw Signals per their Matrices
    auto& rawSigMtxMap = SignalTransformerTools::getRawSigMtxMap(timeWindow, getStatistics(), fSaveControlHistos);

    // Merging max. 4 Raw Signals into a MatrixSignal
    auto mergedSignals = SignalTransformerTools::mergeSignalsAllSiPMs(
      rawSigMtxMap, 11.0, getStatistics(), true
    );

    // Saving method invocation
    saveMatrixSignals(mergedSignals);
  } else { return false; }
  return true;
}

bool SignalTransformer::terminate()
{
  INFO("Signal transforming finished");
  return true;
}

/**
* Save objects and make some histograms
*/
void SignalTransformer::saveMatrixSignals(std::vector<JPetMatrixSignal>& mtxSigVec)
{
  for (auto& mtxSig : mtxSigVec) {
    fOutputEvents->add<JPetMatrixSignal>(mtxSig);
    if(fSaveControlHistos){
      getStatistics().getHisto1D("mtxsig_multi")->Fill(mtxSig.getRawSignals().size());
      if(mtxSig.getMtxSide()==JPetPM::SideA){
        getStatistics().getHisto1D("mtxsig_per_scin_sideA")->Fill(mtxSig.getScinID());
      } else if(mtxSig.getMtxSide()==JPetPM::SideB){
        getStatistics().getHisto1D("mtxsig_per_scin_sideB")->Fill(mtxSig.getScinID());
      }
    }
  }
}

void SignalTransformer::initialiseHistograms()
{
  // Preprocedure histograms
  getStatistics().createHistogram(new TH1F(
    "rawsig_per_pm",
    "Input Raw Signals per SiPM", 111, 399.5, 510.5
  ));
  getStatistics().getHisto1D("rawsig_per_pm")->GetXaxis()->SetTitle("SiPM ID");
  getStatistics().getHisto1D("rawsig_per_pm")->GetYaxis()->SetTitle("Number of Raw Signals Signals");

  // Preprocedure histograms
  getStatistics().createHistogram(new TH1F(
    "rawsig_per_scin",
    "Input Raw Signals per scintillator", 13, 200.5, 213.5
  ));
  getStatistics().getHisto1D("rawsig_per_scin")->GetXaxis()->SetTitle("Scin ID");
  getStatistics().getHisto1D("rawsig_per_scin")->GetYaxis()->SetTitle("Number of Raw Signals Signals");

  // MatrixSignal multiplicity
  getStatistics().createHistogram(new TH1F(
    "mtxsig_multi",
    "Multiplicity of matched MatrixSignals", 4, 0.5, 4.5
  ));
  getStatistics().getHisto1D("mtxsig_multi")->GetXaxis()->SetTitle("Number of Raw Signals in Matrix Signal");
  getStatistics().getHisto1D("mtxsig_multi")->GetYaxis()->SetTitle("Number of Matrix Signals");

  // Mtx Signals per scintillator
  getStatistics().createHistogram(new TH1F(
    "mtxsig_per_scin_sideA",
    "Number of MatrixSignals per scintillator side A", 13, 200.5, 213.5
  ));
  getStatistics().getHisto1D("mtxsig_per_scin_sideA")->GetXaxis()->SetTitle("Scin ID");
  getStatistics().getHisto1D("mtxsig_per_scin_sideA")->GetYaxis()->SetTitle("Number of Matrix Signals");

  getStatistics().createHistogram(new TH1F(
    "mtxsig_per_scin_sideB",
    "Number of MatrixSignals per scintillator side B", 13, 200.5, 213.5
  ));
  getStatistics().getHisto1D("mtxsig_per_scin_sideB")->GetXaxis()->SetTitle("Scin ID");
  getStatistics().getHisto1D("mtxsig_per_scin_sideB")->GetYaxis()->SetTitle("Number of Matrix Signals");

  getStatistics().createHistogram(new TH1F(
    "mtxsig_timediff",
    "Time difference of Raw Signals in created Matrix Signal", 200, 0.0, fMergingTime
  ));
  getStatistics().getHisto1D("mtxsig_timediff")->GetXaxis()->SetTitle("Time difference [ps]");
  getStatistics().getHisto1D("mtxsig_timediff")->GetYaxis()->SetTitle("Number of Raw Signal pairs");


  // Unused histograms
  getStatistics().createHistogram(new TH1F(
    "unused_rawsig_per_scin_sideA",
    "Number of unused Raw Signals per scintillator side A", 13, 200.5, 213.5
  ));
  getStatistics().getHisto1D("unused_rawsig_per_scin_sideA")->GetXaxis()->SetTitle("Scin ID");
  getStatistics().getHisto1D("unused_rawsig_per_scin_sideA")->GetYaxis()->SetTitle("Number of Matrix Signals");

  getStatistics().createHistogram(new TH1F(
    "unused_rawsig_per_scin_sideB",
    "Number of unused Raw Signals per scintillator side B", 13, 200.5, 213.5
  ));
  getStatistics().getHisto1D("unused_rawsig_per_scin_sideB")->GetXaxis()->SetTitle("Scin ID");
  getStatistics().getHisto1D("unused_rawsig_per_scin_sideB")->GetYaxis()->SetTitle("Number of Matrix Signals");

  getStatistics().createHistogram(new TH1F(
    "unused_rawsig_per_pm",
    "Number of unused Raw Signals per SiPM",
    111, 399.5, 510.5
  ));
  getStatistics().getHisto1D("unused_rawsig_per_pm")->GetXaxis()->SetTitle("SiPM ID");
  getStatistics().getHisto1D("unused_rawsig_per_pm")->GetYaxis()->SetTitle("Number of Raw Signals");

}
