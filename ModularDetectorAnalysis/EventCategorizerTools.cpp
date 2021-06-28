/**
 *  @copyright Copyright 2021 The J-PET Framework Authors. All rights reserved.
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may find a copy of the License in the LICENCE file.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 *  @file EventCategorizerTools.cpp
 */

#include "EventCategorizerTools.h"
#include <JPetMatrixSignal/JPetMatrixSignal.h>
#include <TMath.h>
#include <vector>

using namespace std;

/**
 * Selecting pair of hits for calibrations based on TOT and Scin ID
 */
void EventCategorizerTools::selectForTOF(const JPetEvent& event, JPetStatistics& stats, bool saveCalibHistos, double totCutAnniMin,
                                         double totCutAnniMax, double totCutDeexMin, double totCutDeexMax, const TVector3& sourcePos)
{
  if (event.getHits().size() < 2)
  {
    return;
  }
  for (uint i = 0; i < event.getHits().size(); i++)
  {
    for (uint j = i + 1; j < event.getHits().size(); j++)
    {

      auto tot1 = event.getHits().at(i).getEnergy();
      auto tot2 = event.getHits().at(j).getEnergy();
      auto scin1ID = event.getHits().at(i).getScin().getID();
      auto scin2ID = event.getHits().at(j).getScin().getID();

      bool anih1 = false;
      bool anih2 = false;
      bool deex1 = false;
      bool deex2 = false;

      // Checking TOT of hits to classify them as annihilation or deexcitation
      if (tot1 > totCutAnniMin && tot1 < totCutAnniMax)
      {
        anih1 = true;
      }
      if (tot2 > totCutAnniMin && tot2 < totCutAnniMax)
      {
        anih2 = true;
      }
      if (tot1 > totCutDeexMin && tot1 < totCutDeexMax)
      {
        deex1 = true;
      }
      if (tot2 > totCutDeexMin && tot2 < totCutDeexMax)
      {
        deex2 = true;
      }

      // Time differences and strip ID to be assigned
      double aTime = 0.0, dTime = 0.0;
      int aScinID = -1, dScinID = -1;
      TVector3 posA, posD;

      if (anih1 && deex2)
      {
        aScinID = event.getHits().at(i).getScin().getID();
        dScinID = event.getHits().at(j).getScin().getID();
        aTime = event.getHits().at(i).getTime();
        dTime = event.getHits().at(j).getTime();
        posA = event.getHits().at(i).getPos();
        posD = event.getHits().at(j).getPos();
      }
      else if (anih2 && deex1)
      {
        aScinID = event.getHits().at(j).getScin().getID();
        dScinID = event.getHits().at(i).getScin().getID();
        aTime = event.getHits().at(j).getTime();
        dTime = event.getHits().at(i).getTime();
        posA = event.getHits().at(j).getPos();
        posD = event.getHits().at(i).getPos();
      }
      else
      {
        continue;
      }

      double t0_A = aTime - (posA - sourcePos).Mag() / kLightVelocity_cm_ps;
      double t0_D = dTime - (posD - sourcePos).Mag() / kLightVelocity_cm_ps;
      double tDiff_A_D = t0_A - t0_D;

      // Filling histograms for specific scintillators
      if (saveCalibHistos && aScinID != -1 && dScinID != -1)
      {
        stats.getHisto2D("tdiff_annih_scin")->Fill(aScinID, tDiff_A_D);
        stats.getHisto2D("tdiff_deex_scin")->Fill(dScinID, tDiff_A_D);
      }
    }
  }
}

void EventCategorizerTools::selectForTimeWalk(const JPetEvent& event, JPetStatistics& stats, bool saveCalibHistos, double maxthetaDiff,
                                              double maxTimeDiff, double totCutAnniMin, double totCutAnniMax, const TVector3& sourcePos)
{
  if (event.getHits().size() < 2)
  {
    return;
  }

  for (uint i = 0; i < event.getHits().size(); i++)
  {
    for (uint j = i + 1; j < event.getHits().size(); j++)
    {
      JPetHit firstHit, secondHit;

      if (event.getHits().at(i).getTime() < event.getHits().at(j).getTime())
      {
        firstHit = event.getHits().at(i);
        secondHit = event.getHits().at(j);
      }
      else
      {
        firstHit = event.getHits().at(j);
        secondHit = event.getHits().at(i);
      }

      if (checkFor2Gamma(firstHit, secondHit, stats, false, maxthetaDiff, maxTimeDiff, totCutAnniMin, totCutAnniMax, 0.0, 0.0, sourcePos))
      {
        // Calculating reversed TOT for time walk studies
        auto revTOT1 = calculateReveresedTOT(firstHit);
        auto revTOT2 = calculateReveresedTOT(secondHit);

        int scin1ID = firstHit.getScin().getID();
        int scin2ID = secondHit.getScin().getID();

        stats.getHisto2D("time_walk_ab_tdiff")->Fill(firstHit.getTimeDiff(), revTOT1);
        stats.getHisto2D("time_walk_ab_tdiff")->Fill(secondHit.getTimeDiff(), revTOT2);
        // stats.getHisto2D(Form("time_walk_ab_tdiff_scin_%d", scin1ID))->Fill(firstHit.getTimeDiff(), revTOT1);
        // stats.getHisto2D(Form("time_walk_ab_tdiff_scin_%d", scin2ID))->Fill(secondHit.getTimeDiff(), revTOT2);

        if (firstHit.getScin().getSlot().getTheta() < secondHit.getScin().getSlot().getTheta())
        {
          stats.getHisto2D("time_walk_tof")->Fill(calculateTOF(firstHit, secondHit), revTOT1 - revTOT2);
          // stats.getHisto2D(Form("time_walk_tof_scin_%d", scin1ID))->Fill(calculateTOF(firstHit, secondHit), revTOT1 - revTOT2);
        }
        else
        {
          stats.getHisto2D("time_walk_tof")->Fill(calculateTOF(secondHit, firstHit), revTOT2 - revTOT1);
          // stats.getHisto2D(Form("time_walk_tof_scin_%d", scin2ID))->Fill(calculateTOF(secondHit, firstHit), revTOT2 - revTOT1);
        }
      }
    }
  }
}

/**
 * Method for determining type of event - back to back 2 gamma
 */
bool EventCategorizerTools::checkFor2Gamma(const JPetEvent& event, JPetStatistics& stats, bool saveHistos, double maxthetaDiff, double maxTimeDiff,
                                           double totCutAnniMin, double totCutAnniMax, double lorAngleMax, double lorPosZMax,
                                           const TVector3& sourcePos)
{
  if (event.getHits().size() < 2)
  {
    return false;
  }

  bool isEvent2Gamma = false;

  for (uint i = 0; i < event.getHits().size(); i++)
  {
    for (uint j = i + 1; j < event.getHits().size(); j++)
    {
      JPetHit firstHit, secondHit;
      if (event.getHits().at(i).getTime() < event.getHits().at(j).getTime())
      {
        firstHit = event.getHits().at(i);
        secondHit = event.getHits().at(j);
      }
      else
      {
        firstHit = event.getHits().at(j);
        secondHit = event.getHits().at(i);
      }

      if (checkFor2Gamma(firstHit, secondHit, stats, saveHistos, maxthetaDiff, maxTimeDiff, totCutAnniMin, totCutAnniMax, lorAngleMax, lorPosZMax,
                         sourcePos))
      {
        isEvent2Gamma = true;
      }
    }
  }
  return isEvent2Gamma;
}

