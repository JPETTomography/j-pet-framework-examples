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
 *  @file CalibrationTools.cpp
 */

using namespace std;

#include "CalibrationTools.h"
#include "EventCategorizerTools.h"

/**
 * Selecting pair of hits for calibrations based on ToT and Scin ID with fitted scatter test
 */
void CalibrationTools::selectForTOF(const JPetEvent& event, JPetStatistics& stats, bool saveCalibHistos, double totCutAnniMin, double totCutAnniMax,
                                    double totCutDeexMin, double totCutDeexMax, EventCategorizerTools::ScatterTestType testType,
                                    double scatterTestValue, double scatterTimeMin, double scatterTimeMax, double scatterAngleMin,
                                    double scatterAngleMax, boost::property_tree::ptree& calibTree)
{
  if (event.getHits().size() < 2)
  {
    return;
  }

  for (uint i = 0; i < event.getHits().size(); i++)
  {
    auto hit1 = dynamic_cast<const JPetPhysRecoHit*>(event.getHits().at(i));
    if (!hit1)
    {
      continue;
    }

    for (uint j = i + 1; j < event.getHits().size(); j++)
    {
      auto hit2 = dynamic_cast<const JPetPhysRecoHit*>(event.getHits().at(j));
      if (!hit2)
      {
        continue;
      }

      // Skip if scatter
      if (EventCategorizerTools::checkForScatter(hit1, hit2, stats, false, testType, scatterTestValue, scatterTimeMin, scatterTimeMax,
                                                 scatterAngleMin, scatterAngleMax, calibTree))
      {
        continue;
      }

      auto tot1 = hit1->getToT();
      auto tot2 = hit2->getToT();

      // Checking ToT of hits to classify them as annihilation or deexcitation
      bool anih1 = (tot1 > totCutAnniMin && tot1 < totCutAnniMax);
      bool anih2 = (tot2 > totCutAnniMin && tot2 < totCutAnniMax);
      bool deex1 = (tot1 > totCutDeexMin && tot1 < totCutDeexMax);
      bool deex2 = (tot2 > totCutDeexMin && tot2 < totCutDeexMax);

      // Time differences and strip ID to be assigned
      double aTime = 0.0, dTime = 0.0;
      int aScinID = -1, dScinID = -1;
      TVector3 posA, posD;

      if (anih1 && deex2)
      {
        aScinID = hit1->getScin().getID();
        dScinID = hit2->getScin().getID();
        aTime = hit1->getTime();
        dTime = hit2->getTime();
        posA = hit1->getPos();
        posD = hit2->getPos();
      }
      else if (anih2 && deex1)
      {
        aScinID = hit2->getScin().getID();
        dScinID = hit1->getScin().getID();
        aTime = hit2->getTime();
        dTime = hit1->getTime();
        posA = hit2->getPos();
        posD = hit1->getPos();
      }
      else
      {
        continue;
      }

      double tdiff = aTime - dTime;

      // Filling histograms for specific scintillators
      if (saveCalibHistos && aScinID != -1 && dScinID != -1)
      {
        stats.fillHistogram("tdiff_anni_scin", aScinID, tdiff);
        stats.fillHistogram("tdiff_deex_scin", dScinID, tdiff);
      }
    }
  }
}

