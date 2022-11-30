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
vector<JPetPhysRecoHit> RedModuleHitFinderTools::matchHitsWithWLSSignals(const vector<JPetPhysRecoHit>& scinHits,
                                                                         const map<int, std::vector<JPetMatrixSignal>>& wlsSignalsMap,
                                                                         double maxTimeDiffWLS, double timeDiffOffset,
                                                                         boost::property_tree::ptree& calibTree,
                                                                         boost::property_tree::ptree& wlsConfig, JPetStatistics& stats,
                                                                         bool saveHistos, double totCutAnniMin, double totCutAnniMax)
{
  vector<JPetPhysRecoHit> wlsHits;

  // Iterating hit and WLS signals for time coincidences
  for (unsigned int hit_i = 0; hit_i < scinHits.size(); ++hit_i)
  {
    int hitWLSMulti = 0;
    double wlsToTSum = 0.0;

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
        if (fabs(timeDiff - timeDiffOffset) < maxTimeDiffWLS)
        {
          auto wlsHit = createWLSHit(scinHit, wlsSignal, calibTree, wlsConfig, stats, saveHistos, totCutAnniMin, totCutAnniMax);
          wlsHits.push_back(wlsHit);
          hitWLSMulti++;
          if (saveHistos)
          {
            stats.fillHistogram("hit_tot_wls_scin", wlsHit.getToT(), wlsSignal.getToT());

            stats.fillHistogram("hit_scin_wls_tdiff", scinID, wlsID, timeDiff);

            auto zdiff = scinHit.getPosZ() - wlsHit.getPosZ();
            if (zdiff != 0.0)
            {
              stats.fillHistogram("hit_scin_wls_zdiff", scinID, wlsID, zdiff);
            }
          }
        }
      }
    }

    if (saveHistos)
    {
      stats.fillHistogram("hit_scin_wls_multi", scinHits.at(hit_i).getScin().getID(), hitWLSMulti);
      stats.fillHistogram("hit_scin_wls_multi_tot", hitWLSMulti, scinHits.at(hit_i).getToT());
    }
  }

  return wlsHits;
}

/**
 * Method for Hit creation - setting all fields that make sense here
 */