/**
 * Method for determining type of two hits - back to back 2 gamma
 */
bool EventCategorizerTools::checkFor2Gamma(const JPetHit& firstHit, const JPetHit& secondHit, JPetStatistics& stats, bool saveHistos,
                                           double maxthetaDiff, double maxTimeDiff, double totCutAnniMin, double totCutAnniMax, double lorAngleMax,
                                           double lorPosZMax, const TVector3& sourcePos)
{
  int scin1ID = firstHit.getScin().getID();
  int scin2ID = secondHit.getScin().getID();

  TVector3 firstVec = firstHit.getPos() - sourcePos;
  TVector3 secondVec = secondHit.getPos() - sourcePos;
  double theta = TMath::RadToDeg() * firstVec.Angle(secondVec);

  // Registration time difference, always positive
  double timeDiff = fabs(firstHit.getTime() - secondHit.getTime());

  // Average TOT is temporaily stored as hit energy
  auto tot1 = firstHit.getEnergy();
  auto tot2 = secondHit.getEnergy();

  // Calculating reversed TOT for time walk studies
  auto revTOT1 = calculateReveresedTOT(firstHit);
  auto revTOT2 = calculateReveresedTOT(secondHit);

  // TOF calculated by convention
  double tof = calculateTOFByConvention(firstHit, secondHit);

  // LOR angle
  TVector3 vechit1_2D(firstHit.getPosX() - sourcePos.X(), 0.0, firstHit.getPosZ() - sourcePos.Z());
  TVector3 vechit2_2D(secondHit.getPosX() - sourcePos.X(), 0.0, secondHit.getPosZ() - sourcePos.Z());
  TVector3 vechit1_1D(firstHit.getPosX() - sourcePos.X(), 0.0, 0.0);
  TVector3 vechit2_1D(secondHit.getPosX() - sourcePos.X(), 0.0, 0.0);

  double lorAngle1 = TMath::RadToDeg() * vechit1_2D.Angle(vechit1_1D);
  double lorAngle2 = TMath::RadToDeg() * vechit2_2D.Angle(vechit2_2D);

  // Pre-cuts histograms
  if (saveHistos)
  {
    stats.getHisto1D("2g_tot")->Fill(tot1);
    stats.getHisto1D("2g_tot")->Fill(tot2);
    stats.getHisto2D("2g_tot_z_pos")->Fill(firstHit.getPosZ(), tot1);
    stats.getHisto2D("2g_tot_z_pos")->Fill(secondHit.getPosZ(), tot2);
    stats.getHisto2D("2g_tot_scin")->Fill(scin1ID, tot1);
    stats.getHisto2D("2g_tot_scin")->Fill(scin2ID, tot2);

    stats.getHisto1D("2g_revtot")->Fill(revTOT1);
    stats.getHisto1D("2g_revtot")->Fill(revTOT2);
    stats.getHisto2D("2g_revtot_scin")->Fill(scin1ID, revTOT1);
    stats.getHisto2D("2g_revtot_scin")->Fill(scin2ID, revTOT2);
    stats.getHisto2D("2g_revtot_z_pos")->Fill(firstHit.getPosZ(), revTOT1);
    stats.getHisto2D("2g_revtot_z_pos")->Fill(secondHit.getPosZ(), revTOT2);

    stats.getHisto1D("2g_tof")->Fill(tof);
    stats.getHisto2D("2g_tof_scin")->Fill(scin1ID, tof);
    stats.getHisto2D("2g_tof_scin")->Fill(scin1ID, tof);
    stats.getHisto2D("2g_tof_z_pos")->Fill(firstHit.getPosZ(), tof);
    stats.getHisto2D("2g_tof_z_pos")->Fill(secondHit.getPosZ(), tof);

    stats.getHisto1D("2g_ab_tdiff")->Fill(firstHit.getTimeDiff());
    stats.getHisto1D("2g_ab_tdiff")->Fill(secondHit.getTimeDiff());
    stats.getHisto2D("2g_ab_tdiff_scin")->Fill(scin1ID, firstHit.getTimeDiff());
    stats.getHisto2D("2g_ab_tdiff_scin")->Fill(scin2ID, secondHit.getTimeDiff());
    stats.getHisto2D("2g_ab_tdiff_z_pos")->Fill(firstHit.getPosZ(), firstHit.getTimeDiff());
    stats.getHisto2D("2g_ab_tdiff_z_pos")->Fill(secondHit.getPosZ(), secondHit.getTimeDiff());

    stats.getHisto1D("2g_theta")->Fill(theta);
    stats.getHisto2D("2g_theta_scin")->Fill(scin1ID, theta);
    stats.getHisto2D("2g_theta_scin")->Fill(scin2ID, theta);
    stats.getHisto2D("2g_theta_z_pos")->Fill(firstHit.getPosZ(), theta);
    stats.getHisto2D("2g_theta_z_pos")->Fill(secondHit.getPosZ(), theta);

    stats.getHisto1D("cut_stats_none")->Fill(scin1ID);
    stats.getHisto1D("cut_stats_none")->Fill(scin2ID);
  }

  // Checking selection conditions
  bool thetaCut1 = false, thetaCut2 = false, tDiffCut = false, totCut = false;
  // Angular cut is performed to select hits in opposite modules
  // Can be done in a precise way (vector theta diff around 180 degree theta)
  // or exactly opposite slot, based on ID difference (equal to 12)
  int slot1ID = firstHit.getScin().getSlot().getID();
  int slot2ID = secondHit.getScin().getSlot().getID();
  int slotIDDiff = max(slot1ID - slot2ID, slot2ID - slot1ID);
  if (slotIDDiff == 12)
  {
    thetaCut1 = true;
    if (saveHistos)
    {
      stats.getHisto1D("cut_stats_a1")->Fill(scin1ID);
      stats.getHisto1D("cut_stats_a1")->Fill(scin2ID);
    }
  }

  if (180.0 - theta < maxthetaDiff)
  {
    thetaCut2 = true;
    if (saveHistos)
    {
      stats.getHisto1D("cut_stats_a2")->Fill(scin1ID);
      stats.getHisto1D("cut_stats_a2")->Fill(scin2ID);
      stats.getHisto1D("theta_cut_tof")->Fill(tof);
      stats.getHisto1D("theta_cut_tot")->Fill(tot1);
      stats.getHisto1D("theta_cut_tot")->Fill(tot2);
      stats.getHisto1D("theta_cut_ab_tdiff")->Fill(firstHit.getTimeDiff());
      stats.getHisto1D("theta_cut_ab_tdiff")->Fill(secondHit.getTimeDiff());
    }
  }

  // Time difference cut
  if (timeDiff < maxTimeDiff)
  {
    tDiffCut = true;
    if (saveHistos)
    {
      stats.getHisto1D("tof_cut_tot")->Fill(tot1);
      stats.getHisto1D("tof_cut_tot")->Fill(tot2);
      stats.getHisto1D("tof_cut_ab_tdiff")->Fill(firstHit.getTimeDiff());
      stats.getHisto1D("tof_cut_ab_tdiff")->Fill(secondHit.getTimeDiff());
      stats.getHisto1D("tof_cut_theta")->Fill(theta);
    }
  }

  // TOT cut
  if (tot1 > totCutAnniMin && tot1 < totCutAnniMax && tot2 > totCutAnniMin && tot2 < totCutAnniMax)
  {
    totCut = true;
    if (saveHistos)
    {
      stats.getHisto1D("cut_stats_tot")->Fill(scin1ID);
      stats.getHisto1D("cut_stats_tot")->Fill(scin2ID);
      stats.getHisto1D("tot_cut_tof")->Fill(tof);
      stats.getHisto1D("tot_cut_ab_tdiff")->Fill(firstHit.getTimeDiff());
      stats.getHisto1D("tot_cut_ab_tdiff")->Fill(secondHit.getTimeDiff());
      stats.getHisto1D("tot_cut_theta")->Fill(theta);
    }
  }

  // Pair of hits that meet cut conditions are treated as coming from annihilation point
  if (saveHistos && tDiffCut && thetaCut2 && totCut)
  {
    TVector3 annhilationPoint = calculateAnnihilationPoint(firstHit, secondHit);

    stats.getHisto1D("ap_tot")->Fill(tot1);
    stats.getHisto1D("ap_tot")->Fill(tot2);
    stats.getHisto2D("ap_tot_scin")->Fill(scin1ID, tot1);
    stats.getHisto2D("ap_tot_scin")->Fill(scin2ID, tot2);
    stats.getHisto2D("ap_tot_z_pos")->Fill(firstHit.getPosZ(), tot1);
    stats.getHisto2D("ap_tot_z_pos")->Fill(secondHit.getPosZ(), tot2);

    stats.getHisto1D("ap_revtot")->Fill(revTOT1);
    stats.getHisto1D("ap_revtot")->Fill(revTOT2);
    stats.getHisto2D("ap_revtot_scin")->Fill(scin1ID, revTOT1);
    stats.getHisto2D("ap_revtot_scin")->Fill(scin2ID, revTOT2);
    stats.getHisto2D("ap_revtot_z_pos")->Fill(firstHit.getPosZ(), revTOT1);
    stats.getHisto2D("ap_revtot_z_pos")->Fill(secondHit.getPosZ(), revTOT2);

    stats.getHisto1D("ap_ab_tdiff")->Fill(firstHit.getTimeDiff());
    stats.getHisto1D("ap_ab_tdiff")->Fill(secondHit.getTimeDiff());
    stats.getHisto2D("ap_ab_tdiff_scin")->Fill(scin1ID, firstHit.getTimeDiff());
    stats.getHisto2D("ap_ab_tdiff_scin")->Fill(scin2ID, secondHit.getTimeDiff());
    stats.getHisto2D("ap_ab_tdiff_z_pos")->Fill(firstHit.getPosZ(), firstHit.getTimeDiff());
    stats.getHisto2D("ap_ab_tdiff_z_pos")->Fill(secondHit.getPosZ(), secondHit.getTimeDiff());

    stats.getHisto1D("ap_tof")->Fill(tof);
    stats.getHisto2D("ap_tof_scin")->Fill(scin1ID, tof);
    stats.getHisto2D("ap_tof_scin")->Fill(scin2ID, tof);
    stats.getHisto2D("ap_tof_z_pos")->Fill(firstHit.getPosZ(), tof);
    stats.getHisto2D("ap_tof_z_pos")->Fill(secondHit.getPosZ(), tof);

    stats.getHisto2D("ap_yx")->Fill(annhilationPoint.Y(), annhilationPoint.X());
    stats.getHisto2D("ap_zx")->Fill(annhilationPoint.Z(), annhilationPoint.X());
    stats.getHisto2D("ap_zy")->Fill(annhilationPoint.Z(), annhilationPoint.Y());
    stats.getHisto2D("ap_yx_zoom")->Fill(annhilationPoint.Y(), annhilationPoint.X());
    stats.getHisto2D("ap_zx_zoom")->Fill(annhilationPoint.Z(), annhilationPoint.X());
    stats.getHisto2D("ap_zy_zoom")->Fill(annhilationPoint.Z(), annhilationPoint.Y());

    if (lorAngle1 < lorAngleMax && lorAngle2 < lorAngleMax)
    {
      stats.getHisto2D("ap_yx_zoom_lor_cut")->Fill(annhilationPoint.Y(), annhilationPoint.X());
      stats.getHisto2D("ap_zx_zoom_lor_cut")->Fill(annhilationPoint.Z(), annhilationPoint.X());
      stats.getHisto2D("ap_zy_zoom_lor_cut")->Fill(annhilationPoint.Z(), annhilationPoint.Y());
      stats.getHisto1D("ap_tof_lor_cut")->Fill(tof);
    }

    if (fabs(firstHit.getPosZ()) < lorPosZMax && fabs(secondHit.getPosZ()) < lorPosZMax)
    {
      stats.getHisto2D("ap_yx_zoom_z_cut")->Fill(annhilationPoint.Y(), annhilationPoint.X());
      stats.getHisto2D("ap_zx_zoom_z_cut")->Fill(annhilationPoint.Z(), annhilationPoint.X());
      stats.getHisto2D("ap_zy_zoom_z_cut")->Fill(annhilationPoint.Z(), annhilationPoint.Y());
    }
  }
  // Returning event as 2 gamma if meets cut conditions
  if (totCut && tDiffCut && thetaCut2)
  {
    return true;
  }
  return false;
}

