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

    // Distribute Raw Signals per Matrices
    auto rawSigMtxMap = SignalTransformerTools::getRawSigMtxMap(timeWindow);

    // Merging max. 4 Raw Signals into a MatrixSignal
    auto mergedSignals = SignalTransformerTools::mergeSignalsAllSiPMs(
      rawSigMtxMap, 11.0, getStatistics(), true
    );

    // Saving method invocation
    if(mergedSignals.size()>0){
      saveMatrixSignals(mergedSignals);
    }

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
void SignalTransformer::saveMatrixSignals(const std::vector<JPetMatrixSignal>& mtxSigVec)
{
  if(fSaveControlHistos){
    getStatistics().getHisto1D("mtxsig_tslot")->Fill(mtxSigVec.size());
  }
  for (auto& mtxSig : mtxSigVec) {
    fOutputEvents->add<JPetMatrixSignal>(mtxSig);
    if(fSaveControlHistos){
      getStatistics().getHisto1D("mtxsig_multi")->Fill(mtxSig.getRawSignals().size());
      if(mtxSig.getPM().getSide()==JPetPM::SideA){
        getStatistics().getHisto1D("mtxsig_per_scin_sideA")->Fill(mtxSig.getPM().getScin().getID());
      } else if(mtxSig.getPM().getSide()==JPetPM::SideB){
        getStatistics().getHisto1D("mtxsig_per_scin_sideB")->Fill(mtxSig.getPM().getScin().getID());
      }
    }
  }
}

void SignalTransformer::initialiseHistograms()
{
  // MatrixSignal multiplicity
  getStatistics().createHistogram(new TH1F(
    "mtxsig_multi", "Multiplicity of matched MatrixSignals", 4, 0.5, 4.5
  ));
  getStatistics().getHisto1D("mtxsig_multi")->GetXaxis()->SetTitle("Number of Raw Signals in Matrix Signal");
  getStatistics().getHisto1D("mtxsig_multi")->GetYaxis()->SetTitle("Number of Matrix Signals");

  getStatistics().createHistogram(new TH1F(
    "mtxsig_tslot", "Number of Matrix Signals in Time Window", 150, -0.5, 149.5
  ));
  getStatistics().getHisto1D("mtxsig_tslot")->GetXaxis()->SetTitle("Number of Matrix Signal in Time Window");
  getStatistics().getHisto1D("mtxsig_tslot")->GetYaxis()->SetTitle("Number of Time Windows");

  auto minScinID = getParamBank().getScins().begin()->first;
  auto maxScinID = getParamBank().getScins().rbegin()->first;

  // Time differences of consecutive RawSigs per SiPMs pair - all combinations
  for(int scinID=201; scinID<227; scinID++) {
    // if(scinID > 226) { break; }
    for(int i=1; i<=4; i++){
      for(int j=1; j<=4; j++){
        if(i == j) { continue; }
        getStatistics().createHistogram(new TH1F(
          Form("tdiff_%d_A_%d_%d", scinID, i, j),
          Form("Time diff of consecutive signals on scin %d side A matrix positions %d vs %d", scinID, i, j),
          200, 0.0, 100000.0
        ));
        getStatistics().getHisto1D(Form("tdiff_%d_A_%d_%d", scinID, i, j))
        ->GetXaxis()->SetTitle("Time difference [ps]");
        getStatistics().getHisto1D(Form("tdiff_%d_A_%d_%d", scinID, i, j))
        ->GetYaxis()->SetTitle("Number of Raw Signal pairs");

        getStatistics().createHistogram(new TH1F(
          Form("tdiff_%d_B_%d_%d", scinID, i, j),
          Form("Time diff of consecutive signals on scin %d side B matrix positions %d vs %d", scinID, i, j),
          200, 0.0, 100000.0
        ));
        getStatistics().getHisto1D(Form("tdiff_%d_B_%d_%d", scinID, i, j))
        ->GetXaxis()->SetTitle("Time difference [ps]");
        getStatistics().getHisto1D(Form("tdiff_%d_B_%d_%d", scinID, i, j))
        ->GetYaxis()->SetTitle("Number of Raw Signal pairs");
      }
    }
  }

  // Mtx Signals per scintillator
  getStatistics().createHistogram(new TH1F(
    "mtxsig_per_scin_sideA", "Number of MatrixSignals per scintillator side A",
    maxScinID-minScinID+1, minScinID-0.5, maxScinID+0.5
  ));
  getStatistics().getHisto1D("mtxsig_per_scin_sideA")->GetXaxis()->SetTitle("Scin ID");
  getStatistics().getHisto1D("mtxsig_per_scin_sideA")->GetYaxis()->SetTitle("Number of Matrix Signals");

  getStatistics().createHistogram(new TH1F(
    "mtxsig_per_scin_sideB", "Number of MatrixSignals per scintillator side B",
    maxScinID-minScinID+1, minScinID-0.5, maxScinID+0.5
  ));
  getStatistics().getHisto1D("mtxsig_per_scin_sideB")->GetXaxis()->SetTitle("Scin ID");
  getStatistics().getHisto1D("mtxsig_per_scin_sideB")->GetYaxis()->SetTitle("Number of Matrix Signals");
}