JPetPhysRecoHit RedModuleHitFinderTools::createWLSHit(const JPetPhysRecoHit& scinHit, const JPetMatrixSignal& wlsSignal,
                                                      boost::property_tree::ptree& calibTree, boost::property_tree::ptree& wlsConfig,
                                                      JPetStatistics& stats, bool saveHistos, double totCutAnniMin, double totCutAnniMax)
{
  JPetPhysRecoHit wlsHit;
  wlsHit.setSignals(scinHit.getSignalA(), scinHit.getSignalB());
  wlsHit.setTime(scinHit.getTime());
  wlsHit.setTimeDiff(scinHit.getTime() - wlsSignal.getTime());
  wlsHit.setToT(scinHit.getToT());
  // Temp. solution, save WLS signal ToT as energy
  wlsHit.setEnergy(wlsSignal.getToT());

  auto& scin = wlsSignal.getMatrix().getScin();
  wlsHit.setScin(scin);

  double x_pos = scinHit.getScin().getCenterX();
  double y_pos = scinHit.getScin().getCenterY();

  // Estimation of the z-axis position can be done with scintillator hit position
  // or with WLS position weighted by ToT of SiPM signals
  // double z_pos = scinHit.getPosZ();
  double z_pos = estimateZPosWithWLS(wlsSignal, wlsConfig);
  if (z_pos == 0.0)
  {
    z_pos = scinHit.getPosZ();
  }

  // Comparing various attempts of estimating of z position
  if (saveHistos)
  {
    auto wlsID = wlsSignal.getMatrix().getScin().getID();
    stats.fillHistogram("wls_z_pos_1_met0", wlsID, zPosMethod0(wlsSignal, wlsConfig));
    stats.fillHistogram("wls_z_pos_1_met1", wlsID, zPosMethod1(wlsSignal, wlsConfig));
    stats.fillHistogram("wls_z_pos_1_met2", wlsID, zPosMethod2(wlsSignal, wlsConfig));
    stats.fillHistogram("wls_z_pos_2_met0", wlsID, zPosMethod0(wlsSignal, wlsConfig));
    stats.fillHistogram("wls_z_pos_2_met1", wlsID, zPosMethod1(wlsSignal, wlsConfig));
    stats.fillHistogram("wls_z_pos_2_met2", wlsID, zPosMethod2(wlsSignal, wlsConfig));

    if (scinHit.getToT() > totCutAnniMin && scinHit.getToT() < totCutAnniMax)
    {
      stats.fillHistogram("wls_z_pos_1_met0_cut", wlsID, zPosMethod0(wlsSignal, wlsConfig));
      stats.fillHistogram("wls_z_pos_1_met1_cut", wlsID, zPosMethod1(wlsSignal, wlsConfig));
      stats.fillHistogram("wls_z_pos_1_met2_cut", wlsID, zPosMethod2(wlsSignal, wlsConfig));
      stats.fillHistogram("wls_z_pos_2_met0", wlsID, zPosMethod0(wlsSignal, wlsConfig));
      stats.fillHistogram("wls_z_pos_2_met1", wlsID, zPosMethod1(wlsSignal, wlsConfig));
      stats.fillHistogram("wls_z_pos_2_met2", wlsID, zPosMethod2(wlsSignal, wlsConfig));
    }
  }

  TVector3 position(x_pos, y_pos, z_pos);

  // Rotation of position vector according to configuration settings
  // Converting value from file in degrees to radians
  position.RotateX(TMath::DegToRad() * scin.getRotationX());
  position.RotateY(TMath::DegToRad() * scin.getRotationY());
  position.RotateZ(TMath::DegToRad() * scin.getRotationZ());

  // Setting position
  wlsHit.setPos(position);

  // Temp. solution to store z position of the hit in the strip
  wlsHit.setQualityOfTime(scinHit.getPosZ());
  // Here write difference of the positions estimated by tDiff/velocity and WLS/Sipm
  wlsHit.setQualityOfTimeDiff(scinHit.getPosZ() - z_pos);

  // Other default quality fields
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

// The same as above with division by wieght sum (should be equal to 1)
double RedModuleHitFinderTools::zPosMethod0(const JPetMatrixSignal& wlsSignal, boost::property_tree::ptree& wlsConfig)
{
  double z_pos = 0.0;
  double w_all = 0.0;
  double tot_all = 0.0;

  for (auto sigEl : wlsSignal.getPMSignals())
  {
    tot_all += sigEl.second.getToT();
  }

  if (tot_all != 0.0)
  {
    for (auto sigEl : wlsSignal.getPMSignals())
    {
      auto sipmID = sigEl.second.getPM().getID();
      double z_i = wlsConfig.get("sipm.zcenter." + to_string(sipmID), 0.0);
      double tot_i = sigEl.second.getToT();

      double w_i = (tot_i / tot_all);
      w_all += w_i;
      z_pos += z_i * w_i;
    }
  }

  if (w_all != 0.0)
  {
    return z_pos / w_all;
  }
  else
  {
    return 0.0;
  }
}

// Estimating the position with wieghts squared
double RedModuleHitFinderTools::zPosMethod1(const JPetMatrixSignal& wlsSignal, boost::property_tree::ptree& wlsConfig)
{
  double z_pos = 0.0;
  double w_all = 0.0;
  double tot_all = 0.0;

  for (auto sigEl : wlsSignal.getPMSignals())
  {
    tot_all += sigEl.second.getToT();
  }

  if (tot_all != 0.0)
  {
    for (auto sigEl : wlsSignal.getPMSignals())
    {
      auto sipmID = sigEl.second.getPM().getID();
      double z_i = wlsConfig.get("sipm.zcenter." + to_string(sipmID), 0.0);
      double tot_i = sigEl.second.getToT();

      double w_i = (tot_i / tot_all);
      w_all += w_i * w_i;
      z_pos += z_i * w_i * w_i;
    }
  }

  if (w_all != 0.0)
  {
    return z_pos / w_all;
  }
  else
  {
    return 0.0;
  }
}

// Including an additional percentage from the WLS config of the surface covered by SiPM
double RedModuleHitFinderTools::zPosMethod2(const JPetMatrixSignal& wlsSignal, boost::property_tree::ptree& wlsConfig)
{
  auto wlsID = wlsSignal.getMatrix().getID();
  double z_pos = 0.0;
  double w_all = 0.0;
  double tot_all = 0.0;

  for (auto sigEl : wlsSignal.getPMSignals())
  {
    tot_all += sigEl.second.getToT();
  }

  if (tot_all != 0.0)
  {
    for (auto sigEl : wlsSignal.getPMSignals())
    {
      auto sipmID = sigEl.second.getPM().getID();
      double z_i = wlsConfig.get("sipm.zcenter." + to_string(sipmID), 0.0);
      double cover = wlsConfig.get("wls_matrix." + to_string(wlsID) + ".sipm_coverage." + to_string(sipmID), 0.0);
      double tot_i = sigEl.second.getToT();

      double w_i = cover * (tot_i / tot_all);
      w_all += w_i;
      z_pos += z_i * w_i;
    }
  }

  if (w_all != 0.0)
  {
    return z_pos / w_all;
  }
  else
  {
    return 0.0;
  }
}
