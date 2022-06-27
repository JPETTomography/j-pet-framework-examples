/**
 *  @copyright Copyright 2022 The J-PET Framework Authors. All rights reserved.
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
 *  @file RedModuleHitFinderTools.cpp
 */

using namespace std;

#include "RedModuleHitFinderTools.h"
#include "../ModularDetectorAnalysis/HitFinderTools.h"
#include <TMath.h>
#include <cmath>
#include <map>
#include <vector>

/**
 * Loop over all Scins invoking matching procedure
 */
vector<JPetPhysRecoHit> RedModuleHitFinderTools::matchAllSignals(map<int, vector<JPetMatrixSignal>>& allSignals, double timeDiffAB, int refScinID,
                                                                 int refSlotID, boost::property_tree::ptree& calibTree,
                                                                 boost::property_tree::ptree& wlsConfig, JPetStatistics& stats, bool saveHistos)
{
  vector<JPetPhysRecoHit> allHits;
  for (auto& scinSignals : allSignals)
  {
    JPetSlot::Type slotType = scinSignals.second.at(0).getMatrix().getScin().getSlot().getType();
    if (slotType == JPetSlot::Module)
    {
      // Match signals for scintillators
      auto scinHits = matchSignals(scinSignals.second, timeDiffAB, refScinID, refSlotID, calibTree, stats, saveHistos);
      allHits.insert(allHits.end(), scinHits.begin(), scinHits.end());
    }
    else if (slotType == JPetSlot::WLS)
    {
      for (auto signal : scinSignals.second)
      {
        auto wlsHit = createWLSHit(signal, calibTree, wlsConfig);
        allHits.push_back(wlsHit);
      }
    }
  }
  return allHits;
}

/**
 * Method matching signals on the same Scintillator
 */
vector<JPetPhysRecoHit> RedModuleHitFinderTools::matchSignals(vector<JPetMatrixSignal>& scinSignals, double timeDiffAB, int refScinID, int refSlotID,
                                                              boost::property_tree::ptree& calibTree, JPetStatistics& stats, bool saveHistos)
{
  vector<JPetPhysRecoHit> scinHits;
  vector<JPetMatrixSignal> remainSignals;
  HitFinderTools::sortByTime(scinSignals);

  while (scinSignals.size() > 0)
  {
    auto mtxSig = scinSignals.at(0);

    if (mtxSig.getMatrix().getScin().getID() == refScinID || mtxSig.getMatrix().getScin().getSlot().getID() == refSlotID)
    {
      double correction = calibTree.get("scin." + to_string(mtxSig.getMatrix().getScin().getID()) + ".b_correction", 0.0);
      scinHits.push_back(HitFinderTools::createDummyHit(mtxSig, correction));
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
          auto hit = createRedModuleHit(mtxSig, scinSignals.at(j), calibTree);
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
JPetPhysRecoHit RedModuleHitFinderTools::createRedModuleHit(const JPetMatrixSignal& signal1, const JPetMatrixSignal& signal2,
                                                            boost::property_tree::ptree& calibTree)
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
  double velocity = calibTree.get("scin." + to_string(scin.getID()) + ".eff_velocity", 0.0);

  JPetPhysRecoHit hit;
  hit.setSignals(signalA, signalB);
  hit.setScin(scin);

  hit.setTime(((signalA.getTime() + signalB.getTime()) / 2.0) - tofCorrection);
  hit.setTimeDiff(signalB.getTime() - signalA.getTime());

  // ToT of a signal is a average of ToT of AB signals
  double tot = (signalA.getToT() + signalB.getToT()) / 2.0;
  double totNormA = calibTree.get("scin." + to_string(scin.getID()) + ".tot_factor_a", 1.0);
  double totNormB = calibTree.get("scin." + to_string(scin.getID()) + ".tot_factor_b", 0.0);
  hit.setToT(tot * totNormA + totNormB);

  double x_pos = (scin.getSlot().getLayer().getRadius() + scin.getWidth() / 2.0) * TMath::Sin(TMath::DegToRad() * scin.getSlot().getTheta());
  double y_pos = scin.getCenterY();
  double z_pos = velocity * hit.getTimeDiff() / 2.0;
  TVector3 position(x_pos, y_pos, z_pos);

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

JPetPhysRecoHit RedModuleHitFinderTools::createWLSHit(const JPetMatrixSignal& signal, boost::property_tree::ptree& calibTree,
                                                      boost::property_tree::ptree& wlsConfig)
{
  JPetPhysRecoHit hit;
  hit.setSignalA(signal);
  hit.setTime(signal.getTime());
  hit.setTimeDiff(0.0);
  hit.setEnergy(0.0);
  hit.setToT(signal.getToT());

  // Using WLS configuration file to assign z axis position
  // TODO assigning to correct WLS
  auto pmID = signal.getPMSignals().at(0).getPM().getID();
  double z_pos = wlsConfig.get("pm.zcenter." + to_string(pmID), -999.0);

  auto& scin = signal.getMatrix().getScin();
  hit.setPosX(scin.getCenterX());
  hit.setPosY(scin.getCenterY());
  hit.setPosZ(z_pos);
  hit.setScin(scin);

  return hit;
}
