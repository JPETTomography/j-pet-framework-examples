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
 * Method for Hit creation - setting all fields that make sense here
 */
vector<JPetPhysRecoHit> RedModuleHitFinderTools::matchHitsWithWLSSignals(
    const vector<JPetPhysRecoHit>& scinHits, const map<int, std::vector<JPetMatrixSignal>>& wlsSignalsMap, double maxTimeDiffWLS,
    double timeDiffOffset, boost::property_tree::ptree& calibTree, boost::property_tree::ptree& wlsConfig, JPetStatistics& stats, bool saveHistos)
{
  vector<JPetPhysRecoHit> wlsHits;

  // Iterating hit and WLS signals for time coincidences
  for (unsigned int hit_i = 0; hit_i < scinHits.size(); ++hit_i)
  {
    for (auto& wlsSignals : wlsSignalsMap)
    {
      for (unsigned int wls_i = 0; wls_i < wlsSignals.second.size(); ++wls_i)
      {
        auto& scinHit = scinHits.at(hit_i);
        auto& wlsSignal = wlsSignals.second.at(wls_i);
        auto scinID = scinHit.getScin().getID();
        auto wlsID = wlsSignal.getMatrix().getScin().getID();
        double wlsScinOffset = calibTree.get("wls." + to_string(wlsID) + ".scin." + to_string(scinID), 0.0);

        // The time difference between scintilator hits and signals in WLS is artificially streached
        // to some constant value (i.e. 10 ns), so this value and the calibration constant are now
        // substracted from the registered times
        double timeDiff = scinHit.getTime() - wlsSignal.getTime() - wlsScinOffset;
        if (timeDiff - timeDiffOffset < maxTimeDiffWLS)
        {
          auto wlsHit = createWLSHit(scinHit, wlsSignal, calibTree, wlsConfig);
          wlsHits.push_back(wlsHit);
          if (saveHistos)
          {
            stats.fillHistogram("hit_scin_wls_tdiff", scinID, wlsID, timeDiff);
          }
        }
      }
    }
  }

  return wlsHits;
}

/**
 * Method for Hit creation - setting all fields that make sense here
 */
JPetPhysRecoHit RedModuleHitFinderTools::createWLSHit(const JPetPhysRecoHit& scinHit, const JPetMatrixSignal& wlsSignal,
                                                      boost::property_tree::ptree& calibTree, boost::property_tree::ptree& wlsConfig)
{
  JPetPhysRecoHit wlsHit;
  wlsHit.setSignals(scinHit.getSignalA(), scinHit.getSignalB());
  wlsHit.setTime(scinHit.getTime());
  wlsHit.setTimeDiff(scinHit.getTime() - wlsSignal.getTime());
  wlsHit.setToT(scinHit.getToT());

  auto& scin = wlsSignal.getMatrix().getScin();
  wlsHit.setScin(scin);

  double x_pos =
      scin.getCenterX() + (scin.getSlot().getLayer().getRadius() + scin.getWidth() / 2.0) * TMath::Sin(TMath::DegToRad() * scin.getSlot().getTheta());
  double y_pos = scin.getCenterY();

  // Estimation of the z-axis position can be done with scintillator hit position
  // or with WLS position weighted by ToT of SiPM signals
  // double z_pos = scinHit.getPosZ();
  double z_pos = estimateZPosWithWLS(wlsSignal, wlsConfig);
  if (z_pos == 0.0)
  {
    z_pos = scinHit.getPosZ();
  }

  TVector3 position(x_pos, y_pos, z_pos);

  // Rotation of position vector according to configuration settings
  // Converting value from file in degrees to radians
  position.RotateX(TMath::DegToRad() * scin.getRotationX());
  position.RotateY(TMath::DegToRad() * scin.getRotationY());
  position.RotateZ(TMath::DegToRad() * scin.getRotationZ());

  // Setting position
  wlsHit.setPos(position);

  wlsHit.setEnergy(0.0);
  // Default quality fields
  wlsHit.setQualityOfTime(-1.0);
  wlsHit.setQualityOfTimeDiff(-1.0);
  wlsHit.setQualityOfEnergy(-1.0);
  wlsHit.setQualityOfToT(-1.0);

  return wlsHit;
}

double RedModuleHitFinderTools::estimateZPosWithWLS(const JPetMatrixSignal& wlsSignal, boost::property_tree::ptree& wlsConfig)
{
  double z_pos = 0.0;
  double totAll = 0.0;

  for (auto sigEl : wlsSignal.getPMSignals())
  {
    totAll += sigEl.second.getToT();
  }

  if (totAll != 0.0)
  {
    for (auto sigEl : wlsSignal.getPMSignals())
    {
      auto sipmID = sigEl.second.getPM().getID();
      double z_i = wlsConfig.get("sipm.zcenter." + to_string(sipmID), 0.0);
      double tot_i = sigEl.second.getToT();
      z_pos += z_i * (tot_i / totAll);
    }
  }

  return z_pos;
}
