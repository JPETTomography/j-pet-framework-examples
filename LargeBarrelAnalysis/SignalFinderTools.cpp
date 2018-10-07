/**
 *  @copyright Copyright 2018 The J-PET Framework Authors. All rights reserved.
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may find a copy of the License in the LICENCE file.
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  @file SignalFinderTools.cpp
 */

#include "SignalFinderTools.h"
using namespace std;

/**
 * Distributing Signal Channels to PMs and checking RecoFlag
 */
vector<vector<JPetSigCh>> SignalFinderTools::getSigChByPM(
  const JPetTimeWindow* timeWindow, const JPetParamBank& paramBank, bool useCorrupts
){
  if (!timeWindow) {
    WARNING("Pointer of Time Window object is not set, returning empty vector.");
    vector<vector<JPetSigCh>> emptyVec;
    return emptyVec;
  }
  // Init return vector
  int pmNumber = paramBank.getPMsSize();
  vector<JPetSigCh> sigChVec;
  vector<vector<JPetSigCh>> pmSigChVec(pmNumber, sigChVec);
  // Distribute Signal Channels according to PM they belong to
  const unsigned int nSigChs = timeWindow->getNumberOfEvents();
  for (unsigned int i = 0; i < nSigChs; i++) {
    auto sigCh = dynamic_cast<const JPetSigCh&>(timeWindow->operator[](i));
    // If it is set not to use Corrupted SigChs, such flagged objects will be skipped
    if(!useCorrupts && sigCh.getRecoFlag() == JPetSigCh::Corrupted) { continue; }
    int pmID = sigCh.getPM().getID();
    pmSigChVec.at(pmID-1).push_back(sigCh);
  }
  return pmSigChVec;
}

/**
 * Method invoking Raw Signal building method for each PM separately
 */
vector<JPetRawSignal> SignalFinderTools::buildAllSignals(
  vector<vector<JPetSigCh>> sigChByPM, int numOfThresholds,
  double sigChEdgeMaxTime, double sigChLeadTrailMaxTime,
  JPetStatistics& stats, bool saveHistos
){
  vector<JPetRawSignal> allSignals;
  for (auto sigChVec : sigChByPM) {
    auto signals = buildRawSignals(
      sigChVec, numOfThresholds, sigChEdgeMaxTime, sigChLeadTrailMaxTime, stats, saveHistos
    );
    allSignals.insert(allSignals.end(), signals.begin(), signals.end());
  }
return allSignals;
}

/**
 * @brief Reconstruction of Raw Signals based on Signal Channels on the same PM
 *
 * RawSignal is created with all Leading SigChs that are found within first
 * time window (sigChEdgeMaxTime parameter) and all Trailing SigChs that conform
 * to second time window (sigChLeadTrailMaxTime parameter).
 * Also invokes validation method.
 */