void CalibrationTools::selectForTimeWalk(const JPetEvent& event, JPetStatistics& stats, bool saveCalibHistos, double maxThetaDiff, double maxTimeDiff,
                                         double totCutAnniMin, double totCutAnniMax, const TVector3& sourcePos,
                                         EventCategorizerTools::ScatterTestType testType, double scatterTestValue, double scatterTimeMin,
                                         double scatterTimeMax, double scatterAngleMin, double scatterAngleMax,
                                         boost::property_tree::ptree& calibTree)
{
  if (event.getHits().size() < 2)
  {
    return;
  }

  for (uint i = 0; i < event.getHits().size(); i++)
  {
    for (uint j = i + 1; j < event.getHits().size(); j++)
    {
      auto firstHit = dynamic_cast<const JPetPhysRecoHit*>(event.getHits().at(i));
      auto secondHit = dynamic_cast<const JPetPhysRecoHit*>(event.getHits().at(j));

      // Stop if cast hits are null pointer
      if (!firstHit || !secondHit)
      {
        break;
      }

      // Skip if scatter
      if (EventCategorizerTools::checkForScatter(firstHit, secondHit, stats, false, testType, scatterTestValue, scatterTimeMin, scatterTimeMax,
                                                 scatterAngleMin, scatterAngleMax, calibTree))
      {
        continue;
      }

      auto test1 = EventCategorizerTools::checkToT(firstHit, totCutAnniMin, totCutAnniMax);
      auto test2 = EventCategorizerTools::checkToT(secondHit, totCutAnniMin, totCutAnniMax);
      auto test3 = EventCategorizerTools::checkRelativeAngles(firstHit->getPos(), secondHit->getPos(), maxThetaDiff);

      if (test1 && test2 && test3)
      {
        // Calculating reversed ToT for time walk studies
        auto revToT1 = calculateReveresedToT(firstHit);
        auto revToT2 = calculateReveresedToT(secondHit);

        stats.fillHistogram("time_walk_ab_tdiff", firstHit->getTimeDiff(), revToT1);
        stats.fillHistogram("time_walk_ab_tdiff", secondHit->getTimeDiff(), revToT2);
        stats.fillHistogram("time_walk_ab_tdiff_scin", firstHit->getTimeDiff(), revToT1, firstHit->getScin().getID());
        stats.fillHistogram("time_walk_ab_tdiff_scin", secondHit->getTimeDiff(), revToT2, secondHit->getScin().getID());

        if (firstHit->getScin().getSlot().getTheta() < secondHit->getScin().getSlot().getTheta())
        {
          stats.fillHistogram("time_walk_tof", EventCategorizerTools::calculateTOF(firstHit, secondHit), revToT1 - revToT2);
          stats.fillHistogram("time_walk_ab_tdiff_scin", EventCategorizerTools::calculateTOF(firstHit, secondHit), revToT1 - revToT2,
                              firstHit->getScin().getID());
          stats.fillHistogram("time_walk_ab_tdiff_scin", EventCategorizerTools::calculateTOF(firstHit, secondHit), revToT1 - revToT2,
                              secondHit->getScin().getID());
        }
        else
        {
          stats.fillHistogram("time_walk_tof_scin", EventCategorizerTools::calculateTOF(secondHit, firstHit), revToT2 - revToT1,
                              firstHit->getScin().getID());
          stats.fillHistogram("time_walk_tof_scin", EventCategorizerTools::calculateTOF(secondHit, firstHit), revToT2 - revToT1,
                              secondHit->getScin().getID());
        }

        vector<JPetMatrixSignal> mtxSigs;
        mtxSigs.push_back(firstHit->getSignalA());
        mtxSigs.push_back(firstHit->getSignalB());
        mtxSigs.push_back(secondHit->getSignalA());
        mtxSigs.push_back(secondHit->getSignalB());

        for (auto& mtxSig : mtxSigs)
        {
          auto sigMap = mtxSig.getPMSignals();
          if (sigMap.find(1) != sigMap.end())
          {
            auto t_1_1 = sigMap.at(1).getLeadTrailPairs().at(0).first.getTime();
            for (auto pmSig : sigMap)
            {
              auto pairs = pmSig.second.getLeadTrailPairs();
              for (auto pair : pairs)
              {
                auto t_ch_i = pair.first.getTime();
                auto channelID = pair.first.getChannel().getID();
                if (t_1_1 == t_ch_i)
                {
                  continue;
                }
                stats.fillHistogram("evtcat_channel_offsets", channelID, t_ch_i - t_1_1);
              }
            }
          }
        }
      }
    }
  }
}

/**
 * @brief Tools for managing calibrations
 */
