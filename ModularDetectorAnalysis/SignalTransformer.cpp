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
#include "CalibrationTools.h"

#include <boost/property_tree/json_parser.hpp>

using namespace jpet_options_tools;

SignalTransformer::SignalTransformer(const char* name): JPetUserTask(name) {}

SignalTransformer::~SignalTransformer() {}

bool SignalTransformer::init()
{
  INFO("Signal Transformer started: Raw to Matrix Signal");
  fOutputEvents = new JPetTimeWindow("JPetMatrixSignal");

  // Getting bools for saving control and calibration histograms
  if (isOptionSet(fParams.getOptions(), kSaveControlHistosParamKey)) {
    fSaveControlHistos = getOptionAsBool(fParams.getOptions(), kSaveControlHistosParamKey);
  }
  if (isOptionSet(fParams.getOptions(), kSaveCalibHistosParamKey)) {
    fSaveCalibHistos = getOptionAsBool(fParams.getOptions(), kSaveCalibHistosParamKey);
  }

  // Reading file with Side B signals correction to property tree
  if (isOptionSet(fParams.getOptions(), kConstantsFileParamKey)) {
    boost::property_tree::read_json(getOptionAsString(fParams.getOptions(), kConstantsFileParamKey), fConstansTree);
  }

  // Signal merging time parameter
  if (isOptionSet(fParams.getOptions(), kMergeSignalsTimeParamKey)) {
    fMergingTime = getOptionAsDouble(fParams.getOptions(), kMergeSignalsTimeParamKey);
  } else {
    WARNING(Form(
      "No value of the %s parameter provided by the user. Using default value of %lf.",
      kMergeSignalsTimeParamKey.c_str(), fMergingTime
    ));
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
    auto mergedSignals = SignalTransformerTools::mergeSignalsAllSiPMs(rawSigMtxMap, fMergingTime, fConstansTree);

    // Saving method invocation
    if(mergedSignals.size()>0){
      saveMatrixSignals(mergedSignals);
    }

  } else { return false; }
  return true;
}

bool SignalTransformer::terminate()
{
  if(fSaveCalibHistos && isOptionSet(fParams.getOptions(), kCalibBankFileParamKey)){
    INFO(Form(
      "Signal Transformer - adding offsets histograms to calibration bank %s",
      getOptionAsString(fParams.getOptions(), kCalibBankFileParamKey).c_str()
    ));

    std::vector<TH1F*> histograms;
    for(auto pm_e : getParamBank().getPMs()) {
      TH1F* histo = dynamic_cast<TH1F*>(getStatistics().getHisto1D(Form("offset_%d", pm_e.second->getID()))->Clone());
      histograms.push_back(histo);
    }
    CalibrationTools::addHistograms(histograms, getOptionAsString(fParams.getOptions(), kCalibBankFileParamKey));
  }
  INFO("Signal Transformer finished");
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
      auto scinID = mtxSig.getPM().getScin().getID();
      getStatistics().getHisto1D("mtxsig_multi")->Fill(mtxSig.getRawSignals().size());
      if(mtxSig.getPM().getSide()==JPetPM::SideA){
        getStatistics().getHisto1D("mtxsig_per_scin_sideA")->Fill(scinID);
      } else if(mtxSig.getPM().getSide()==JPetPM::SideB){
        getStatistics().getHisto1D("mtxsig_per_scin_sideB")->Fill(scinID);
      }
    }

    if(fSaveCalibHistos){
      if(mtxSig.getRawSignals().size() > 1 && mtxSig.getRawSignals().find(1)!=mtxSig.getRawSignals().end()) {
        auto t1 = SignalTransformerTools::getRawSigBaseTime(mtxSig.getRawSignals().at(1));
        if(mtxSig.getRawSignals().find(2)!=mtxSig.getRawSignals().end()){
          auto pm_2 = mtxSig.getRawSignals().at(2).getPM().getID();
          auto t2 = SignalTransformerTools::getRawSigBaseTime(mtxSig.getRawSignals().at(2));
          getStatistics().getHisto1D(Form("offset_%d", pm_2))->Fill(t2-t1);
        }
        if(mtxSig.getRawSignals().find(3)!=mtxSig.getRawSignals().end()){
          auto pm_3 = mtxSig.getRawSignals().at(3).getPM().getID();
          auto t3 = SignalTransformerTools::getRawSigBaseTime(mtxSig.getRawSignals().at(3));
          getStatistics().getHisto1D(Form("offset_%d", pm_3))->Fill(t3-t1);
        }
        if(mtxSig.getRawSignals().find(4)!=mtxSig.getRawSignals().end()){
          auto pm_4 = mtxSig.getRawSignals().at(4).getPM().getID();
          auto t4 = SignalTransformerTools::getRawSigBaseTime(mtxSig.getRawSignals().at(4));
          getStatistics().getHisto1D(Form("offset_%d", pm_4))->Fill(t4-t1);
        }
      }
    }
  }
}

void SignalTransformer::initialiseHistograms()
{
  auto minScinID = getParamBank().getScins().begin()->first;
  auto maxScinID = getParamBank().getScins().rbegin()->first;

  // MatrixSignal multiplicity
  getStatistics().createHistogram(new TH1F(
    "mtxsig_multi", "Multiplicity of matched MatrixSignals", 5, 0.5, 5.5
  ));
  getStatistics().getHisto1D("mtxsig_multi")->GetXaxis()->SetTitle("Number of Raw Signals in Matrix Signal");
  getStatistics().getHisto1D("mtxsig_multi")->GetYaxis()->SetTitle("Number of Matrix Signals");

  getStatistics().createHistogram(new TH1F(
    "mtxsig_tslot", "Number of Matrix Signals in Time Window", 100, 0.5, 100.5
  ));
  getStatistics().getHisto1D("mtxsig_tslot")->GetXaxis()->SetTitle("Number of Matrix Signal in Time Window");
  getStatistics().getHisto1D("mtxsig_tslot")->GetYaxis()->SetTitle("Number of Time Windows");

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

  // SiPM offsets if needed
  if(fSaveCalibHistos){
    for(auto pm_e : getParamBank().getPMs()){
      auto pmID = pm_e.second->getID();

      getStatistics().createHistogram(new TH1F(
        Form("offset_%d", pmID), Form("Offset for SiPM %d", pmID),
        200, -1.1*fMergingTime, 1.1*fMergingTime
      ));
      getStatistics().getHisto1D(Form("offset_%d", pmID))->GetXaxis()->SetTitle("Time difference [ps]");
      getStatistics().getHisto1D(Form("offset_%d", pmID))->GetYaxis()->SetTitle("Number of Raw Signal pairs");
    }
  }
}
