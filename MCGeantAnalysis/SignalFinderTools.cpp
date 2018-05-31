/**
 *  @copyright Copyright 2016 The J-PET Framework Authors. All rights reserved.
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
 *  @file SignalFinderToolsTools.cpp
 */

#include "SignalFinderTools.h"
using namespace std;

/**
 * Method returns a map of vectors of JPetSigCh ordered by photomultiplier ID
 */
map<int, vector<JPetSigCh>> SignalFinderTools::getSigChsPMMapById(
  const JPetTimeWindow* timeWindow)
{
  map<int, vector<JPetSigCh>> sigChsPMMap;
  if (!timeWindow) {
    WARNING("Pointer of Time Window object is not set, returning empty map");
    return sigChsPMMap;
  }

  // Map Signal Channels according to PM they belong to
  const unsigned int nSigChs = timeWindow->getNumberOfEvents();
  for (unsigned int i = 0; i < nSigChs; i++) {
    JPetSigCh sigCh = dynamic_cast<const JPetSigCh&>(timeWindow->operator[](i));
    int pmtID = sigCh.getPM().getID();
    auto search = sigChsPMMap.find(pmtID);
    if (search == sigChsPMMap.end()) {
      vector<JPetSigCh> tmp;
      tmp.push_back(sigCh);
      sigChsPMMap.insert(pair<int, vector<JPetSigCh>>(pmtID, tmp));
    } else search->second.push_back(sigCh);
  }
  return sigChsPMMap;
}

/**
 * Method invoking Raw Signal building method
 * for each PM separately, looping over whole PM map
 */
vector<JPetRawSignal> SignalFinderTools::buildAllSignals(
  map<int, vector<JPetSigCh>> sigChsPMMap,
  unsigned int numOfThresholds,
  JPetStatistics& stats,
  double sigChEdgeMaxTime,
  double sigChLeadTrailMaxTime,
  bool saveHistos)
{
  vector<JPetRawSignal> allSignals;
  for (auto& sigChPair : sigChsPMMap) {
    vector<JPetRawSignal> currentSignals = buildRawSignals(
      sigChPair.second,
      numOfThresholds,
      stats,
      sigChEdgeMaxTime,
      sigChLeadTrailMaxTime,
      saveHistos);
    allSignals.insert(
      allSignals.end(),
      currentSignals.begin(),
      currentSignals.end());
  }
  return allSignals;
}

/**
 * Method reconstructs Raw Signals based on Signal Channels on one PM
 */
