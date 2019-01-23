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
 * Method returns a map of vectors of JPetSigCh ordered by photomultiplier ID
 */
const map<int, vector<JPetSigCh>> SignalFinderTools::getSigChByPM(
  const JPetTimeWindow* timeWindow, bool useCorrupts
){
  map<int, vector<JPetSigCh>> sigChsPMMap;
  if (!timeWindow) {
    WARNING("Pointer of Time Window object is not set, returning empty map");
    return sigChsPMMap;
  }
  // Map Signal Channels according to PM they belong to
  const unsigned int nSigChs = timeWindow->getNumberOfEvents();
  for (unsigned int i = 0; i < nSigChs; i++) {
    auto sigCh = dynamic_cast<const JPetSigCh&>(timeWindow->operator[](i));
    // If it is set not to use Corrupted SigChs, such flagged objects will be skipped
    if(!useCorrupts && sigCh.getRecoFlag() == JPetSigCh::Corrupted) { continue; }
    int pmtID = sigCh.getPM().getID();
    auto search = sigChsPMMap.find(pmtID);
    if (search == sigChsPMMap.end()) {
      vector<JPetSigCh> tmp;
      tmp.push_back(sigCh);
      sigChsPMMap.insert(pair<int, vector<JPetSigCh>>(pmtID, tmp));
    } else {
      search->second.push_back(sigCh);
    }
  }
  return sigChsPMMap;
}

/**
 * Method invoking Raw Signal building method for each PM separately
 */