/**
 * Checking each pair of hits in the event if meet selection conditions for 2 gamma annihilation.
 * If yes, the two hits are used to create a new event, that represent Line of Response, and can be used
 * to calculate annihilation point based on TOF and distance.
 */
vector<JPetEvent> EventCategorizerTools::getLORs(const JPetEvent& event, JPetStatistics& stats, bool saveHistos, double maxTOF, double maxScatter,
                                                 double totCutAnniMin, double totCutAnniMax)
{
  vector<JPetEvent> lors;
  if (event.getHits().size() < 2)
  {
    return lors;
  }

  for (uint i = 0; i < event.getHits().size(); i++)
  {
    for (uint j = i + 1; j < event.getHits().size(); j++)
    {
      JPetHit firstHit, secondHit;

      if (event.getHits().at(i).getTime() < event.getHits().at(j).getTime())
      {
        firstHit = event.getHits().at(i);
        secondHit = event.getHits().at(j);
      }
      else
      {
        firstHit = event.getHits().at(j);
        secondHit = event.getHits().at(i);
      }

      // The pair of hits is rejected, if they pass scatter test
      if (checkForScatter(firstHit, secondHit, stats, saveHistos, maxScatter))
      {
        continue;
      }

      // TOF calculated by convention
      double tof = calculateTOFByConvention(firstHit, secondHit);

      // Average TOT is temporaily stored as hit energy
      auto tot1 = firstHit.getEnergy();
      auto tot2 = secondHit.getEnergy();

      // Pre-cut statistics
      if (saveHistos)
      {
        stats.fillHistogram("2g_tot", tot1);
        stats.fillHistogram("2g_tot", tot2);
        stats.fillHistogram("2g_tof", tof);
      }

      // Checking conditions
      bool tofCutPass = false, totCutPass = false;
      if (tof < maxTOF)
      {
        tofCutPass = true;
      }
      if (tot1 > totCutAnniMin && tot1 < totCutAnniMax && tot2 > totCutAnniMin && tot2 < totCutAnniMax)
      {
        totCutPass = true;
      }

      if (tofCutPass && totCutPass)
      {
        // Creating LOR
        JPetEvent lor;
        lor.addHit(firstHit);
        lor.addHit(secondHit);
        lor.addEventType(JPetEventType::k2Gamma);
        lors.push_back(lor);

        // Pair of hits that meet cut conditions are treated as coming from annihilation point
        if (saveHistos)
        {
          TVector3 ap = calculateAnnihilationPoint(firstHit, secondHit);
          stats.fillHistogram("ap_yx", ap.Y(), ap.X());
          stats.fillHistogram("ap_zx", ap.Z(), ap.X());
          stats.fillHistogram("ap_zy", ap.Z(), ap.Y());
          stats.fillHistogram("ap_yx_zoom", ap.Y(), ap.X());
          stats.fillHistogram("ap_zx_zoom", ap.Z(), ap.X());
          stats.fillHistogram("ap_zy_zoom", ap.Z(), ap.Y());
        }
      }
    }
  }
  return lors;
}

