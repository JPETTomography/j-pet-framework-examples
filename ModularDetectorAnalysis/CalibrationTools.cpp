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
 * Selecting pair of hits for calibrations based on ToT and Scin ID
 */
void CalibrationTools::selectForTOF2Gamma(const JPetEvent& event, JPetStatistics& stats, bool saveCalibHistos, double totCutAnniMin,
                                          double totCutAnniMax, double totCutDeexMin, double totCutDeexMax, double scatterTestValue)
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
      if (EventCategorizerTools::checkForScatter(hit1, hit2, stats, true, scatterTestValue))
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

      // double t0_A = aTime - posA.Mag() / kLightVelocity_cm_ps;
      // double t0_D = dTime - posD.Mag() / kLightVelocity_cm_ps;
      // double tDiff_A_D = t0_A - t0_D;
      // double tDiff_A_D = t0_A - t0_D;

      // Filling histograms for specific scintillators
      if (saveCalibHistos && aScinID != -1 && dScinID != -1)
      {
        // stats.fillHistogram("tdiff_anni_scin", aScinID, tDiff_A_D);
        // stats.fillHistogram("tdiff_deex_scin", dScinID, tDiff_A_D);
        stats.fillHistogram("tdiff_anni_scin", aScinID, aTime - dTime);
        stats.fillHistogram("tdiff_deex_scin", dScinID, dTime - aTime);
      }
    }
  }
}

void CalibrationTools::selectForTOF3Gamma(const JPetEvent& event, JPetStatistics& stats, bool saveCalibHistos, double totCutAnniMin,
                                          double totCutAnniMax, double totCutDeexMin, double totCutDeexMax, double scatterTestValue)
{
  if (event.getHits().size() < 3)
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
      if (EventCategorizerTools::checkForScatter(hit1, hit2, stats, true, scatterTestValue))
      {
        continue;
      }

      for (uint k = j + 1; k < event.getHits().size(); k++)
      {
        auto hit3 = dynamic_cast<const JPetPhysRecoHit*>(event.getHits().at(k));
        if (!hit3)
        {
          continue;
        }

        // Skip if scatter
        if (EventCategorizerTools::checkForScatter(hit2, hit3, stats, true, scatterTestValue))
        {
          continue;
        }

        auto tot1 = hit1->getToT();
        auto tot2 = hit2->getToT();
        auto tot3 = hit3->getToT();

        // Checking ToT of hits to classify them as annihilation or deexcitation
        bool anih1 = (tot1 > totCutAnniMin && tot1 < totCutAnniMax);
        bool anih2 = (tot2 > totCutAnniMin && tot2 < totCutAnniMax);
        bool anih3 = (tot3 > totCutAnniMin && tot3 < totCutAnniMax);
        bool deex1 = (tot1 > totCutDeexMin && tot1 < totCutDeexMax);
        bool deex2 = (tot2 > totCutDeexMin && tot2 < totCutDeexMax);
        bool deex3 = (tot3 > totCutDeexMin && tot3 < totCutDeexMax);

        // Time differences and strip ID to be assigned
        double a1Time = 0.0, a2Time = 0.0, dTime = 0.0;
        int a1ScinID = -1, a2ScinID, dScinID = -1;
        TVector3 posA1, posA2, posD;

        if (anih1 && anih2 && deex3)
        {
          a1ScinID = hit1->getScin().getID();
          a2ScinID = hit2->getScin().getID();
          dScinID = hit3->getScin().getID();
          a1Time = hit1->getTime();
          a2Time = hit2->getTime();
          dTime = hit3->getTime();
          posA1 = hit1->getPos();
          posA2 = hit2->getPos();
          posD = hit3->getPos();
        }
        else if (anih1 && anih3 && deex2)
        {
          a1ScinID = hit1->getScin().getID();
          a2ScinID = hit3->getScin().getID();
          dScinID = hit2->getScin().getID();
          a1Time = hit1->getTime();
          a2Time = hit3->getTime();
          dTime = hit2->getTime();
          posA1 = hit1->getPos();
          posA2 = hit3->getPos();
          posD = hit2->getPos();
        }
        else if (anih2 && anih3 && deex1)
        {
          a1ScinID = hit2->getScin().getID();
          a2ScinID = hit3->getScin().getID();
          dScinID = hit1->getScin().getID();
          a1Time = hit2->getTime();
          a2Time = hit3->getTime();
          dTime = hit1->getTime();
          posA1 = hit2->getPos();
          posA2 = hit3->getPos();
          posD = hit1->getPos();
        }
        else
        {
          continue;
        }

        // double t0_A1 = a1Time - posA1.Mag() / kLightVelocity_cm_ps;
        // double t0_A2 = a2Time - posA2.Mag() / kLightVelocity_cm_ps;
        // double t0_D = dTime - posD.Mag() / kLightVelocity_cm_ps;
        // double tDiff_A1_D = t0_A1 - t0_D;
        // double tDiff_A2_D = t0_A1 - t0_D;

        // Filling histograms for specific scintillators
        if (saveCalibHistos && a1ScinID != -1 && a2ScinID != -1 && dScinID != -1)
        {
          // stats.fillHistogram("tdiff_anni_scin_3g", a1ScinID, tDiff_A1_D);
          // stats.fillHistogram("tdiff_anni_scin_3g", a2ScinID, tDiff_A2_D);
          // stats.fillHistogram("tdiff_deex_scin_3g", dScinID, tDiff_A1_D);
          // stats.fillHistogram("tdiff_deex_scin_3g", dScinID, tDiff_A2_D);

          stats.fillHistogram("tdiff_anni_scin_3g", a1ScinID, a1Time - dTime);
          stats.fillHistogram("tdiff_anni_scin_3g", a2ScinID, a2Time - dTime);
          stats.fillHistogram("tdiff_deex_scin_3g", dScinID, dTime - a1Time);
          stats.fillHistogram("tdiff_deex_scin_3g", dScinID, dTime - a2Time);
        }
      }
    }
  }
}

void CalibrationTools::selectForTimeWalk(const JPetEvent& event, JPetStatistics& stats, bool saveCalibHistos, double maxThetaDiff, double maxTimeDiff,
                                         double totCutAnniMin, double totCutAnniMax, const TVector3& sourcePos)
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

        if (fabs(firstHit->getPosZ()) < 2.0 && fabs(secondHit->getPosZ()) < 2.0)
        {
          stats.fillHistogram("time_walk_ab_tdiff_z_cut", firstHit->getTimeDiff(), revToT1);
        }

        if (firstHit->getScin().getSlot().getTheta() < secondHit->getScin().getSlot().getTheta())
        {
          stats.fillHistogram("time_walk_tof", EventCategorizerTools::calculateTOF(firstHit, secondHit), revToT1 - revToT2);
          if (fabs(firstHit->getPosZ()) < 2.0 && fabs(secondHit->getPosZ()) < 2.0)
          {
            stats.fillHistogram("time_walk_tof_z_cut", EventCategorizerTools::calculateTOF(firstHit, secondHit), revToT1 - revToT2);
          }
        }
        else
        {
          stats.fillHistogram("time_walk_tof", EventCategorizerTools::calculateTOF(secondHit, firstHit), revToT2 - revToT1);
          if (fabs(firstHit->getPosZ()) < 2.0 && fabs(secondHit->getPosZ()) < 2.0)
          {
            stats.fillHistogram("time_walk_tof_z_cut", EventCategorizerTools::calculateTOF(secondHit, firstHit), revToT2 - revToT1);
          }
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
