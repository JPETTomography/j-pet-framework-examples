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

#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

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
    fMergingTime = getOptionAsDouble(fParams.getOptions(), kMergeSignalsTimeParamKey);
  } else {
    WARNING(Form(
      "No value of the %s parameter provided by the user. Using default value of %lf.",
      kMergeSignalsTimeParamKey.c_str(), fMergingTime
    ));
  }

  if (isOptionSet(fParams.getOptions(), kMinScinIDParamKey)) {
    fMinScinID = getOptionAsInt(fParams.getOptions(), kMinScinIDParamKey);
  } else {
    fMinScinID = getParamBank().getScins().begin()->first;
  }

  if (isOptionSet(fParams.getOptions(), kMaxScinIDParamKey)) {
    fMaxScinID = getOptionAsInt(fParams.getOptions(), kMaxScinIDParamKey);
  } else {
    fMaxScinID = getParamBank().getScins().rbegin()->first;
  }

  if (isOptionSet(fParams.getOptions(), kMinPMIDParamKey)) {
    fMinPMID = getOptionAsInt(fParams.getOptions(), kMinPMIDParamKey);
  } else {
    fMinPMID = getParamBank().getPMs().begin()->first;
  }

  if (isOptionSet(fParams.getOptions(), kMaxPMIDParamKey)) {
    fMaxPMID = getOptionAsInt(fParams.getOptions(), kMaxPMIDParamKey);
  } else {
    fMaxPMID = getParamBank().getPMs().rbegin()->first;
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
      rawSigMtxMap, fMergingTime, getStatistics(), true
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
  if (isOptionSet(fParams.getOptions(), kOffestsFileParamKey) && fSaveControlHistos) {
    INFO("Signal transforming - printing out offsets for SiPMs in matrices");
    fOffsetsFile = getOptionAsString(fParams.getOptions(), kOffestsFileParamKey);

    namespace pt = boost::property_tree;
    using namespace std;

    pt::ptree root;
    pt::ptree sipm_node;

    for(int pmID=fMinPMID; pmID<=fMaxPMID; pmID++){
      auto mean = getStatistics().getHisto1D(Form("offset_sipm_%d", pmID))->GetMean();
      sipm_node.put(to_string(pmID), mean);
    }

    root.add_child("sipm_offsets", sipm_node);
    pt::write_json(fOffsetsFile, root);
  }

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
      auto scinID = mtxSig.getPM().getScin().getID();
      if(scinID<fMinScinID || scinID>fMaxScinID) { continue; }
      getStatistics().getHisto1D("mtxsig_multi")->Fill(mtxSig.getRawSignals().size());
      if(mtxSig.getPM().getSide()==JPetPM::SideA){
        getStatistics().getHisto1D("mtxsig_per_scin_sideA")->Fill(scinID);
      } else if(mtxSig.getPM().getSide()==JPetPM::SideB){
        getStatistics().getHisto1D("mtxsig_per_scin_sideB")->Fill(scinID);
      }
      auto rawSigVec = mtxSig.getRawSignals();
      if(rawSigVec.size() == 4) {
        auto side = rawSigVec.at(1).getPM().getSide();
        auto t1 = SignalTransformerTools::getRawSigBaseTime(rawSigVec.at(1));
        auto t2 = SignalTransformerTools::getRawSigBaseTime(rawSigVec.at(2));
        auto t3 = SignalTransformerTools::getRawSigBaseTime(rawSigVec.at(3));
        auto t4 = SignalTransformerTools::getRawSigBaseTime(rawSigVec.at(4));

        if(rawSigVec.at(2).getPM().getID()>fMinPMID && rawSigVec.at(2).getPM().getID()<fMaxPMID){
          getStatistics().getHisto1D(Form("offset_sipm_%d", rawSigVec.at(2).getPM().getID()))->Fill(t2-t1);
        }
        if(rawSigVec.at(3).getPM().getID()>fMinPMID && rawSigVec.at(3).getPM().getID()<fMaxPMID){
          getStatistics().getHisto1D(Form("offset_sipm_%d", rawSigVec.at(3).getPM().getID()))->Fill(t3-t1);
        }
        if(rawSigVec.at(4).getPM().getID()>fMinPMID && rawSigVec.at(4).getPM().getID()<fMaxPMID){
          getStatistics().getHisto1D(Form("offset_sipm_%d", rawSigVec.at(4).getPM().getID()))->Fill(t4-t1);
        }

        if(side==JPetPM::SideA) {
          getStatistics().getHisto1D(Form("tdiff_%d_A_%d_%d", scinID, 2, 1))->Fill(t2-t1);
          getStatistics().getHisto1D(Form("tdiff_%d_A_%d_%d", scinID, 3, 1))->Fill(t3-t1);
          getStatistics().getHisto1D(Form("tdiff_%d_A_%d_%d", scinID, 4, 1))->Fill(t4-t1);
        } else if(side==JPetPM::SideB){
          getStatistics().getHisto1D(Form("tdiff_%d_B_%d_%d", scinID, 2, 1))->Fill(t2-t1);
          getStatistics().getHisto1D(Form("tdiff_%d_B_%d_%d", scinID, 3, 1))->Fill(t3-t1);
          getStatistics().getHisto1D(Form("tdiff_%d_B_%d_%d", scinID, 4, 1))->Fill(t4-t1);
        }
      }
    }
  }
}