/**
 * Method for determining type of event - 3Gamma
 */
bool EventCategorizerTools::checkFor3Gamma(const JPetEvent& event, JPetStatistics& stats, bool saveHistos)
{
  if (event.getHits().size() < 3)
    return false;
  for (uint i = 0; i < event.getHits().size(); i++)
  {
    for (uint j = i + 1; j < event.getHits().size(); j++)
    {
      for (uint k = j + 1; k < event.getHits().size(); k++)
      {
        JPetHit firstHit = event.getHits().at(i);
        JPetHit secondHit = event.getHits().at(j);
        JPetHit thirdHit = event.getHits().at(k);

        vector<double> thetaAngles;
        thetaAngles.push_back(firstHit.getScin().getSlot().getTheta());
        thetaAngles.push_back(secondHit.getScin().getSlot().getTheta());
        thetaAngles.push_back(thirdHit.getScin().getSlot().getTheta());
        sort(thetaAngles.begin(), thetaAngles.end());

        vector<double> relativeAngles;
        relativeAngles.push_back(thetaAngles.at(1) - thetaAngles.at(0));
        relativeAngles.push_back(thetaAngles.at(2) - thetaAngles.at(1));
        relativeAngles.push_back(360.0 - thetaAngles.at(2) + thetaAngles.at(0));
        sort(relativeAngles.begin(), relativeAngles.end());
        double transformedX = relativeAngles.at(1) + relativeAngles.at(0);
        double transformedY = relativeAngles.at(1) - relativeAngles.at(0);

        if (saveHistos)
        {
          stats.getHisto2D("3Gamma_Angles")->Fill(transformedX, transformedY);
        }
      }
    }
  }
  return true;
}

/**
 * Method for determining type of event - prompt
 */
bool EventCategorizerTools::checkForPrompt(const JPetEvent& event, JPetStatistics& stats, bool saveHistos, double deexTOTCutMin, double deexTOTCutMax)
{
  for (unsigned i = 0; i < event.getHits().size(); i++)
  {
    double tot = event.getHits().at(i).getEnergy();
    if (tot > deexTOTCutMin && tot < deexTOTCutMax)
    {
      if (saveHistos)
      {
        stats.fillHistogram("deex_tot_cut_pass", tot);
      }
      return true;
    }
  }
  return false;
}

/**
 * Method for determining type of event - scatter
 */
bool EventCategorizerTools::checkForScatter(const JPetEvent& event, JPetStatistics& stats, bool saveHistos, double scatterTOF)
{
  if (event.getHits().size() < 2)
  {
    return false;
  }
  bool hasScatteredHits = false;
  for (uint i = 0; i < event.getHits().size(); ++i)
  {
    for (uint j = i + 1; j < event.getHits().size(); ++j)
    {
      JPetHit primaryHit, scatterHit;
      if (event.getHits().at(i).getTime() < event.getHits().at(j).getTime())
      {
        primaryHit = event.getHits().at(i);
        scatterHit = event.getHits().at(j);
      }
      else
      {
        primaryHit = event.getHits().at(j);
        scatterHit = event.getHits().at(i);
      }

      if (checkForScatter(primaryHit, scatterHit, stats, saveHistos, scatterTOF))
      {
        hasScatteredHits = true;
      }
    }
  }
  return hasScatteredHits;
}

/**
 * Checking if pair of hits meet scattering condition
 */
bool EventCategorizerTools::checkForScatter(const JPetHit& primaryHit, const JPetHit& scatterHit, JPetStatistics& stats, bool saveHistos,
                                            double scatterTestValue)
{
  double dist = calculateDistance(primaryHit, scatterHit);
  double timeDiff = scatterHit.getTime() - primaryHit.getTime();

  double testDist = fabs(dist - timeDiff * kLightVelocity_cm_ps);
  double testTime = fabs(timeDiff - dist / kLightVelocity_cm_ps);

  stats.fillHistogram("scatter_test_dist", testDist);
  stats.fillHistogram("scatter_test_time", testTime);

  if (testTime < scatterTestValue)
  {
    if (saveHistos)
    {
      double scattAngle = calculateScatteringAngle(primaryHit, scatterHit);
      stats.fillHistogram("scatter_test_pass", testTime);
      stats.fillHistogram("scatter_angle_tot_primary", scattAngle, primaryHit.getEnergy());
      stats.fillHistogram("scatter_angle_tot_scatter", scattAngle, scatterHit.getEnergy());
    }
    return true;
  }
  else
  {
    if (saveHistos)
    {
      stats.fillHistogram("scatter_test_fail", testTime);
    }
    return false;
  }
}

/**
 * Calculation of the total TOT of the hit - Time over Threshold:
 * the sum of the TOTs on all of the thresholds and on the both sides (A,B)
 */
double EventCategorizerTools::calculateTOT(const JPetHit& hit)
{
  auto multi = hit.getSignalA().getRawSignals().size() + hit.getSignalB().getRawSignals().size();
  return hit.getEnergy() / ((double)multi);
}

/**
 * Calculation of the total reversed TOT of the hit. It is defined as:
 * revTOT_hit = revTOT_sigB - revTOT_sigA. Reversed matrix signal TOT is defined as:
 * revTOT_sig = 1/tot_simp1 + 1/tot_simp2 + 1/tot_simp3 + 1/tot_simp4;
 */
double EventCategorizerTools::calculateReveresedTOT(const JPetHit& hit)
{
  if (hit.getSignalA().getTOT() != 0.0 && hit.getSignalB().getTOT() != 0.0)
  {
    return (1.0 / hit.getSignalB().getTOT()) - (1.0 / hit.getSignalA().getTOT());
  }
  else
  {
    return 0.0;
  }
  // double revTOT_sigA = 0.0, revTOT_sigB = 0.0;
  // for (auto& rawSig : hit.getSignalA().getRawSignals())
  // {
  //   for (auto& thrTOT : rawSig.second.getTOTsVsThresholdNumber())
  //   {
  //     if (thrTOT.second != 0.0)
  //     {
  //       revTOT_sigA += 1.0 / thrTOT.second;
  //     }
  //   }
  // }
  //
  // for (auto& rawSig : hit.getSignalB().getRawSignals())
  // {
  //   for (auto& thrTOT : rawSig.second.getTOTsVsThresholdNumber())
  //   {
  //     if (thrTOT.second != 0.0)
  //     {
  //       revTOT_sigB += 1.0 / thrTOT.second;
  //     }
  //   }
  // }
  // return revTOT_sigB - revTOT_sigA;
}

