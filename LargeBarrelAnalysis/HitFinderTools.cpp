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
 *  @file HitFinderTools.cpp
 */

using namespace std;

#include "UniversalFileLoader.h"
#include "HitFinderTools.h"
#include <TMath.h>
#include <vector>
#include <cmath>
#include <map>

/**
 * Helper method for sotring signals in vector
 */
void HitFinderTools::sortByTime(vector<JPetPhysSignal>& sigVec)
{
  sort(sigVec.begin(), sigVec.end(),
    [](const JPetPhysSignal & sig1, const JPetPhysSignal & sig2) {
      return sig1.getTime() < sig2.getTime();
    }
  );
 }

/**
 * Method distributing Signals according to Scintillator they belong to
 */
map<int, vector<JPetPhysSignal>> HitFinderTools::getSignalsBySlot(
  const JPetTimeWindow* timeWindow, bool useCorrupts
){
  map<int, vector<JPetPhysSignal>> signalSlotMap;
  if (!timeWindow) {
    WARNING("Pointer of Time Window object is not set, returning empty map");
    return signalSlotMap;
  }
  const unsigned int nSignals = timeWindow->getNumberOfEvents();
  for (unsigned int i = 0; i < nSignals; i++) {
    auto physSig = dynamic_cast<const JPetPhysSignal&>(timeWindow->operator[](i));
    if(!useCorrupts && physSig.getRecoFlag() == JPetBaseSignal::Corrupted) { continue; }
    int slotID = physSig.getBarrelSlot().getID();
    auto search = signalSlotMap.find(slotID);
    if (search == signalSlotMap.end()) {
      vector<JPetPhysSignal> tmp;
      tmp.push_back(physSig);
      signalSlotMap.insert(pair<int, vector<JPetPhysSignal>>(slotID, tmp));
    } else {
      search->second.push_back(physSig);
    }
  }
  return signalSlotMap;
}

/**
 * Loop over all Scins invoking matching procedure
 */
vector<JPetHit> HitFinderTools::matchAllSignals(
  map<int, vector<JPetPhysSignal>>& allSignals,
  const map<unsigned int, vector<double>>& velocitiesMap,
  double timeDiffAB, int refDetScinId, JPetStatistics& stats, bool saveHistos
) {
  vector<JPetHit> allHits;
  for (auto& slotSigals : allSignals) {
    // Loop for Reference Detector ID
    if (slotSigals.first == refDetScinId) {
      for (auto refSignal : slotSigals.second) {
        auto refHit = createDummyRefDetHit(refSignal);
        allHits.push_back(refHit);
      }
      continue;
    }
    // Loop for other slots than reference one
    auto slotHits = matchSignals(
      slotSigals.second, velocitiesMap, timeDiffAB, stats, saveHistos
    );
    allHits.insert(allHits.end(), slotHits.begin(), slotHits.end());
  }
  return allHits;
}

/**
 * Method matching signals on the same Scintillator
 */
vector<JPetHit> HitFinderTools::matchSignals(
  vector<JPetPhysSignal>& slotSignals,
  const map<unsigned int, vector<double>>& velocitiesMap,
  double timeDiffAB, JPetStatistics& stats, bool saveHistos
) {
  vector<JPetHit> slotHits;
  vector<JPetPhysSignal> remainSignals;
  sortByTime(slotSignals);
  while (slotSignals.size() > 0) {
    auto physSig = slotSignals.at(0);
    if(slotSignals.size() == 1){
      remainSignals.push_back(physSig);
      break;
    }
    for (unsigned int j = 1; j < slotSignals.size(); j++) {
      if (slotSignals.at(j).getTime() - physSig.getTime() < timeDiffAB) {
        if (physSig.getPM().getSide() != slotSignals.at(j).getPM().getSide()) {
          auto hit = createHit(
            physSig, slotSignals.at(j), velocitiesMap, stats, saveHistos
          );
          slotHits.push_back(hit);
          slotSignals.erase(slotSignals.begin() + j);
          slotSignals.erase(slotSignals.begin() + 0);
          break;
        } else {
          if (j == slotSignals.size() - 1) {
            remainSignals.push_back(physSig);
            slotSignals.erase(slotSignals.begin() + 0);
            break;
          } else { continue; }
        }
      } else {
        remainSignals.push_back(physSig);
        slotSignals.erase(slotSignals.begin() + 0);
        break;
      }
    }
  }
  if(remainSignals.size()>0 && saveHistos){
    stats.getHisto1D("remain_signals_per_scin")
      ->Fill((float)(remainSignals.at(0).getPM().getScin().getID()), remainSignals.size());
  }
  return slotHits;
}

