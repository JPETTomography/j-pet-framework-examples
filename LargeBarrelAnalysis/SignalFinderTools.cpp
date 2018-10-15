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
 * Sorting method for Signal Channels by time value
 */
vector<JPetSigCh> SignalFinderTools::sortByValue(vector<JPetSigCh> input)
{
  auto sigChs(input);
  std::sort(sigChs.begin(), sigChs.end(),
    [] (JPetSigCh sigCh1, JPetSigCh sigCh2) {
      return sigCh1.getValue() < sigCh2.getValue();
    }
  );
  return sigChs;
}

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

  auto sortedSigChs = sortByValue(sigChFromSamePM);
  unsigned int lastTrailingIterator = 0;
  for(unsigned int i=0; i<sortedSigChs.size(); i++){
    auto sigCh1 = sortedSigChs.at(i);
    if(sigCh1.getType()==JPetSigCh::Trailing){
      if(saveHistos && i > lastTrailingIterator) {
        stats.getHisto1D("unused_trails")->Fill(sigCh1.getThresholdNumber());
      }
      continue;
    }
    if(sigCh1.getType()==JPetSigCh::Leading){
      JPetRawSignal rawSig;
      rawSig.addPoint(sigCh1);
      rawSig.setBarrelSlot(sigCh1.getPM().getBarrelSlot());
      rawSig.setPM(sigCh1.getPM());
      if(sigCh1.getRecoFlag() == JPetSigCh::Good){
        rawSig.setRecoFlag(JPetBaseSignal::Good);
      } else if(sigCh1.getRecoFlag() == JPetSigCh::Corrupted){
        rawSig.setRecoFlag(JPetBaseSignal::Corrupted);
      }

      bool noMoreLeadings = false;
      bool noMoreTrailings = false;
      int lastLeadingIterator = 0;
      for(unsigned int j=i+1; j<sortedSigChs.size(); j++){
        if(noMoreLeadings && noMoreTrailings) {
          i += lastLeadingIterator-1;
          break;
        }
        auto sigCh2 = sortedSigChs.at(j);
        if(sigCh2.getType()==JPetSigCh::Leading){
          if(fabs(sigCh2.getValue()-sigCh1.getValue()) < sigChEdgeMaxTime) {
            rawSig.addPoint(sigCh2);
            if(sigCh2.getRecoFlag() == JPetSigCh::Corrupted){
              rawSig.setRecoFlag(JPetBaseSignal::Corrupted);
            }
            lastLeadingIterator = j;
            if(saveHistos) {
              stats.getHisto1D("L_time_diff")
                ->Fill(fabs(sigCh2.getValue()-sigCh1.getValue()));
            }
          } else {
            noMoreLeadings = true;
          }
        }
        if(sigCh1.getType()==JPetSigCh::Trailing){
          if(fabs(sigCh2.getValue()-sigCh1.getValue()) < sigChLeadTrailMaxTime) {
            rawSig.addPoint(sigCh2);
            lastTrailingIterator = j;
            if(sigCh2.getRecoFlag() == JPetSigCh::Corrupted){
              rawSig.setRecoFlag(JPetBaseSignal::Corrupted);
            }
            if(saveHistos) {
              stats.getHisto1D("LT_time_diff")
                ->Fill(fabs(sigCh2.getValue()-sigCh1.getValue()));
            }
          } else {
            noMoreTrailings = true;
          }
        }
      }
      if(saveHistos) {
        if(rawSig.getRecoFlag()==JPetBaseSignal::Good){
          stats.getHisto1D("good_v_bad_raw_sigs")->Fill(1);
        } else if(rawSig.getRecoFlag()==JPetBaseSignal::Corrupted){
          stats.getHisto1D("good_v_bad_raw_sigs")->Fill(2);
        }
      }
      rawSigVec.push_back(rawSig);
    }
  }
  return rawSigVec;
}