/**
 * Calculation of distance between two hits
 */
double EventCategorizerTools::calculateDistance(const JPetHit& hit1, const JPetHit& hit2) { return (hit1.getPos() - hit2.getPos()).Mag(); }

/**
 * Calculation of time that light needs to travel the distance between primary gamma
 * and scattered gamma. Return value in picoseconds.
 */
double EventCategorizerTools::calculateScatteringTime(const JPetHit& hit1, const JPetHit& hit2)
{
  return calculateDistance(hit1, hit2) / kLightVelocity_cm_ps;
}

/**
 * Calculation of scatter angle between primary hit and scattered hit.
 * This function assumes that source of first gamma was in (0,0,0).
 * Angle is calculated from scalar product, return value in degrees.
 */
double EventCategorizerTools::calculateScatteringAngle(const JPetHit& hit1, const JPetHit& hit2)
{
  return TMath::RadToDeg() * hit1.getPos().Angle(hit2.getPos() - hit1.getPos());
}

/**
 * @brief Calculation of an annihilation point based on LOR and TOFof two hits.
 *
 * Line of Response between two hits is used to estimate annihilation point by calculating its
 * middle and shifting it along LOR based on Time of Flight value toward one hit or the other.
 */
TVector3 EventCategorizerTools::calculateAnnihilationPoint(const JPetHit& hit1, const JPetHit& hit2)
{
  TVector3 middleOfLOR = 0.5 * (hit1.getPos() + hit2.getPos());
  TVector3 versorOnLOR = (hit2.getPos() - hit1.getPos()).Unit();

  double tof = EventCategorizerTools::calculateTOFByConvention(hit1, hit2);
  double shift = 0.5 * tof * kLightVelocity_cm_ps;

  TVector3 annihilationPoint;
  annihilationPoint.SetX(middleOfLOR.X() + shift * versorOnLOR.X());
  annihilationPoint.SetY(middleOfLOR.Y() + shift * versorOnLOR.Y());
  annihilationPoint.SetZ(middleOfLOR.Z() + shift * versorOnLOR.Z());
  return annihilationPoint;
}

/**
 * @brief Calculation of an annihilation point based on positions of three hits.
 */
TVector3 EventCategorizerTools::calculateAnnihilationPoint(const JPetHit& hit1, const JPetHit& hit2, const JPetHit& hit3)
{
  // Calculating norm vector for a surface created by 3 hits (vectors of their positions)
  TVector3 surfaceVec;
  surfaceVec.SetX((hit2.getPosY() - hit1.getPosY()) * (hit3.getPosZ() - hit1.getPosZ()) -
                  (hit2.getPosZ() - hit1.getPosZ()) * (hit3.getPosY() - hit1.getPosY()));
  surfaceVec.SetY((hit2.getPosZ() - hit1.getPosZ()) * (hit3.getPosX() - hit1.getPosX()) -
                  (hit2.getPosX() - hit1.getPosX()) * (hit3.getPosZ() - hit1.getPosZ()));
  surfaceVec.SetZ((hit2.getPosX() - hit1.getPosX()) * (hit3.getPosY() - hit1.getPosY()) -
                  (hit2.getPosY() - hit1.getPosY()) * (hit3.getPosX() - hit1.getPosX()));

  // Unitary perpendicular vector
  TVector3 perpVec(-surfaceVec.Y(), surfaceVec.X(), 0);
  perpVec = perpVec.Unit();

  double theta = -acos(surfaceVec.Z() / surfaceVec.Mag());

  // Defining rotation transformation to 2D plane and its reverse
  TVector3 rotX, rotY, rotZ, rotXr, rotYr, rotZr;
  rotX.SetX(cos(theta) + perpVec.X() * perpVec.X() * (1 - cos(theta)));
  rotX.SetY(perpVec.X() * perpVec.Y() * (1 - cos(theta)));
  rotX.SetZ(perpVec.Y() * sin(theta));
  rotY.SetX(perpVec.X() * perpVec.Y() * (1 - cos(theta)));
  rotY.SetY(cos(theta) + perpVec.Y() * perpVec.Y() * (1 - cos(theta)));
  rotY.SetZ(-perpVec.X() * sin(theta));
  rotZ.SetX(-perpVec.Y() * sin(theta));
  rotZ.SetY(perpVec.X() * sin(theta));
  rotZ.SetZ(cos(theta));
  rotXr.SetX(cos(-theta) + perpVec.X() * perpVec.X() * (1 - cos(-theta)));
  rotXr.SetY(perpVec.X() * perpVec.Y() * (1 - cos(-theta)));
  rotXr.SetZ(perpVec.Y() * sin(-theta));
  rotYr.SetX(perpVec.X() * perpVec.Y() * (1 - cos(-theta)));
  rotYr.SetY(cos(-theta) + perpVec.Y() * perpVec.Y() * (1 - cos(-theta)));
  rotYr.SetZ(-perpVec.X() * sin(-theta));
  rotZr.SetX(-perpVec.Y() * sin(-theta));
  rotZr.SetY(perpVec.X() * sin(-theta));
  rotZr.SetZ(cos(-theta));

  // Centers of circles transformed
  TVector3 p1(rotX * hit1.getPos(), rotY * hit1.getPos(), rotZ * hit1.getPos());
  TVector3 p2(rotX * hit2.getPos(), rotY * hit2.getPos(), rotZ * hit2.getPos());
  TVector3 p3(rotX * hit3.getPos(), rotY * hit3.getPos(), rotZ * hit3.getPos());

  // Time differences of hits registration
  double tdiff21 = hit2.getTime() - hit1.getTime();
  double tdiff31 = hit3.getTime() - hit1.getTime();

  TVector3 intersection = findIntersection(p1, p2, p3, tdiff21, tdiff31);

  // Transforming back found intersection by reverse rotation
  TVector3 annihilationPoint(rotXr * intersection, rotYr * intersection, rotZr * intersection);
  return annihilationPoint;
}

double EventCategorizerTools::calculateTOFByConvention(const JPetHit& hitA, const JPetHit& hitB)
{
  if (hitA.getScin().getSlot().getTheta() < hitB.getScin().getSlot().getTheta())
  {
    return calculateTOF(hitA, hitB);
  }
  else
  {
    return calculateTOF(hitB, hitA);
  }
}

double EventCategorizerTools::calculateTOF(const JPetHit& hitA, const JPetHit& hitB)
{
  return EventCategorizerTools::calculateTOF(hitA.getTime(), hitB.getTime());
}

double EventCategorizerTools::calculateTOF(double time1, double time2) { return (time1 - time2); }

/**
 * Calculating distance from the center of the decay plane
 */