vector<JPetRawSignal> SignalFinderTools::buildRawSignals(
  const vector<JPetSigCh>& sigChFromSamePM,
  unsigned int numOfThresholds,
  JPetStatistics& stats,
  double sigChEdgeMaxTime,
  double sigChLeadTrailMaxTime,
  bool saveHistos)
{
  vector<JPetRawSignal> rawSigVec;

  // Threshold number check - fixed number equal 4
  if (numOfThresholds != 4) {
    ERROR("This function is meant to work with 4 thresholds only!");
    return rawSigVec;
  }

  vector<JPetSigCh> tmpVec;
  vector<vector<JPetSigCh>> thrLeadingSigCh(numOfThresholds, tmpVec);
  vector<vector<JPetSigCh>> thrTrailingSigCh(numOfThresholds, tmpVec);

  for (const JPetSigCh & sigCh : sigChFromSamePM) {
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

    // Searching for matching trailing on first THR
    int closestTrailingSigCh = findTrailingSigCh(
      thrLeadingSigCh.at(0).at(0),
      thrTrailingSigCh.at(0),
      sigChLeadTrailMaxTime
    );

    if(closestTrailingSigCh != -1) {
      rawSig.addPoint(thrTrailingSigCh.at(0).at(closestTrailingSigCh));
      if(saveHistos)
        stats.getHisto1D("lead_trail_thr1_diff")->Fill(
          thrTrailingSigCh.at(0)
            .at(closestTrailingSigCh).getValue()-thrLeadingSigCh.at(0).at(0).getValue()
        );
      thrTrailingSigCh.at(0).erase(thrTrailingSigCh.at(0).begin()+closestTrailingSigCh);
    }

    // Procedure follows in loop for THR 2,3,4
    // First search for leading SigCh on iterated THR,
    // then search for trailing SigCh on iterated THR
    for(uint kk=1;kk<numOfThresholds;kk++){
      int nextThrSigChIndex = findSigChOnNextThr(
        thrLeadingSigCh.at(0).at(0).getValue(),
        thrLeadingSigCh.at(kk),
        sigChEdgeMaxTime
      );

      if (nextThrSigChIndex != -1) {
        closestTrailingSigCh = findTrailingSigCh(
          thrLeadingSigCh.at(0).at(0),
          thrTrailingSigCh.at(kk),
          sigChLeadTrailMaxTime
        );

        if (closestTrailingSigCh != -1) {
          rawSig.addPoint(thrTrailingSigCh.at(kk).at(closestTrailingSigCh));
          if(saveHistos)
            stats.getHisto1D(Form("lead_trail_thr%d_diff", kk+1))->Fill(
              thrTrailingSigCh.at(kk)
                .at(closestTrailingSigCh).getValue()-thrLeadingSigCh.at(kk).at(nextThrSigChIndex).getValue()
            );
          thrTrailingSigCh.at(kk).erase(thrTrailingSigCh.at(kk).begin()+closestTrailingSigCh);
        }

        rawSig.addPoint(thrLeadingSigCh.at(kk).at(nextThrSigChIndex));
        if(saveHistos)
          stats.getHisto1D(Form("lead_thr1_thr%d_diff", kk+1))->Fill(
            thrLeadingSigCh.at(kk)
              .at(nextThrSigChIndex).getValue()-thrLeadingSigCh.at(0).at(0).getValue()
          );
        thrLeadingSigCh.at(kk).erase(thrLeadingSigCh.at(kk).begin()+nextThrSigChIndex);
      }
    }

    // Adding created Raw Signal to vector
    rawSigVec.push_back(rawSig);
    thrLeadingSigCh.at(0).erase(thrLeadingSigCh.at(0).begin());
  }

  // Filling controll histograms
  if(saveHistos)
    for(uint jj=0;jj<numOfThresholds;jj++){
      stats.getHisto1D("remainig_leading_sig_ch_per_thr")
        ->Fill(jj+1, thrLeadingSigCh.at(jj).size());
      stats.getHisto1D("remainig_trailing_sig_ch_per_thr")
        ->Fill(jj+1, thrTrailingSigCh.at(jj).size());
    }
  return rawSigVec;
}

/**
 * Method finds Signal Channels that belong to the same leading edge
 */
int SignalFinderTools::findSigChOnNextThr(
  double sigChValue,
  const vector<JPetSigCh>& sigChVec,
  double sigChEdgeMaxTime)
{
  for (size_t i = 0; i < sigChVec.size(); i++) {
    if (fabs(sigChValue - sigChVec.at(i).getValue()) < sigChEdgeMaxTime)
      return i;
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
  const JPetSigCh& leadingSigCh,
  const vector<JPetSigCh>& trailingSigChVec,
  double sigChLeadTrailMaxTime)
{
  vector<int> trailingFoundIdices;
  for (size_t i = 0; i < trailingSigChVec.size(); i++) {
    if (fabs(leadingSigCh.getValue() - trailingSigChVec.at(i).getValue()) < sigChLeadTrailMaxTime)
      trailingFoundIdices.push_back(i);
  }
  if (trailingFoundIdices.size() == 0) return -1;
  sort(trailingFoundIdices.begin(), trailingFoundIdices.end());
  return trailingFoundIdices.at(0);
}
