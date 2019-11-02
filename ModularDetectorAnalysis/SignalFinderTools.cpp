/**
 *  @copyright Copyright 2019 The J-PET Framework Authors. All rights reserved.
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
  const JPetTimeWindow* timeWindow, bool useCorrupts, int refPMID
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
    // Here we ignore the corrupted flag of signals from Refference detector
    // since removing them results in double peak structures in the calibration spectra
    int pmtID = sigCh.getChannel().getPM().getID();
    if(pmtID == refPMID) {
      sigCh.setRecoFlag(JPetSigCh::Good);
    }

    if(!useCorrupts && sigCh.getRecoFlag() == JPetSigCh::Corrupted) { continue; }

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
  const map<int, vector<JPetSigCh>>& sigChByPM,
  double sigChEdgeMaxTime, double sigChLeadTrailMaxTime,
  int numberOfThrs, JPetStatistics& stats, bool saveHistos
) {
 vector<JPetRawSignal> allSignals;
 for (auto& sigChPair : sigChByPM) {
    auto signals = buildRawSignals(
      sigChPair.second, sigChEdgeMaxTime, sigChLeadTrailMaxTime, numberOfThrs, stats, saveHistos
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
  const vector<JPetSigCh>& sigChByPM,
  double sigChEdgeMaxTime, double sigChLeadTrailMaxTime,
  int numberOfThrs, JPetStatistics& stats, bool saveHistos
) {
  vector<JPetRawSignal> rawSigVec;

  vector<JPetSigCh> tmpVec;
  vector<vector<JPetSigCh>> thrLeadingSigCh(numberOfThrs, tmpVec);
  vector<vector<JPetSigCh>> thrTrailingSigCh(numberOfThrs, tmpVec);

  for (const JPetSigCh& sigCh : sigChByPM) {
    if(sigCh.getType() == JPetSigCh::Leading) {
      thrLeadingSigCh.at(sigCh.getChannel().getThresholdNumber()-1).push_back(sigCh);
    } else if(sigCh.getType() == JPetSigCh::Trailing) {
      thrTrailingSigCh.at(sigCh.getChannel().getThresholdNumber()-1).push_back(sigCh);
    }
  }

  assert(thrLeadingSigCh.size() > 0);
  while (thrLeadingSigCh.at(0).size() > 0) {
    JPetRawSignal rawSig;
    rawSig.setPM(thrLeadingSigCh.at(0).at(0).getChannel().getPM());

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
          thrTrailingSigCh.at(0).at(closestTrailingSigCh).getTime()-thrLeadingSigCh.at(0).at(0).getTime()
        );
      }
      thrTrailingSigCh.at(0).erase(thrTrailingSigCh.at(0).begin()+closestTrailingSigCh);
    }

    // Procedure follows for THR 2
    int nextThrSigChIndex = findSigChOnNextThr(
      thrLeadingSigCh.at(0).at(0).getTime(), sigChEdgeMaxTime, thrLeadingSigCh.at(1)
    );
    if (nextThrSigChIndex != -1) {
      closestTrailingSigCh = findTrailingSigCh(
        thrLeadingSigCh.at(0).at(0), sigChLeadTrailMaxTime, thrTrailingSigCh.at(1)
      );
      if (closestTrailingSigCh != -1) {
        rawSig.addPoint(thrTrailingSigCh.at(1).at(closestTrailingSigCh));
        if(thrTrailingSigCh.at(1).at(closestTrailingSigCh).getRecoFlag()==JPetSigCh::Corrupted){
          rawSig.setRecoFlag(JPetBaseSignal::Corrupted);
        }
        if(saveHistos){
          stats.getHisto1D("lead_trail_thr2_diff")->Fill(
            thrTrailingSigCh.at(1).at(closestTrailingSigCh).getTime()-thrLeadingSigCh.at(1).at(nextThrSigChIndex).getTime()
          );
        }
        thrTrailingSigCh.at(1).erase(thrTrailingSigCh.at(1).begin()+closestTrailingSigCh);
      }
      rawSig.addPoint(thrLeadingSigCh.at(1).at(nextThrSigChIndex));
      if(thrLeadingSigCh.at(1).at(nextThrSigChIndex).getRecoFlag()==JPetSigCh::Corrupted){
        rawSig.setRecoFlag(JPetBaseSignal::Corrupted);
      }
      if(saveHistos){
        stats.getHisto1D("lead_thr1_thr2_diff")->Fill(
          thrLeadingSigCh.at(1).at(nextThrSigChIndex).getTime()-thrLeadingSigCh.at(0).at(0).getTime()
        );
      }
      thrLeadingSigCh.at(1).erase(thrLeadingSigCh.at(1).begin()+nextThrSigChIndex);
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
    for(unsigned int jj=0; jj<numberOfThrs; jj++){
      for(auto sigCh : thrLeadingSigCh.at(jj)){
        stats.getHisto1D("unused_sigch_all")->Fill(2*sigCh.getChannel().getThresholdNumber()-1);
        if(sigCh.getRecoFlag()==JPetSigCh::Good){
          stats.getHisto1D("unused_sigch_good")->Fill(2*sigCh.getChannel().getThresholdNumber()-1);
        } else if(sigCh.getRecoFlag()==JPetSigCh::Corrupted){
          stats.getHisto1D("unused_sigch_corr")->Fill(2*sigCh.getChannel().getThresholdNumber()-1);
        }
      }
      for(auto sigCh : thrTrailingSigCh.at(jj)){
        stats.getHisto1D("unused_sigch_all")->Fill(2*sigCh.getChannel().getThresholdNumber());
        if(sigCh.getRecoFlag()==JPetSigCh::Good){
          stats.getHisto1D("unused_sigch_good")->Fill(2*sigCh.getChannel().getThresholdNumber());
        } else if(sigCh.getRecoFlag()==JPetSigCh::Corrupted){
          stats.getHisto1D("unused_sigch_corr")->Fill(2*sigCh.getChannel().getThresholdNumber());
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
    if (fabs(sigChValue-sigChVec.at(i).getTime()) < sigChEdgeMaxTime){ return i; }
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
    double timeDiff = trailingSigChVec.at(i).getTime() - leadingSigCh.getTime();
    if (timeDiff > 0.0 && timeDiff < sigChLeadTrailMaxTime){
      trailingFoundIdices.push_back(i);
    }
  }
  if (trailingFoundIdices.size() == 0) { return -1; }
  sort(trailingFoundIdices.begin(), trailingFoundIdices.end());
  return trailingFoundIdices.at(0);
}