double EventCategorizerTools::calculatePlaneCenterDistance(const JPetHit& firstHit, const JPetHit& secondHit, const JPetHit& thirdHit)
{
  TVector3 crossProd = (secondHit.getPos() - firstHit.getPos()).Cross(thirdHit.getPos() - secondHit.getPos());
  double distCoef = -crossProd.X() * secondHit.getPosX() - crossProd.Y() * secondHit.getPosY() - crossProd.Z() * secondHit.getPosZ();
  if (crossProd.Mag() != 0)
  {
    return fabs(distCoef) / crossProd.Mag();
  }
  else
  {
    ERROR("One of the hit has zero position vector - unable to calculate distance from the center of the surface");
    return -1.;
  }
}

/**
 * Method for determining type of event for streaming - 2 gamma
 * @todo: the selection criteria b2b distance from center needs to be checked
 * and implemented again
 */
bool EventCategorizerTools::stream2Gamma(const JPetEvent& event, JPetStatistics& stats, bool saveHistos, double b2bSlotthetaDiff, double b2bTimeDiff,
                                         double maxScatter)
{
  if (event.getHits().size() < 2)
  {
    return false;
  }
  for (uint i = 0; i < event.getHits().size(); i++)
  {
    for (uint j = i + 1; j < event.getHits().size(); j++)
    {
      JPetHit firstHit, secondHit;
      if (event.getHits().at(i).getTime() < event.getHits().at(j).getTime())
      {
        firstHit = event.getHits().at(i);
        secondHit = event.getHits().at(j);
      }
      else
      {
        firstHit = event.getHits().at(j);
        secondHit = event.getHits().at(i);
      }

      // Cutting pairs passing scatter test
      if (checkForScatter(firstHit, secondHit, stats, saveHistos, maxScatter))
      {
        continue;
      }

      // Checking for back to back
      double timeDiff = fabs(firstHit.getTime() - secondHit.getTime());
      double deltaLor = (secondHit.getTime() - firstHit.getTime()) * kLightVelocity_cm_ps / 2.0;
      double theta = TMath::RadToDeg() * firstHit.getPos().Angle(secondHit.getPos());

      if (saveHistos)
      {
        stats.fillHistogram("stream2g_tdiff", timeDiff);
        stats.fillHistogram("stream2g_dlor_dist", deltaLor);
        stats.fillHistogram("stream2g_theta_diff", theta);
      }
      if (fabs(theta - 180.0) < b2bSlotthetaDiff && timeDiff < b2bTimeDiff)
      {
        if (saveHistos)
        {
          TVector3 ap = calculateAnnihilationPoint(firstHit, secondHit);
          stats.fillHistogram("ap_yx", ap.Y(), ap.X());
          stats.fillHistogram("ap_zx", ap.Z(), ap.X());
          stats.fillHistogram("ap_zy", ap.Z(), ap.Y());
          stats.fillHistogram("ap_yx_zoom", ap.Y(), ap.X());
          stats.fillHistogram("ap_zx_zoom", ap.Z(), ap.X());
          stats.fillHistogram("ap_zy_zoom", ap.Z(), ap.Y());
        }
        return true;
      }
    }
  }
  return false;
}

/**
 * Method for determining type of event for streaming - 3 gamma annihilation
 */
bool EventCategorizerTools::stream3Gamma(const JPetEvent& event, JPetStatistics& stats, bool saveHistos, double d3SlotthetaMin, double d3TimeDiff,
                                         double d3PlaneCenterDist, double maxScatter)
{
  if (event.getHits().size() < 3)
  {
    return false;
  }
  for (uint i = 0; i < event.getHits().size(); i++)
  {
    JPetHit firstHit = event.getHits().at(i);

    for (uint j = i + 1; j < event.getHits().size(); j++)
    {
      JPetHit secondHit = event.getHits().at(j);

      if (checkForScatter(firstHit, secondHit, stats, saveHistos, maxScatter))
      {
        continue;
      }

      for (uint k = j + 1; k < event.getHits().size(); k++)
      {
        JPetHit thirdHit = event.getHits().at(k);

        if (checkForScatter(firstHit, thirdHit, stats, saveHistos, maxScatter))
        {
          continue;
        }

        if (checkForScatter(secondHit, thirdHit, stats, saveHistos, maxScatter))
        {
          continue;
        }

        vector<double> relativeAngles;
        relativeAngles.push_back(TMath::RadToDeg() * firstHit.getPos().Angle(secondHit.getPos()));
        relativeAngles.push_back(TMath::RadToDeg() * secondHit.getPos().Angle(thirdHit.getPos()));
        relativeAngles.push_back(TMath::RadToDeg() * thirdHit.getPos().Angle(firstHit.getPos()));
        sort(relativeAngles.begin(), relativeAngles.end());

        double transformedX = relativeAngles.at(1) + relativeAngles.at(0);
        double transformedY = relativeAngles.at(1) - relativeAngles.at(0);
        double timeDiff = fabs(thirdHit.getTime() - firstHit.getTime());
        double planeCenterDist = calculatePlaneCenterDistance(firstHit, secondHit, thirdHit);

        if (saveHistos)
        {
          stats.fillHistogram("stream3g_thetas", transformedX, transformedY);
          stats.fillHistogram("stream3g_plane_dist", planeCenterDist);
          stats.fillHistogram("stream3g_tdiff", timeDiff);
        }
        if (transformedX > d3SlotthetaMin && timeDiff < d3TimeDiff && planeCenterDist < d3PlaneCenterDist)
        {
          if (saveHistos)
          {
            TVector3 ap = calculateAnnihilationPoint(firstHit, secondHit, thirdHit);
            stats.fillHistogram("ap_yx", ap.Y(), ap.X());
            stats.fillHistogram("ap_zx", ap.Z(), ap.X());
            stats.fillHistogram("ap_zy", ap.Z(), ap.Y());
            stats.fillHistogram("ap_yx_zoom", ap.Y(), ap.X());
            stats.fillHistogram("ap_zx_zoom", ap.Z(), ap.X());
            stats.fillHistogram("ap_zy_zoom", ap.Z(), ap.Y());
          }
          return true;
        }
      }
    }
  }
  return false;
}

/**
 * Helper method for estimating anihilation point
 */
