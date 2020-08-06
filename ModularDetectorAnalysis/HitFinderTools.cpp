/**
 *  @copyright Copyright 2020 The J-PET Framework Authors. All rights reserved.
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
 *  @file HitFinderTools.cpp
 */

using namespace std;

#include "HitFinderTools.h"
#include <TMath.h>
#include <vector>
#include <cmath>
#include <map>

/**
 * Helper method for sotring signals in vector
 */
void HitFinderTools::sortByTime(vector<JPetMatrixSignal>& sigVec)
{
  sort(sigVec.begin(), sigVec.end(),
    [](const JPetMatrixSignal & sig1, const JPetMatrixSignal & sig2) {
      return sig1.getTime() < sig2.getTime();
    }
  );
 }

/**
 * Method distributing Signals according to Scintillator they belong to
 */
map<int, vector<JPetMatrixSignal>> HitFinderTools::getSignalsByScin(
  const JPetTimeWindow* timeWindow
){
  map<int, vector<JPetMatrixSignal>> signalScinMap;
  if (!timeWindow) {
    WARNING("Pointer of Time Window object is not set, returning empty map");
    return signalScinMap;
  }
  const unsigned int nSignals = timeWindow->getNumberOfEvents();
  for (unsigned int i = 0; i < nSignals; i++) {
    auto mtxSig = dynamic_cast<const JPetMatrixSignal&>(timeWindow->operator[](i));
    int scinID = mtxSig.getPM().getScin().getID();
    auto search = signalScinMap.find(scinID);
    if (search == signalScinMap.end()) {
      vector<JPetMatrixSignal> tmp;
      tmp.push_back(mtxSig);
      signalScinMap.insert(pair<int, vector<JPetMatrixSignal>>(scinID, tmp));
    } else {
      search->second.push_back(mtxSig);
    }
  }
  return signalScinMap;
}

/**
 * Loop over all Scins invoking matching procedure
 */
vector<JPetHit> HitFinderTools::matchAllSignals(
  map<int, vector<JPetMatrixSignal>>& allSignals, double timeDiffAB,
  int refDetScinID, JPetStatistics& stats, bool saveHistos
) {
  vector<JPetHit> allHits;
  vector<JPetHit> refHits;
  for (auto& scinSigals : allSignals) {
    // Loop for Reference Detector ID
    if (scinSigals.first == refDetScinID) {
      for (auto refSignal : scinSigals.second) {
        auto refHit = createDummyRefDetHit(refSignal);
        refHits.push_back(refHit);
        if (saveHistos && refHit.getEnergy()!=0.0) {
          stats.getHisto1D("ref_hit_signalB_tot")->Fill(refHit.getEnergy());
        }
      }
      allHits.insert(allHits.end(), refHits.begin(), refHits.end());
      if (saveHistos) {
        stats.getHisto1D("ref_hits_per_time_slot")->Fill(refHits.size());
      }
      continue;
    }
    // Match signals for scintillators
    auto scinHits = matchSignals(
      scinSigals.second, timeDiffAB, stats, saveHistos
    );
    allHits.insert(allHits.end(), scinHits.begin(), scinHits.end());
  }
  if (saveHistos) {
    stats.getHisto1D("hits_per_time_slot")->Fill(allHits.size());
  }
  return allHits;
}

/**
 * Method matching signals on the same Scintillator
 */
vector<JPetHit> HitFinderTools::matchSignals(
  vector<JPetMatrixSignal>& scinSigals, double timeDiffAB,
  JPetStatistics& stats, bool saveHistos
) {
  vector<JPetHit> scinHits;
  vector<JPetMatrixSignal> remainSignals;
  sortByTime(scinSigals);
  while (scinSigals.size() > 0) {
    auto mtxSig = scinSigals.at(0);
    if(scinSigals.size() == 1){
      remainSignals.push_back(mtxSig);
      break;
    }
    for (unsigned int j = 1; j < scinSigals.size(); j++) {
      if (scinSigals.at(j).getTime() - mtxSig.getTime() < timeDiffAB) {
        if (mtxSig.getPM().getSide() != scinSigals.at(j).getPM().getSide()) {
          auto hit = createHit(mtxSig, scinSigals.at(j));
          scinHits.push_back(hit);
          scinSigals.erase(scinSigals.begin() + j);
          scinSigals.erase(scinSigals.begin() + 0);
          break;
        } else {
          if (j == scinSigals.size() - 1) {
            remainSignals.push_back(mtxSig);
            scinSigals.erase(scinSigals.begin() + 0);
            break;
          } else { continue; }
        }
      } else {
        if(saveHistos && mtxSig.getPM().getSide() != scinSigals.at(j).getPM().getSide()){
          stats.getHisto1D("remain_signals_tdiff")->Fill(
            scinSigals.at(j).getTime() - mtxSig.getTime()
          );
        }
        remainSignals.push_back(mtxSig);
        scinSigals.erase(scinSigals.begin() + 0);
        break;
      }
    }
  }
  if(remainSignals.size()>0 && saveHistos){
    stats.getHisto1D("remain_signals_per_scin")
      ->Fill((double)(remainSignals.at(0).getPM().getScin().getID()), remainSignals.size());
  }
  return scinHits;
}

