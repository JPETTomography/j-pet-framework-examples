/**
 *  @copyright Copyright 2021 The J-PET Framework Authors. All rights reserved.
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
#include <cmath>
#include <map>
#include <vector>

/**
 * Helper method for sotring signals in vector
 */
void HitFinderTools::sortByTime(vector<JPetMatrixSignal>& sigVec)
{
  sort(sigVec.begin(), sigVec.end(), [](const JPetMatrixSignal& sig1, const JPetMatrixSignal& sig2) { return sig1.getTime() < sig2.getTime(); });
}

/**
 * Helper method for sotring hits in vector
 */
void HitFinderTools::sortByTime(std::vector<JPetPhysRecoHit>& hitsVec)
{
  sort(hitsVec.begin(), hitsVec.end(), [](const JPetPhysRecoHit& hit1, const JPetPhysRecoHit& hit2) { return hit1.getTime() < hit2.getTime(); });
}

/**
 * Method distributing Signals according to Scintillator they belong to
 */
map<int, vector<JPetMatrixSignal>> HitFinderTools::getSignalsByScin(const JPetTimeWindow* timeWindow)
{
  map<int, vector<JPetMatrixSignal>> signalScinMap;
  if (!timeWindow)
  {
    WARNING("Pointer of Time Window object is not set, returning empty map");
    return signalScinMap;
  }
  const unsigned int nSignals = timeWindow->getNumberOfEvents();
  for (unsigned int i = 0; i < nSignals; i++)
  {
    auto mtxSig = dynamic_cast<const JPetMatrixSignal&>(timeWindow->operator[](i));
    int scinID = mtxSig.getMatrix().getScin().getID();
    auto search = signalScinMap.find(scinID);
    if (search == signalScinMap.end())
    {
      vector<JPetMatrixSignal> tmp;
      tmp.push_back(mtxSig);
      signalScinMap.insert(pair<int, vector<JPetMatrixSignal>>(scinID, tmp));
    }
    else
    {
      search->second.push_back(mtxSig);
    }
  }
  return signalScinMap;
}

/**
 * Loop over all Scins invoking matching procedure
 */
vector<JPetPhysRecoHit> HitFinderTools::matchAllSignals(map<int, vector<JPetMatrixSignal>>& allSignals, double timeDiffAB, int refScinID,
                                                        int refSlotID, boost::property_tree::ptree& calibTree, JPetStatistics& stats, bool saveHistos)
{
  vector<JPetPhysRecoHit> allHits;
  for (auto& scinSignals : allSignals)
  {
    // Match signals for scintillators
    auto scinHits = matchSignals(scinSignals.second, timeDiffAB, refScinID, refSlotID, calibTree, stats, saveHistos);
    allHits.insert(allHits.end(), scinHits.begin(), scinHits.end());
  }
  return allHits;
}

/**
 * Method matching signals on the same Scintillator
 */
vector<JPetPhysRecoHit> HitFinderTools::matchSignals(vector<JPetMatrixSignal>& scinSignals, double timeDiffAB, int refScinID, int refSlotID,
                                                     boost::property_tree::ptree& calibTree, JPetStatistics& stats, bool saveHistos)
{
  vector<JPetPhysRecoHit> scinHits;
  vector<JPetMatrixSignal> remainSignals;
  sortByTime(scinSignals);

  while (scinSignals.size() > 0)
  {
    auto mtxSig = scinSignals.at(0);

    // Handling signals from scin/slot used as a reference detector
    if (mtxSig.getMatrix().getScin().getID() == refScinID || mtxSig.getMatrix().getScin().getSlot().getID() == refSlotID)
    {
      auto refHit = createDummyHit(mtxSig);
      scinHits.push_back(refHit);
      scinSignals.erase(scinSignals.begin() + 0);
      continue;
    }

    if (scinSignals.size() == 1)
    {
      remainSignals.push_back(mtxSig);
      break;
    }

    for (unsigned int j = 1; j < scinSignals.size(); j++)
    {
      if (scinSignals.at(j).getTime() - mtxSig.getTime() < timeDiffAB)
      {
        if (mtxSig.getMatrix().getSide() != scinSignals.at(j).getMatrix().getSide())
        {
          auto hit = createHit(mtxSig, scinSignals.at(j), calibTree);
          scinHits.push_back(hit);
          scinSignals.erase(scinSignals.begin() + j);
          scinSignals.erase(scinSignals.begin() + 0);
          break;
        }
        else
        {
          if (j == scinSignals.size() - 1)
          {
            remainSignals.push_back(mtxSig);
            scinSignals.erase(scinSignals.begin() + 0);
            break;
          }
          else
          {
            continue;
          }
        }
      }
      else
      {
        if (saveHistos && mtxSig.getMatrix().getSide() != scinSignals.at(j).getMatrix().getSide())
        {
          stats.fillHistogram("remain_signals_tdiff", scinSignals.at(j).getTime() - mtxSig.getTime());
        }
        remainSignals.push_back(mtxSig);
        scinSignals.erase(scinSignals.begin() + 0);
        break;
      }
    }
  }
  if (remainSignals.size() > 0 && saveHistos)
  {
    stats.fillHistogram("remain_signals_scin", (double)(remainSignals.at(0).getMatrix().getScin().getID()), remainSignals.size());
  }
  return scinHits;
}