TVector3 EventCategorizerTools::findIntersection(TVector3 hit1Pos, TVector3 hit2Pos, TVector3 hit3Pos, double t21, double t31)
{
  double R21 = sqrt(pow(hit2Pos(0) - hit1Pos(0), 2) + pow(hit2Pos(1) - hit1Pos(1), 2));
  double R32 = sqrt(pow(hit3Pos(0) - hit2Pos(0), 2) + pow(hit3Pos(1) - hit2Pos(1), 2));
  double R13 = sqrt(pow(hit1Pos(0) - hit3Pos(0), 2) + pow(hit1Pos(1) - hit3Pos(1), 2));

  double TDiffTOR1 = 0.0;
  double TDiffTOR2 = t21;
  double TDiffTOR3 = t31;

  TDiffTOR2 = kLightVelocity_cm_ps * TDiffTOR2;
  TDiffTOR3 = kLightVelocity_cm_ps * TDiffTOR3;

  double R0 = 0.0;

  if (R0 < (R21 - TDiffTOR2) / 2.0)
  {
    R0 = (R21 - TDiffTOR2) / 2.0;
  }
  if (R0 < (R32 - TDiffTOR2 - TDiffTOR3) / 2.0)
  {
    R0 = (R32 - TDiffTOR2 - TDiffTOR3) / 2.0;
  }
  if (R0 < (R13 - TDiffTOR3) / 2.0)
  {
    R0 = (R13 - TDiffTOR3) / 2.0;
  }

  double R1 = 0.;
  double R2 = 0.;
  double R3 = 0.;

  vector<double> temp, temp2;
  vector<vector<double>> points;
  temp.push_back(0.0);
  temp.push_back(0.0);

  points.push_back(temp);
  points.push_back(temp);
  points.push_back(temp);
  points.push_back(temp);
  points.push_back(temp);
  points.push_back(temp);
  temp.clear();

  double Distance = 0.0;
  double MinDistance = 0.0;
  double PreviousDistance = 10000000.0;

  int test = 1;
  while (test)
  {
    R1 = TDiffTOR1 + R0 + 1;
    R2 = TDiffTOR2 + R0 + 1;
    R3 = TDiffTOR2 + R0 + 1;
    points = findIntersectiosOfCircles(hit1Pos, hit2Pos, hit3Pos, R1, R2, R3, R13, R21, R32);

    MinDistance = 1000000.0;
    for (unsigned i = 0; i < 2; i++)
    {
      for (unsigned j = 0; j < 2; j++)
      {
        for (unsigned k = 0; k < 2; k++)
        {
          Distance = sqrt(pow(points[i][0] - points[j + 2][0], 2) + pow(points[i][1] - points[j + 2][1], 2)) +
                     sqrt(pow(points[i][0] - points[k + 4][0], 2) + pow(points[i][1] - points[k + 4][1], 2)) +
                     sqrt(pow(points[k + 4][0] - points[j + 2][0], 2) + pow(points[k + 4][1] - points[j + 2][1], 2));
          if (Distance < MinDistance)
          {
            MinDistance = Distance;
            temp.clear();
            temp.push_back(points[i][0]);
            temp.push_back(points[i][1]);
            temp.push_back(points[2 + j][0]);
            temp.push_back(points[2 + j][1]);
            temp.push_back(points[4 + k][0]);
            temp.push_back(points[4 + k][1]);
          }
        }
      }
    }
    test++;
    if (test % 50 == 0)
    {
      if (MinDistance == 1000000.0)
      {
        temp.clear();
        temp.push_back(100.0);
        temp.push_back(100.0);
        temp.push_back(100.0);
        temp.push_back(100.0);
        temp.push_back(100.0);
        temp.push_back(100.0);
        break;
      }
    }
    if (MinDistance > PreviousDistance)
      test = 0;
    else
    {
      PreviousDistance = MinDistance;
      temp2 = temp;
    }
    R0 += 1;
  }
  vector<double> R0s, Distances;
  if (MinDistance != 1000000.0)
    test = 1;

  double MinnDistance = 1000000.0;
  while (test)
  {
    R1 = TDiffTOR1 + R0 + 1;
    R2 = TDiffTOR2 + R0 + 1;
    R3 = TDiffTOR2 + R0 + 1;
    points = findIntersectiosOfCircles(hit1Pos, hit2Pos, hit3Pos, R1, R2, R3, R13, R21, R32);

    MinDistance = 1000000.;
    for (unsigned i = 0; i < 2; i++)
    {
      for (unsigned j = 0; j < 2; j++)
      {
        for (unsigned k = 0; k < 2; k++)
        {
          Distance = sqrt(pow(points[i][0] - points[j + 2][0], 2) + pow(points[i][1] - points[j + 2][1], 2)) +
                     sqrt(pow(points[i][0] - points[k + 4][0], 2) + pow(points[i][1] - points[k + 4][1], 2)) +
                     sqrt(pow(points[k + 4][0] - points[j + 2][0], 2) + pow(points[k + 4][1] - points[j + 2][1], 2));
          if (Distance < MinDistance)
          {
            MinDistance = Distance;
            temp.clear();
            temp.push_back(points[i][0]);
            temp.push_back(points[i][1]);
            temp.push_back(points[2 + j][0]);
            temp.push_back(points[2 + j][1]);
            temp.push_back(points[4 + k][0]);
            temp.push_back(points[4 + k][1]);
          }
        }
      }
    }
    if (MinDistance != 1000000.0)
    {
      R0s.push_back(R0);
      Distances.push_back(MinDistance);
    }

    test++;
    if (test % 50 == 0)
    {
      if (MinDistance == 1000000.0)
      {
        temp.clear();
        temp.push_back(100.0);
        temp.push_back(100.0);
        temp.push_back(100.0);
        temp.push_back(100.0);
        temp.push_back(100.0);
        temp.push_back(100.0);
        break;
      }
      test = 0;
    }
    if (MinDistance < MinnDistance)
    {
      MinnDistance = MinDistance;
    }
    PreviousDistance = MinDistance;
    temp2 = temp;
    R0 -= 0.1;
  }

  if (MinnDistance != 1000000.0)
  {
    double R0Min;
    double minEle = *min_element(begin(Distances), end(Distances));
    if (minEle == Distances[0])
    {
      R0Min = R0s[0];
    }
    else if (minEle == Distances[Distances.size() - 1])
    {
      R0Min = R0s[R0s.size() - 1];
    }
    else
    {
      R0Min = findMinimumFromDerivative(R0s, Distances);
    }
    R1 = TDiffTOR1 + R0Min + 1;
    R2 = TDiffTOR2 + R0Min + 1;
    R3 = TDiffTOR2 + R0Min + 1;
    points = findIntersectiosOfCircles(hit1Pos, hit2Pos, hit3Pos, R1, R2, R3, R13, R21, R32);

    MinDistance = 1000000.0;
    for (unsigned i = 0; i < 2; i++)
    {
      for (unsigned j = 0; j < 2; j++)
      {
        for (unsigned k = 0; k < 2; k++)
        {
          Distance = sqrt(pow(points[i][0] - points[j + 2][0], 2) + pow(points[i][1] - points[j + 2][1], 2)) +
                     sqrt(pow(points[i][0] - points[k + 4][0], 2) + pow(points[i][1] - points[k + 4][1], 2)) +
                     sqrt(pow(points[k + 4][0] - points[j + 2][0], 2) + pow(points[k + 4][1] - points[j + 2][1], 2));
          if (Distance < MinDistance)
          {
            MinDistance = Distance;
            temp.clear();
            temp.push_back(points[i][0]);
            temp.push_back(points[i][1]);
            temp.push_back(points[2 + j][0]);
            temp.push_back(points[2 + j][1]);
            temp.push_back(points[4 + k][0]);
            temp.push_back(points[4 + k][1]);
          }
        }
      }
    }
  }

  TVector3 recoPoint((temp[0] + temp[2] + temp[4]) / 3, (temp[1] + temp[3] + temp[5]) / 3, hit1Pos(2));
  return recoPoint;
}