/**
 * Method for Hit creation - setting all fields that make sense here
 */
JPetHit HitFinderTools::createHit(
  const JPetMatrixSignal& signal1, const JPetMatrixSignal& signal2
) {
  JPetMatrixSignal signalA;
  JPetMatrixSignal signalB;

  if (signal1.getPM().getSide() == JPetPM::SideA) {
    signalA = signal1;
    signalB = signal2;
  } else {
    signalA = signal2;
    signalB = signal1;
  }

  JPetHit hit;
  hit.setSignals(signalA, signalB);
  hit.setTime((signalA.getTime() + signalB.getTime()) / 2.0);
  hit.setQualityOfTime(-1.0);
  hit.setTimeDiff(signalB.getTime() - signalA.getTime());
  hit.setQualityOfTimeDiff(-1.0);
  hit.setEnergy(signalA.getTOT()+signalB.getTOT());
  hit.setQualityOfEnergy(-1.0);
  hit.setPosX(signalA.getPM().getScin().getCenterX());
  hit.setPosY(signalA.getPM().getScin().getCenterY());
  // Hardcoded velocity = 11 ns/cm
  hit.setPosZ(11.0 * hit.getTimeDiff() / 2000.0);
  hit.setScin(signalA.getPM().getScin());

  return hit;
}

/**
 * Method for Hit creation in case of reference detector.
 * Setting only some necessary fields.
 */
JPetHit HitFinderTools::createDummyRefDetHit(const JPetMatrixSignal& signal)
{
  JPetHit hit;
  JPetMatrixSignal dummy;
  hit.setSignalA(dummy);
  hit.setSignalB(signal);
  hit.setTime(signal.getTime());
  hit.setQualityOfTime(-1.0);
  hit.setTimeDiff(0.0);
  hit.setQualityOfTimeDiff(-1.0);
  hit.setEnergy(signal.getTOT());
  hit.setQualityOfEnergy(-1.0);
  hit.setPosX(signal.getPM().getScin().getCenterX());
  hit.setPosY(signal.getPM().getScin().getCenterY());
  hit.setPosZ(0.0);
  hit.setScin(signal.getPM().getScin());
  return hit;
}

/**
* Calculation of the total TOT of the hit - Time over Threshold:
* the sum of the TOTs on both thresholds and on the both sides (A,B)
*/
double HitFinderTools::calculateTOT(JPetHit& hit)
{
  double tot = 0.0;

  auto rawSignalsA = dynamic_cast<const JPetMatrixSignal&>(hit.getSignalA()).getRawSignals();
  auto rawSignalsB = dynamic_cast<const JPetMatrixSignal&>(hit.getSignalB()).getRawSignals();

  for(auto rawSig: rawSignalsA){
    auto sigALead = rawSig.second.getPoints(JPetSigCh::Leading, JPetRawSignal::ByThrNum);
    auto sigATrail = rawSig.second.getPoints(JPetSigCh::Trailing, JPetRawSignal::ByThrNum);
    if (sigALead.size() > 0 && sigATrail.size() > 0){
      for (unsigned i = 0; i < sigALead.size() && i < sigATrail.size(); i++){
        tot += (sigATrail.at(i).getTime() - sigALead.at(i).getTime());
      }
    }
  }

  for(auto rawSig: rawSignalsB){
    auto sigBLead = rawSig.second.getPoints(JPetSigCh::Leading, JPetRawSignal::ByThrNum);
    auto sigBTrail = rawSig.second.getPoints(JPetSigCh::Trailing, JPetRawSignal::ByThrNum);
    if (sigBLead.size() > 0 && sigBTrail.size() > 0){
      for (unsigned i = 0; i < sigBLead.size() && i < sigBTrail.size(); i++){
        tot += (sigBTrail.at(i).getTime() - sigBLead.at(i).getTime());
      }
    }
  }

  return tot;
}
