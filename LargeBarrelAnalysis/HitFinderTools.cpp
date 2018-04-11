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
 *  @file HitFinderTools.h
 */

using namespace std;

#include "UniversalFileLoader.h"
#include "HitFinderTools.h"
#include <TMath.h>
#include <vector>
#include <cmath>
#include <map>

/**
 * Method mapping Signals according to Scintillator they belong to
 */
map<int, vector<JPetPhysSignal>> HitFinderTools::getSignalsSlotMap(
  const JPetTimeWindow* timeWindow){

    map<int, vector<JPetPhysSignal>> signalSlotMap;
    if (!timeWindow) {
      WARNING("Pointer of Time Window object is not set, returning empty map");
      return signalSlotMap;
    }

    const unsigned int nSignals = timeWindow->getNumberOfEvents();
    for (unsigned int i = 0; i < nSignals; i++) {
      JPetPhysSignal physSig = dynamic_cast<const JPetPhysSignal&>(timeWindow->operator[](i));
      int scinID = physSig.getPM().getScin().getID();
      auto search = signalSlotMap.find(scinID);
      if (search == signalSlotMap.end()) {
        vector<JPetPhysSignal> tmp;
        tmp.push_back(physSig);
        signalSlotMap.insert(pair<int, vector<JPetPhysSignal>>(scinID, tmp));
      } else {
        search->second.push_back(physSig);
      }
    }
    return signalSlotMap;
}

/**
 * Method matching Signals on the same Scintillator
 */
vector<JPetHit> HitFinderTools::matchSignals(
  JPetStatistics& stats,
  const map<int, vector<JPetPhysSignal>>& signalSlotMap,
  const map<unsigned int, vector<double>>& velocitiesMap,
  double timeDiffAB,
  int refDetScinId,
  bool saveHistos)
{
  vector<JPetHit> allHits;
  vector<JPetPhysSignal> remainSignals;
  for(auto& physSigPair : signalSlotMap) {
    if(physSigPair.first == refDetScinId) {
      for(auto refSignal : physSigPair.second){
        JPetHit refHit = createDummyRefDetHit(refSignal);
        allHits.push_back(refHit);
      }
      continue;
    }
    vector<JPetPhysSignal> sigsVec = physSigPair.second;
    sortByTime(sigsVec);
    while(sigsVec.size()>0){
      if(sigsVec.size()==1){
        if(saveHistos) stats.getHisto1D("remain_signals_per_scin")
          ->Fill((float)(sigsVec.at(0).getPM().getScin().getID()));
        sigsVec.erase(sigsVec.begin()+0);
        break;
      }
      for(uint j=1;j<sigsVec.size();j++){
        if(sigsVec.at(j).getTime()-sigsVec.at(0).getTime()<timeDiffAB){
          if(sigsVec.at(0).getPM().getSide()!=sigsVec.at(j).getPM().getSide()){
            JPetHit hit = createHit(sigsVec.at(0),sigsVec.at(j),velocitiesMap);
            allHits.push_back(hit);
            if(saveHistos){
              stats.getHisto2D("time_diff_per_scin")->Fill(
                hit.getTimeDiff(), (float)(hit.getScintillator().getID()));
              stats.getHisto2D("hit_pos_per_scin")->Fill(
                hit.getPosZ(), (float)(hit.getScintillator().getID()));
            }
            sigsVec.erase(sigsVec.begin()+j);
            sigsVec.erase(sigsVec.begin()+0);
            break;
          }else{
            if(j==sigsVec.size()-1){
              sigsVec.erase(sigsVec.begin()+0);
              break;
            } else continue;
          }
        }else{
          if(saveHistos) stats.getHisto1D("remain_signals_per_scin")
            ->Fill((float)(sigsVec.at(0).getPM().getScin().getID()));
          sigsVec.erase(sigsVec.begin()+0);
          break;
        }
      }
    }
  }
  return allHits;
}

/**
 * Method for Hit creation - setting all fields, that make sense here
 */
JPetHit HitFinderTools::createHit(
  const JPetPhysSignal& signal1,
  const JPetPhysSignal& signal2,
  const map<unsigned int, vector<double>>& velocitiesMap)
{
  JPetPhysSignal signalA;
  JPetPhysSignal signalB;
  if(signal1.getPM().getSide()==JPetPM::SideA){
    signalA = signal1;
    signalB = signal2;
  } else {
    signalA = signal2;
    signalB = signal1;
  }
  auto radius = signalA.getPM().getBarrelSlot().getLayer().getRadius();
  auto theta = TMath::DegToRad() * signalA.getPM().getBarrelSlot().getTheta();
  auto velocity = UniversalFileLoader::getConfigurationParameter(
    velocitiesMap, getProperChannel(signalA));
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
  return hit;
}

/**
 * Method for Hit creation in case of reference detector
 * - setting only some necessary fields
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
 * Helper method for checking if theta is in radians
 */
void HitFinderTools::checkTheta(const double& theta)
{
  if (theta > 2*TMath::Pi())
    WARNING("Probably wrong values of Barrel Slot theta - conversion to radians failed. Check please.");
}

/**
 * Helper method for getting TOMB channel for velocity map
 */
int HitFinderTools::getProperChannel(const JPetPhysSignal& signal){
  JPetSigCh someSigCh = signal.getRecoSignal().getRawSignal()
    .getPoints(JPetSigCh::Leading, JPetRawSignal::ByThrNum)[0];
  return someSigCh.getTOMBChannel().getChannel();
}

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