vector<vector<double>> EventCategorizerTools::findIntersectiosOfCircles(TVector3 hit1Pos, TVector3 hit2Pos, TVector3 hit3Pos, double R1, double R2,
                                                                        double R3, double R13, double R21, double R32)
{
  vector<vector<double>> points;
  vector<double> temp;
  temp.push_back(0);
  temp.push_back(0);
  points.push_back(temp);
  points.push_back(temp);
  points.push_back(temp);
  points.push_back(temp);
  points.push_back(temp);
  points.push_back(temp);

  points[0][0] =
      (hit1Pos(0) + hit2Pos(0)) / 2 + (pow(R1, 2) - pow(R2, 2)) * (hit2Pos(0) - hit1Pos(0)) / 2 / pow(R21, 2) +
      0.5 * (hit2Pos(1) - hit1Pos(1)) * sqrt(2 * (pow(R1, 2) + pow(R2, 2)) / pow(R21, 2) - pow(pow(R1, 2) - pow(R2, 2), 2) / pow(R21, 4) - 1);
  points[0][1] =
      (hit1Pos(1) + hit2Pos(1)) / 2 + (pow(R1, 2) - pow(R2, 2)) * (hit2Pos(1) - hit1Pos(1)) / 2 / pow(R21, 2) +
      0.5 * (hit1Pos(0) - hit2Pos(0)) * sqrt(2 * (pow(R1, 2) + pow(R2, 2)) / pow(R21, 2) - pow(pow(R1, 2) - pow(R2, 2), 2) / pow(R21, 4) - 1);
  points[1][0] =
      (hit1Pos(0) + hit2Pos(0)) / 2 + (pow(R1, 2) - pow(R2, 2)) * (hit2Pos(0) - hit1Pos(0)) / 2 / pow(R21, 2) -
      0.5 * (hit2Pos(1) - hit1Pos(1)) * sqrt(2 * (pow(R1, 2) + pow(R2, 2)) / pow(R21, 2) - pow(pow(R1, 2) - pow(R2, 2), 2) / pow(R21, 4) - 1);
  points[1][1] =
      (hit1Pos(1) + hit2Pos(1)) / 2 + (pow(R1, 2) - pow(R2, 2)) * (hit2Pos(1) - hit1Pos(1)) / 2 / pow(R21, 2) -
      0.5 * (hit1Pos(0) - hit2Pos(0)) * sqrt(2 * (pow(R1, 2) + pow(R2, 2)) / pow(R21, 2) - pow(pow(R1, 2) - pow(R2, 2), 2) / pow(R21, 4) - 1);

  points[2][0] =
      (hit2Pos(0) + hit3Pos(0)) / 2 + (pow(R2, 2) - pow(R3, 2)) * (hit3Pos(0) - hit2Pos(0)) / 2 / pow(R32, 2) +
      0.5 * (hit3Pos(1) - hit2Pos(1)) * sqrt(2 * (pow(R2, 2) + pow(R3, 2)) / pow(R32, 2) - pow(pow(R2, 2) - pow(R3, 2), 2) / pow(R32, 4) - 1);
  points[2][1] =
      (hit2Pos(1) + hit3Pos(1)) / 2 + (pow(R2, 2) - pow(R3, 2)) * (hit3Pos(1) - hit2Pos(1)) / 2 / pow(R32, 2) +
      0.5 * (hit2Pos(0) - hit3Pos(0)) * sqrt(2 * (pow(R2, 2) + pow(R3, 2)) / pow(R32, 2) - pow(pow(R2, 2) - pow(R3, 2), 2) / pow(R32, 4) - 1);
  points[3][0] =
      (hit2Pos(0) + hit3Pos(0)) / 2 + (pow(R2, 2) - pow(R3, 2)) * (hit3Pos(0) - hit2Pos(0)) / 2 / pow(R32, 2) -
      0.5 * (hit3Pos(1) - hit2Pos(1)) * sqrt(2 * (pow(R2, 2) + pow(R3, 2)) / pow(R32, 2) - pow(pow(R2, 2) - pow(R3, 2), 2) / pow(R32, 4) - 1);
  points[3][1] =
      (hit2Pos(1) + hit3Pos(1)) / 2 + (pow(R2, 2) - pow(R3, 2)) * (hit3Pos(1) - hit2Pos(1)) / 2 / pow(R32, 2) -
      0.5 * (hit2Pos(0) - hit3Pos(0)) * sqrt(2 * (pow(R2, 2) + pow(R3, 2)) / pow(R32, 2) - pow(pow(R2, 2) - pow(R3, 2), 2) / pow(R32, 4) - 1);

  points[4][0] =
      (hit1Pos(0) + hit3Pos(0)) / 2 + (pow(R3, 2) - pow(R1, 2)) * (hit1Pos(0) - hit3Pos(0)) / 2 / pow(R13, 2) +
      0.5 * (hit1Pos(1) - hit3Pos(1)) * sqrt(2 * (pow(R3, 2) + pow(R1, 2)) / pow(R13, 2) - pow(pow(R3, 2) - pow(R1, 2), 2) / pow(R13, 4) - 1);
  points[4][1] =
      (hit1Pos(1) + hit3Pos(1)) / 2 + (pow(R3, 2) - pow(R1, 2)) * (hit1Pos(1) - hit3Pos(1)) / 2 / pow(R13, 2) +
      0.5 * (hit3Pos(0) - hit1Pos(0)) * sqrt(2 * (pow(R3, 2) + pow(R1, 2)) / pow(R13, 2) - pow(pow(R3, 2) - pow(R1, 2), 2) / pow(R13, 4) - 1);
  points[5][0] =
      (hit1Pos(0) + hit3Pos(0)) / 2 + (pow(R3, 2) - pow(R1, 2)) * (hit1Pos(0) - hit3Pos(0)) / 2 / pow(R13, 2) -
      0.5 * (hit1Pos(1) - hit3Pos(1)) * sqrt(2 * (pow(R3, 2) + pow(R1, 2)) / pow(R13, 2) - pow(pow(R3, 2) - pow(R1, 2), 2) / pow(R13, 4) - 1);
  points[5][1] =
      (hit1Pos(1) + hit3Pos(1)) / 2 + (pow(R3, 2) - pow(R1, 2)) * (hit1Pos(1) - hit3Pos(1)) / 2 / pow(R13, 2) -
      0.5 * (hit3Pos(0) - hit1Pos(0)) * sqrt(2 * (pow(R3, 2) + pow(R1, 2)) / pow(R13, 2) - pow(pow(R3, 2) - pow(R1, 2), 2) / pow(R13, 4) - 1);

  return points;
}

double EventCategorizerTools::findMinimumFromDerivative(std::vector<double> x_vec, std::vector<double> y_vec)
{
  // Checking which element i of y values vecotr is a minimum, smaller than elements i-1 and i+1
  unsigned minIndex = 0;
  for (unsigned i = 1; i < y_vec.size() - 1; ++i)
  {
    if (y_vec.at(i) < y_vec.at(i - 1) && y_vec.at(i) < y_vec.at(i + 1))
    {
      minIndex = i;
      break;
    }
  }

  double a = (y_vec[minIndex + 1] - y_vec[minIndex] - (y_vec[minIndex] - y_vec[minIndex - 1])) /
             ((x_vec[minIndex + 1] + x_vec[minIndex]) / 2.0 - (x_vec[minIndex] + x_vec[minIndex - 1]) / 2.0);

  double b = y_vec[minIndex + 1] - y_vec[minIndex] - a * (x_vec[minIndex + 1] + x_vec[minIndex]) / 2.0;

  if (a > 0.0)
  {
    return -b / a;
  }
  else
  {
    return 0.0;
  }
}