void CalibrationTools::selectCosmicsForToF(const JPetEvent& event, JPetStatistics& stats, bool saveCalibHistos, double maxThetaDiff,
                                           double detectorYRot)
{
  if (event.getHits().size() < 2)
  {
    return;
  }

  for (uint i = 0; i < event.getHits().size(); i++)
  {
    for (uint j = i + 1; j < event.getHits().size(); j++)
    {
      auto hit1 = dynamic_cast<const JPetPhysRecoHit*>(event.getHits().at(i));
      auto hit2 = dynamic_cast<const JPetPhysRecoHit*>(event.getHits().at(j));

      // Check if hits are from different layers and assign them as upper or lower hit
      auto layer1ID = hit1->getScin().getSlot().getLayer().getID();
      auto layer2ID = hit2->getScin().getSlot().getLayer().getID();
      auto layer1Radius = hit1->getScin().getSlot().getLayer().getRadius();
      auto layer2Radius = hit2->getScin().getSlot().getLayer().getRadius();

      if (layer1ID == layer2ID)
      {
        continue;
      }

      // Change the order of hits depending on layers
      if (layer1Radius < layer2Radius)
      {
        hit1 = dynamic_cast<const JPetPhysRecoHit*>(event.getHits().at(j));
        hit2 = dynamic_cast<const JPetPhysRecoHit*>(event.getHits().at(i));
      }

      auto test1 = false;
      auto test2 = false;

      stats.fillHistogram("cosmic_hits_x_diff_all", hit2->getPosX() - hit1->getPosX());

      // Normal vector in X pointing down
      auto vecN_X = TVector3(-1.0, 0.0, 0.0);

      // Checknig angle in XZ plane - including detector rotation in Y
      TVector3 hit1PosXZ(hit1->getPosX(), 0.0, hit1->getPosZ());
      TVector3 hit2PosXZ(hit2->getPosX(), 0.0, hit2->getPosZ());
      TVector3 cosmicXZ = hit2PosXZ - hit1PosXZ;
      double thetaXZ = TMath::RadToDeg() * cosmicXZ.Angle(vecN_X);

      stats.fillHistogram("cosmic_hits_theta_xz_all", thetaXZ);
      stats.fillHistogram("cosmic_hits_z_diff_all", hit2->getPosZ() - hit1->getPosZ());
      if (fabs(thetaXZ - detectorYRot) < maxThetaDiff)
      {
        test1 = true;
        stats.fillHistogram("cosmic_hits_theta_xz_cut", thetaXZ);
        stats.fillHistogram("cosmic_hits_z_diff_cut", hit2->getPosZ() - hit1->getPosZ());
      }

      // Checknig angle in XZ plane - to select neighbouring strips
      TVector3 hit1PosXY(hit1->getPosX(), hit1->getPosY(), 0.0);
      TVector3 hit2PosXY(hit2->getPosX(), hit2->getPosY(), 0.0);
      TVector3 cosmicXY = hit2PosXY - hit1PosXY;
      double thetaXY = TMath::RadToDeg() * hit1PosXY.Angle(vecN_X);

      stats.fillHistogram("cosmic_hits_theta_xy_all", thetaXY);
      stats.fillHistogram("cosmic_hits_y_diff_all", hit2->getPosY() - hit1->getPosY());
      if (fabs(180.0 - thetaXY) < maxThetaDiff)
      {
        test2 = true;
        stats.fillHistogram("cosmic_hits_y_diff_cut", hit2->getPosY() - hit1->getPosY());
        stats.fillHistogram("cosmic_hits_theta_xy_cut", thetaXY);
      }

      auto tDiff1 = hit2->getTime() - hit1->getTime();
      auto tDiff2 = EventCategorizerTools::calculateDistance(hit1, hit2) / kMuonAvVelocity_cm_ps;

      stats.fillHistogram(Form("cosmic_tof_tdiff_scin_%d_all", hit1->getScin().getID()), hit2->getScin().getID(), tDiff1);
      stats.fillHistogram(Form("cosmic_tof_offset_scin_%d_all", hit1->getScin().getID()), hit2->getScin().getID(), tDiff1 - tDiff2);
      if (test1 && test2)
      {
        stats.fillHistogram(Form("cosmic_tof_tdiff_scin_%d_cut", hit1->getScin().getID()), hit2->getScin().getID(), tDiff1);
        stats.fillHistogram(Form("cosmic_tof_offset_scin_%d_cut", hit1->getScin().getID()), hit2->getScin().getID(), tDiff1 - tDiff2);
      }
    }
  }
}

double CalibrationTools::calculateReveresedToT(const JPetPhysRecoHit* hit)
{
  if (hit->getSignalA().getToT() != 0.0 && hit->getSignalB().getToT() != 0.0)
  {
    return (1.0 / hit->getSignalB().getToT()) - (1.0 / hit->getSignalA().getToT());
  }
  else
  {
    return 0.0;
  }
}