vector<JPetRawSignal> SignalFinderTools::buildAllSignals(
   const map<int, vector<JPetSigCh>>& sigChByPM, unsigned int numOfThresholds,
   double sigChEdgeMaxTime, double sigChLeadTrailMaxTime,
   JPetStatistics& stats, bool saveHistos
) {
  vector<JPetRawSignal> allSignals;
  for (auto& sigChPair : sigChByPM) {
    auto signals = buildRawSignals(
      sigChPair.second, numOfThresholds, sigChEdgeMaxTime,
      sigChLeadTrailMaxTime, stats, saveHistos
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
 */
 vector<JPetRawSignal> SignalFinderTools::buildRawSignals(
   const vector<JPetSigCh>& sigChByPM, unsigned int numOfThresholds,
   double sigChEdgeMaxTime, double sigChLeadTrailMaxTime,
   JPetStatistics& stats, bool saveHistos
 ) {
  vector<JPetRawSignal> rawSigVec;
  // Threshold number check - fixed number equal 4
  if (numOfThresholds != 4) {
    ERROR("This function is meant to work with 4 thresholds only!");
    return rawSigVec;
  }
  vector<JPetSigCh> tmpVec;
  vector<vector<JPetSigCh>> thrLeadingSigCh(numOfThresholds, tmpVec);
  vector<vector<JPetSigCh>> thrTrailingSigCh(numOfThresholds, tmpVec);
  for (const JPetSigCh& sigCh : sigChByPM) {
    if(sigCh.getType() == JPetSigCh::Leading) {
      thrLeadingSigCh.at(sigCh.getThresholdNumber()-1).push_back(sigCh);
    } else if(sigCh.getType() == JPetSigCh::Trailing) {
      thrTrailingSigCh.at(sigCh.getThresholdNumber()-1).push_back(sigCh);
    }
  }
  assert(thrLeadingSigCh.size() > 0);
  while (thrLeadingSigCh.at(0).size() > 0) {
    JPetRawSignal rawSig;
    rawSig.setPM(thrLeadingSigCh.at(0).at(0).getPM());
    rawSig.setBarrelSlot(thrLeadingSigCh.at(0).at(0).getPM().getBarrelSlot());
    // First THR leading added by default
    rawSig.addPoint(thrLeadingSigCh.at(0).at(0));
    if(thrLeadingSigCh.at(0).at(0).getRecoFlag()==JPetSigCh::Good){
      rawSig.setRecoFlag(JPetBaseSignal::Good);
    } else if(thrLeadingSigCh.at(0).at(0).getRecoFlag()==JPetSigCh::Corrupted){
      rawSig.setRecoFlag(JPetBaseSignal::Corrupted);
    }
    // Searching for matching trailing on first THR
    int closestTrailingSigCh = findTrailingSigCh(
      thrLeadingSigCh.at(0).at(0), sigChLeadTrailMaxTime, thrTrailingSigCh.at(0)
    );
    if(closestTrailingSigCh != -1) {
      rawSig.addPoint(thrTrailingSigCh.at(0).at(closestTrailingSigCh));
      if(thrTrailingSigCh.at(0).at(closestTrailingSigCh).getRecoFlag()==JPetSigCh::Corrupted){
        rawSig.setRecoFlag(JPetBaseSignal::Corrupted);
      }
      if(saveHistos){
        stats.getHisto1D("lead_trail_thr1_diff")->Fill(
          thrTrailingSigCh.at(0).at(closestTrailingSigCh).getValue()-thrLeadingSigCh.at(0).at(0).getValue()
        );
      }
      thrTrailingSigCh.at(0).erase(thrTrailingSigCh.at(0).begin()+closestTrailingSigCh);
    }
    // Procedure follows in loop for THR 2,3,4
    // First search for leading SigCh on iterated THR,
    // then search for trailing SigCh on iterated THR
    for(unsigned int kk=1;kk<numOfThresholds;kk++){
      int nextThrSigChIndex = findSigChOnNextThr(
        thrLeadingSigCh.at(0).at(0).getValue(), sigChEdgeMaxTime, thrLeadingSigCh.at(kk)
      );
      if (nextThrSigChIndex != -1) {
        closestTrailingSigCh = findTrailingSigCh(
          thrLeadingSigCh.at(0).at(0), sigChLeadTrailMaxTime, thrTrailingSigCh.at(kk)
        );
        if (closestTrailingSigCh != -1) {
          rawSig.addPoint(thrTrailingSigCh.at(kk).at(closestTrailingSigCh));
          if(thrTrailingSigCh.at(kk).at(closestTrailingSigCh).getRecoFlag()==JPetSigCh::Corrupted){
            rawSig.setRecoFlag(JPetBaseSignal::Corrupted);
          }
          if(saveHistos){
            stats.getHisto1D(Form("lead_trail_thr%d_diff", kk+1))->Fill(
              thrTrailingSigCh.at(kk).at(closestTrailingSigCh).getValue()
                -thrLeadingSigCh.at(kk).at(nextThrSigChIndex).getValue()
            );
          }
          thrTrailingSigCh.at(kk).erase(thrTrailingSigCh.at(kk).begin()+closestTrailingSigCh);
        }
        rawSig.addPoint(thrLeadingSigCh.at(kk).at(nextThrSigChIndex));
        if(thrLeadingSigCh.at(kk).at(nextThrSigChIndex).getRecoFlag()==JPetSigCh::Corrupted){
          rawSig.setRecoFlag(JPetBaseSignal::Corrupted);
        }
        if(saveHistos){
          stats.getHisto1D(Form("lead_thr1_thr%d_diff", kk+1))->Fill(
            thrLeadingSigCh.at(kk).at(nextThrSigChIndex).getValue()-thrLeadingSigCh.at(0).at(0).getValue()
          );
        }
        thrLeadingSigCh.at(kk).erase(thrLeadingSigCh.at(kk).begin()+nextThrSigChIndex);
      }
    }
    if(saveHistos){
      if(rawSig.getRecoFlag()==JPetBaseSignal::Good){
        stats.getHisto1D("good_v_bad_raw_sigs")->Fill(1);
      } else if(rawSig.getRecoFlag()==JPetBaseSignal::Corrupted){
        stats.getHisto1D("good_v_bad_raw_sigs")->Fill(2);
      } else if(rawSig.getRecoFlag()==JPetBaseSignal::Unknown){
        stats.getHisto1D("good_v_bad_raw_sigs")->Fill(3);
      }
    }
    // Adding created Raw Signal to vector
    rawSigVec.push_back(rawSig);
    thrLeadingSigCh.at(0).erase(thrLeadingSigCh.at(0).begin());
  }
  // Filling control histograms
  if(saveHistos){
    for(unsigned int jj=0;jj<numOfThresholds;jj++){
      for(auto sigCh : thrLeadingSigCh.at(jj)){
        stats.getHisto1D("unused_sigch_all")->Fill(2*sigCh.getThresholdNumber()-1);
        if(sigCh.getRecoFlag()==JPetSigCh::Good){
          stats.getHisto1D("unused_sigch_good")->Fill(2*sigCh.getThresholdNumber()-1);
        } else if(sigCh.getRecoFlag()==JPetSigCh::Corrupted){
          stats.getHisto1D("unused_sigch_corr")->Fill(2*sigCh.getThresholdNumber()-1);
        }
      }
      for(auto sigCh : thrTrailingSigCh.at(jj)){
        stats.getHisto1D("unused_sigch_all")->Fill(2*sigCh.getThresholdNumber());
        if(sigCh.getRecoFlag()==JPetSigCh::Good){
          stats.getHisto1D("unused_sigch_good")->Fill(2*sigCh.getThresholdNumber());
        } else if(sigCh.getRecoFlag()==JPetSigCh::Corrupted){
          stats.getHisto1D("unused_sigch_corr")->Fill(2*sigCh.getThresholdNumber());
        }
      }
    }
  }
  return rawSigVec;
}

/**
 * Method finds Signal Channels that belong to the same leading edge
 */
int SignalFinderTools::findSigChOnNextThr(
  double sigChValue,double sigChEdgeMaxTime,
  const vector<JPetSigCh>& sigChVec
) {
  for (size_t i = 0; i < sigChVec.size(); i++) {
    if (fabs(sigChValue-sigChVec.at(i).getValue()) < sigChEdgeMaxTime){ return i; }
  }
  return -1;
}

/**
 * Method finds trailing edge Signal Channel that suits certian leading edge
 * Signal Channel, if more than one trailing edge Signal Channel found,
 * returning the one with the smallest index, that is equivalent of SigCh
 * earliest in time
 */
int SignalFinderTools::findTrailingSigCh(
  const JPetSigCh& leadingSigCh, double sigChLeadTrailMaxTime,
  const vector<JPetSigCh>& trailingSigChVec
) {
  vector<int> trailingFoundIdices;
  for (size_t i = 0; i < trailingSigChVec.size(); i++) {
    double timeDiff = trailingSigChVec.at(i).getValue() - leadingSigCh.getValue();
    if (timeDiff > 0.0 && timeDiff < sigChLeadTrailMaxTime){
      trailingFoundIdices.push_back(i);
    }
  }
  if (trailingFoundIdices.size() == 0) { return -1; }
  sort(trailingFoundIdices.begin(), trailingFoundIdices.end());
  return trailingFoundIdices.at(0);
}

/**
 * Method finds a 4-element permutation which has to be applied to threshold numbers
 * to have them sorted by increasing threshold values.
 *
 * The ordering may be different for each PMT, therefore the method creates a map
 * with PMT ID numbers as keys and 4-element permutations as values.
 */
ThresholdOrderings SignalFinderTools::findThresholdOrders(const JPetParamBank& bank){

  ThresholdOrderings orderings;
  std::map<unsigned int, std::array<float, 4> > thr_values;
  
  for(auto& tc: bank.getTOMBChannels()){
    unsigned int id = tc.second->getPM().getID();
    
    thr_values[id][tc.second->getLocalChannelNumber()-1] = tc.second->getThreshold();
  }

  for(auto& pm: thr_values){
    orderings[pm.first] = {1,2,3,4};
    auto& indices = orderings[pm.first];
    auto& values = pm.second;
    
    sort(indices.begin(), indices.end(),
         [&](const int& a, const int& b) {
           return (values.at(a-1) < values.at(b-1));
         }
         );
  }

  return orderings;
}