vector<JPetRawSignal> SignalFinderTools::buildRawSignals(
  vector<JPetSigCh> sigChFromSamePM, int numOfThresholds,
  double sigChEdgeMaxTime, double sigChLeadTrailMaxTime,
  JPetStatistics& stats, bool saveHistos)
{
  vector<JPetRawSignal> rawSigVec;
  // Threshold number check - fixed number equal 4
  if (numOfThresholds != 4) {
    ERROR("This function is meant to work with 4 thresholds only!");
    return rawSigVec;
  }

  auto leadSigChs = getSigChsByEdge(sigChFromSamePM, JPetSigCh::Leading);
  auto trailSigChs = getSigChsByEdge(sigChFromSamePM, JPetSigCh::Trailing);
  int trailingIterator = 0;

  for(unsigned int i = 0; i<leadSigChs.size(); i++) {
    JPetRawSignal rawSig;
    rawSig.setPM(leadSigChs.at(i).getPM());
    rawSig.setBarrelSlot(leadSigChs.at(i).getPM().getBarrelSlot());
    rawSig.addPoint(leadSigChs.at(i));
    double referenceValue = leadSigChs.at(i).getValue();
    // Check for fitting trailing signal channels
    for(unsigned int k = trailingIterator; k<trailSigChs.size(); k++) {
      if(fabs(referenceValue-trailSigChs.at(k).getValue()) < sigChLeadTrailMaxTime) {
        rawSig.addPoint(trailSigChs.at(k));
        if(saveHistos) {
          stats.getHisto1D("LT_time_diff")->Fill(fabs(referenceValue-trailSigChs.at(k).getValue()));
        }
      } else {
        trailingIterator = k;
        break;
      }
    }
    // Check for other fitting leading signal channels
    for(unsigned int j = i+1; j<leadSigChs.size(); j++) {
      if(fabs(referenceValue-leadSigChs.at(j).getValue())<sigChEdgeMaxTime) {
        rawSig.addPoint(leadSigChs.at(j));
        if(saveHistos) {
          stats.getHisto1D("L_time_diff")->Fill(fabs(referenceValue-leadSigChs.at(j).getValue()));
        }
      } else {
        i = j;
        break;
      }
    }
    if(validateSignal(rawSig, numOfThresholds)){
      rawSig.setRecoFlag(JPetRawSignal::Good);
      if(saveHistos) {
        stats.getHisto1D("good_v_bad_raw_sigs")->Fill(1);
      }
    }else{
      rawSig.setRecoFlag(JPetRawSignal::Corrupted);
      if(saveHistos) {
        stats.getHisto1D("good_v_bad_raw_sigs")->Fill(2);
      }
    }
    rawSigVec.push_back(rawSig);
  }
  return rawSigVec;
}

/**
 * Destributing Signal Channels to vectors, according to the Edge Type
 */
vector<JPetSigCh> SignalFinderTools::getSigChsByEdge(
  vector<JPetSigCh> sigChFromSamePM, JPetSigCh::EdgeType edgeType
){
  vector<JPetSigCh> sameEdgeSigCh;
  for (auto sigCh : sigChFromSamePM) {
    if(sigCh.getType() == edgeType) { sameEdgeSigCh.push_back(sigCh); }
  }
  return sameEdgeSigCh;
}

/**
 * @brief Method validating a signal - setting Reco Flag to Good or Corrupted.
 *
 * Good has Signal a THR with a pair of SigChs of Leading and Trailing edge
 * or is empty. If other case found - Corrupted Flag. Also if any of
 * the used SigChs has RecoFlag::Corrupted, that RawSignal inherits that setting.
 */
bool SignalFinderTools::validateSignal(JPetRawSignal rawSignal, int numOfThresholds)
{
  vector<JPetSigCh> tmpVec;
  vector<vector<JPetSigCh>> thrLeadSigChs(numOfThresholds, tmpVec);
  vector<vector<JPetSigCh>> thrTrailSigChs(numOfThresholds, tmpVec);
  auto leadSigChs = rawSignal.getPoints(JPetSigCh::Leading, JPetRawSignal::ByThrNum);
  auto trailSigChs = rawSignal.getPoints(JPetSigCh::Trailing, JPetRawSignal::ByThrNum);
  for(auto sigCh : leadSigChs) {
    if(sigCh.getRecoFlag() == JPetSigCh::Corrupted) { return false; }
    thrLeadSigChs.at(sigCh.getThresholdNumber()-1).push_back(sigCh);
  }
  for(auto sigCh : trailSigChs) {
    if(sigCh.getRecoFlag() == JPetSigCh::Corrupted){ return false; }
    thrTrailSigChs.at(sigCh.getThresholdNumber()-1).push_back(sigCh);
  }
  for(int i=1; i<numOfThresholds; i++){
    if(!(thrLeadSigChs.at(i).size()==0 && thrTrailSigChs.at(i).size()==0)
      && !(thrLeadSigChs.at(i).size()==1 && thrTrailSigChs.at(i).size()==1)) {
        return false;
    }
  }
  return true;
}