/**
 * Method for Hit creation - setting all fields that make sense here
 */
JPetPhysRecoHit HitFinderTools::createHit(const JPetMatrixSignal& signal1, const JPetMatrixSignal& signal2, boost::property_tree::ptree& calibTree)
{
  JPetMatrixSignal signalA;
  JPetMatrixSignal signalB;

  if (signal1.getMatrix().getSide() == JPetMatrix::SideA)
  {
    signalA = signal1;
    signalB = signal2;
  }
  else
  {
    signalA = signal2;
    signalB = signal1;
  }

  auto& scin = signalA.getMatrix().getScin();

  // Getting constants for this scintillator
  double tofCorrection = calibTree.get("scin." + to_string(scin.getID()) + ".tof_correction", 0.0);
  double tofCorrection2 = calibTree.get("scin." + to_string(scin.getID()) + ".tof_correction_2", 0.0);
  double velocity = calibTree.get("scin." + to_string(scin.getID()) + ".eff_velocity", 0.0);

  JPetPhysRecoHit hit;
  hit.setSignals(signalA, signalB);
  hit.setScin(scin);

  hit.setTime(((signalA.getTime() + signalB.getTime()) / 2.0) - tofCorrection - tofCorrection2);
  hit.setTimeDiff(signalB.getTime() - signalA.getTime());

  // ToT of a signal is a average of ToT of AB signals
  double tot = (signalA.getToT() + signalB.getToT()) / 2.0;
  double totNormA = calibTree.get("scin." + to_string(scin.getID()) + ".tot_factor_a", 1.0);
  double totNormB = calibTree.get("scin." + to_string(scin.getID()) + ".tot_factor_b", 0.0);
  hit.setToT(tot * totNormA + totNormB);

  TVector3 position(scin.getCenterX(), scin.getCenterY(), velocity * hit.getTimeDiff() / 2.0);
  // Rotation of position vector according to configuration settings
  // Converting value from file in degrees to radians
  position.RotateX(TMath::DegToRad() * scin.getRotationX());
  position.RotateY(TMath::DegToRad() * scin.getRotationY());
  position.RotateZ(TMath::DegToRad() * scin.getRotationZ());

  // Setting position
  hit.setPos(position);

  hit.setEnergy(0.0);
  // Default quality fields
  hit.setQualityOfTime(-1.0);
  hit.setQualityOfTimeDiff(-1.0);
  hit.setQualityOfEnergy(-1.0);
  hit.setQualityOfToT(-1.0);

  return hit;
}

/**
 * Method for a dummy Hit creation, setting only some necessary fields.
 */
JPetPhysRecoHit HitFinderTools::createDummyHit(const JPetMatrixSignal& signal)
{
  JPetPhysRecoHit hit;
  JPetMatrixSignal dummy;
  // hit.setSignalA(dummy);
  hit.setSignalB(signal);
  hit.setTime(signal.getTime());
  hit.setTimeDiff(0.0);
  hit.setEnergy(0.0);
  hit.setToT(signal.getToT());

  auto& scin = signal.getMatrix().getScin();
  hit.setPosX(scin.getCenterX());
  hit.setPosY(scin.getCenterY());
  hit.setPosZ(scin.getCenterZ());
  hit.setScin(scin);

  return hit;
}
