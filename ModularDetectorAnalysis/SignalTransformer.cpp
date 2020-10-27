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
    auto mergedSignals = SignalTransformerTools::mergeSignalsAllSiPMs(rawSigMtxMap, fMergingTime);

    // Saving method invocation
    if(mergedSignals.size()>0){
      saveMatrixSignals(mergedSignals);
    }

  } else { return false; }
  return true;
}

bool SignalTransformer::terminate()
{
  // if(fSaveControlHistos){
    // namespace pt = boost::property_tree;
    // using namespace std;

  //   if (isOptionSet(fParams.getOptions(), kOffestsFileParamKey)) {
  //     INFO("Signal transforming - printing out offsets for SiPMs in matrices");
  //     fOffsetsFile = getOptionAsString(fParams.getOptions(), kOffestsFileParamKey);
  //
  //     pt::ptree root;
  //     pt::ptree sipm_node;
  //
  //     for(int pmID=fMinPMID; pmID<=fMaxPMID; pmID++){
  //       auto mean = getStatistics().getHisto1D(Form("offset_sipm_%d", pmID))->GetMean();
  //       sipm_node.put(to_string(pmID), mean);
  //     }
  //     root.add_child("sipm_offsets", sipm_node);
  //
  //     // Merging used calibration with new one - iteration alike
  //     if (isOptionSet(fParams.getOptions(), kSiPMCalibFileParamKey)) {
  //       auto siPMCalibFileName = getOptionAsString(fParams.getOptions(), kSiPMCalibFileParamKey);
  //
  //       pt::ptree rootOld;
  //       pt::read_json(siPMCalibFileName, rootOld);
  //
  //       pt::ptree new_root;
  //       pt::ptree new_sipm_node;
  //
  //       for(int pmID=fMinPMID; pmID<=fMaxPMID; pmID++){
  //         double oldOffset = rootOld.get("sipm_offsets."+to_string(pmID), 0.0);
  //         double newOffset = root.get("sipm_offsets."+to_string(pmID), 0.0);
  //         new_sipm_node.put(to_string(pmID), oldOffset+newOffset);
  //       }
  //       new_root.add_child("sipm_offsets", new_sipm_node);
  //       pt::write_json(fOffsetsFile, new_root);
  //
  //     } else {
  //       pt::write_json(fOffsetsFile, root);
  //     }
  //   }
  // }

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
      getStatistics().getHisto1D("mtxsig_multi")->Fill(mtxSig.getRawSignals().size());
      if(mtxSig.getPM().getSide()==JPetPM::SideA){
        getStatistics().getHisto1D("mtxsig_per_scin_sideA")->Fill(scinID);
      } else if(mtxSig.getPM().getSide()==JPetPM::SideB){
        getStatistics().getHisto1D("mtxsig_per_scin_sideB")->Fill(scinID);
      }
      // auto rawSigVec = mtxSig.getRawSignals();
      // if(rawSigVec.size() == 4) {
      //   auto side = rawSigVec.at(1).getPM().getSide();
      //   auto t1 = SignalTransformerTools::getRawSigBaseTime(rawSigVec.at(1));
      //   auto t2 = SignalTransformerTools::getRawSigBaseTime(rawSigVec.at(2));
      //   auto t3 = SignalTransformerTools::getRawSigBaseTime(rawSigVec.at(3));
      //   auto t4 = SignalTransformerTools::getRawSigBaseTime(rawSigVec.at(4));
      //
      //   if(rawSigVec.at(2).getPM().getID()>fMinPMID && rawSigVec.at(2).getPM().getID()<fMaxPMID){
      //     getStatistics().getHisto1D(Form("offset_sipm_%d", rawSigVec.at(2).getPM().getID()))->Fill(t2-t1);
      //   }
      //   if(rawSigVec.at(3).getPM().getID()>fMinPMID && rawSigVec.at(3).getPM().getID()<fMaxPMID){
      //     getStatistics().getHisto1D(Form("offset_sipm_%d", rawSigVec.at(3).getPM().getID()))->Fill(t3-t1);
      //   }
      //   if(rawSigVec.at(4).getPM().getID()>fMinPMID && rawSigVec.at(4).getPM().getID()<fMaxPMID){
      //     getStatistics().getHisto1D(Form("offset_sipm_%d", rawSigVec.at(4).getPM().getID()))->Fill(t4-t1);
      //   }
      //
      //   if(side==JPetPM::SideA) {
      //     getStatistics().getHisto1D(Form("tdiff_%d_A_%d_%d", scinID, 2, 1))->Fill(t2-t1);
      //     getStatistics().getHisto1D(Form("tdiff_%d_A_%d_%d", scinID, 3, 1))->Fill(t3-t1);
      //     getStatistics().getHisto1D(Form("tdiff_%d_A_%d_%d", scinID, 4, 1))->Fill(t4-t1);
      //   } else if(side==JPetPM::SideB){
      //     getStatistics().getHisto1D(Form("tdiff_%d_B_%d_%d", scinID, 2, 1))->Fill(t2-t1);
      //     getStatistics().getHisto1D(Form("tdiff_%d_B_%d_%d", scinID, 3, 1))->Fill(t3-t1);
      //     getStatistics().getHisto1D(Form("tdiff_%d_B_%d_%d", scinID, 4, 1))->Fill(t4-t1);
      //   }
      // }
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

  // // SiPM offsets
  // for(int pmID=fMinPMID; pmID<=fMaxPMID; pmID++){
  //   getStatistics().createHistogram(new TH1F(
  //     Form("offset_sipm_%d", pmID), Form("Offset for SiPM %d", pmID),
  //     200, -1.1*fMergingTime, 1.1*fMergingTime
  //   ));
  //   getStatistics().getHisto1D(Form("offset_sipm_%d", pmID))->GetXaxis()->SetTitle("Time difference [ps]");
  //   getStatistics().getHisto1D(Form("offset_sipm_%d", pmID))->GetYaxis()->SetTitle("Number of Raw Signal pairs");
  // }
}