void SignalTransformer::initialiseHistograms()
{
  // MatrixSignal multiplicity
  getStatistics().createHistogram(new TH1F(
    "mtxsig_multi", "Multiplicity of matched MatrixSignals", 5, 0.5, 5.5
  ));
  getStatistics().getHisto1D("mtxsig_multi")->GetXaxis()->SetTitle("Number of Raw Signals in Matrix Signal");
  getStatistics().getHisto1D("mtxsig_multi")->GetYaxis()->SetTitle("Number of Matrix Signals");

  getStatistics().createHistogram(new TH1F(
    "mtxsig_tslot", "Number of Matrix Signals in Time Window", 50, -0.5, 49.5
  ));
  getStatistics().getHisto1D("mtxsig_tslot")->GetXaxis()->SetTitle("Number of Matrix Signal in Time Window");
  getStatistics().getHisto1D("mtxsig_tslot")->GetYaxis()->SetTitle("Number of Time Windows");

  getStatistics().createHistogram(new TH1F(
    "mtxsig_per_scin_sideA", "Number of MatrixSignals per scintillator side A",
    fMaxScinID-fMinScinID+1, fMinScinID-0.5, fMaxScinID+0.5
  ));
  getStatistics().getHisto1D("mtxsig_per_scin_sideA")->GetXaxis()->SetTitle("Scin ID");
  getStatistics().getHisto1D("mtxsig_per_scin_sideA")->GetYaxis()->SetTitle("Number of Matrix Signals");

  getStatistics().createHistogram(new TH1F(
    "mtxsig_per_scin_sideB", "Number of MatrixSignals per scintillator side B",
    fMaxScinID-fMinScinID+1, fMinScinID-0.5, fMaxScinID+0.5
  ));
  getStatistics().getHisto1D("mtxsig_per_scin_sideB")->GetXaxis()->SetTitle("Scin ID");
  getStatistics().getHisto1D("mtxsig_per_scin_sideB")->GetYaxis()->SetTitle("Number of Matrix Signals");

  // Time offsets for SiPMs from the same matrix in respect to SiPM on position 1
  for(int scinID=fMinScinID; scinID<=fMaxScinID; scinID++) {
    for(int i=2; i<=4; i++){
      int j = 1;
      getStatistics().createHistogram(new TH1F(
        Form("tdiff_%d_A_%d_%d", scinID, i, j),
        Form("Time diff of signals on scin %d side A matrix positions %d vs %d", scinID, i, j),
        200, -1.1*fMergingTime, 1.1*fMergingTime
      ));
      getStatistics().getHisto1D(Form("tdiff_%d_A_%d_%d", scinID, i, j))
      ->GetXaxis()->SetTitle("Time difference [ps]");
      getStatistics().getHisto1D(Form("tdiff_%d_A_%d_%d", scinID, i, j))
      ->GetYaxis()->SetTitle("Number of Raw Signal pairs");

      getStatistics().createHistogram(new TH1F(
        Form("tdiff_%d_B_%d_%d", scinID, i, j),
        Form("Time diff of signals on scin %d side B matrix positions %d vs %d", scinID, i, j),
        200, -1.1*fMergingTime, 1.1*fMergingTime
      ));
      getStatistics().getHisto1D(Form("tdiff_%d_B_%d_%d", scinID, i, j))
      ->GetXaxis()->SetTitle("Time difference [ps]");
      getStatistics().getHisto1D(Form("tdiff_%d_B_%d_%d", scinID, i, j))
      ->GetYaxis()->SetTitle("Number of Raw Signal pairs");
    }
  }

  // Same offsets but with SiPM ID in the name
  for(int pmID=fMinPMID; pmID<=fMaxPMID; pmID++){
    getStatistics().createHistogram(new TH1F(
      Form("offset_sipm_%d", pmID), Form("Offset for SiPM %d", pmID),
      200, -1.1*fMergingTime, 1.1*fMergingTime
    ));
    getStatistics().getHisto1D(Form("offset_sipm_%d", pmID))->GetXaxis()->SetTitle("Time difference [ps]");
    getStatistics().getHisto1D(Form("offset_sipm_%d", pmID))->GetYaxis()->SetTitle("Number of Raw Signal pairs");
  }

}