/**
 * Method for Hit creation - setting all fields, that make sense here
 */
JPetHit HitFinderTools::createHit(
  const JPetPhysSignal& signal1, const JPetPhysSignal& signal2,
  const map<unsigned int, vector<double>>& velocitiesMap,
  JPetStatistics& stats, bool saveHistos
) {
  JPetPhysSignal signalA;
  JPetPhysSignal signalB;
  if (signal1.getPM().getSide() == JPetPM::SideA) {
    signalA = signal1;
    signalB = signal2;
  } else {
    signalA = signal2;
    signalB = signal1;
  }
  auto radius = signalA.getPM().getBarrelSlot().getLayer().getRadius();
  auto theta = TMath::DegToRad() * signalA.getPM().getBarrelSlot().getTheta();
  auto velocity = UniversalFileLoader::getConfigurationParameter(
    velocitiesMap, getProperChannel(signalA)
  );
  checkTheta(theta);

  JPetHit hit;
  hit.setSignalA(signalA);
  hit.setSignalB(signalB);
  hit.setTime((signalA.getTime() + signalB.getTime()) / 2.0);
  hit.setQualityOfTime(-1.0);
  hit.setTimeDiff(signalB.getTime() - signalA.getTime());
  hit.setQualityOfTimeDiff(-1.0);
  hit.setEnergy(-1.0);
  hit.setQualityOfEnergy(-1.0);
  hit.setScintillator(signalA.getPM().getScin());
  hit.setBarrelSlot(signalA.getPM().getBarrelSlot());
  hit.setPosX(radius * cos(theta));
  hit.setPosY(radius * sin(theta));
  hit.setPosZ(velocity * hit.getTimeDiff() / 2000.0);

  if(signalA.getRecoFlag() == JPetBaseSignal::Good
    && signalB.getRecoFlag() == JPetBaseSignal::Good) {
      hit.setRecoFlag(JPetHit::Good);
      if(saveHistos) {
        stats.getHisto1D("good_vs_bad_hits")->Fill(1);
        stats.getHisto2D("time_diff_per_scin")
          ->Fill(hit.getTimeDiff(), (float)(hit.getScintillator().getID()));
        stats.getHisto2D("hit_pos_per_scin")
          ->Fill(hit.getPosZ(), (float)(hit.getScintillator().getID()));
      }
  } else if (signalA.getRecoFlag() == JPetBaseSignal::Corrupted
    || signalB.getRecoFlag() == JPetBaseSignal::Corrupted){
      hit.setRecoFlag(JPetHit::Corrupted);
      if(saveHistos) { stats.getHisto1D("good_vs_bad_hits")->Fill(2); }
  } else {
    hit.setRecoFlag(JPetHit::Unknown);
    if(saveHistos) { stats.getHisto1D("good_vs_bad_hits")->Fill(3); }
  }
  return hit;
}

/**
 * Method for Hit creation in case of reference detector.
 * Setting only some necessary fields.
 */
JPetHit HitFinderTools::createDummyRefDetHit(const JPetPhysSignal& signalB)
{
  JPetHit hit;
  hit.setSignalB(signalB);
  hit.setTime(signalB.getTime());
  hit.setQualityOfTime(-1.0);
  hit.setTimeDiff(0.0);
  hit.setQualityOfTimeDiff(-1.0);
  hit.setEnergy(-1.0);
  hit.setQualityOfEnergy(-1.0);
  hit.setScintillator(signalB.getPM().getScin());
  hit.setBarrelSlot(signalB.getPM().getBarrelSlot());
  return hit;
}

/**
 * Helper method for getting TOMB channel for velocity map
 */
int HitFinderTools::getProperChannel(const JPetPhysSignal& signal)
{
  auto someSigCh = signal.getRecoSignal().getRawSignal()
    .getPoints(JPetSigCh::Leading, JPetRawSignal::ByThrValue)[0];
  return someSigCh.getTOMBChannel().getChannel();
}

/**
* Helper method for checking if theta is in radians
*/
void HitFinderTools::checkTheta(const double& theta)
{
  if (theta > 2 * TMath::Pi()) {
    WARNING("Probably wrong values of Barrel Slot theta - conversion to radians failed. Check please.");
  }
}
