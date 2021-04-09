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
void EventCategorizerTools::selectForCalibration(const JPetEvent& event, JPetStatistics& stats, bool saveCalibHistos, double totCutAnniMin,
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

bool EventCategorizerTools::collimator2Gamma(const JPetEvent& event, JPetStatistics& stats, bool saveHistos, double maxThetaDiff, double maxTimeDiff,
                                             double totCutAnniMin, double totCutAnniMax, double lorAngleMax, double lorPosZMax,
                                             const TVector3& sourcePos, boost::property_tree::ptree& calibTree)
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

      int scin1ID = firstHit.getScin().getID();
      int scin2ID = secondHit.getScin().getID();

      int slot1ID = firstHit.getScin().getSlot().getID();
      int slot2ID = secondHit.getScin().getSlot().getID();

      int idDiff = max(scin1ID, scin2ID) - min(scin1ID, scin2ID);

      // if (max(slot1ID, slot2ID) - min(slot1ID, slot2ID) == 12 && fabs(firstHit.getTime() - secondHit.getTime()) < maxTimeDiff)
      if (idDiff > 154 && idDiff < 158 && fabs(firstHit.getTime() - secondHit.getTime()) < maxTimeDiff)
      {
        timeWalkStuff(firstHit, stats, saveHistos, calibTree);
        timeWalkStuff(secondHit, stats, saveHistos, calibTree);
      }
      return true;
    }
  }
  return false;
}

void EventCategorizerTools::timeWalkStuff(const JPetHit& hit, JPetStatistics& stats, bool saveHistos, boost::property_tree::ptree& calibTree)
{
  int scinID = hit.getScin().getID();
  int pmAID = hit.getSignalA().getPM().getID();
  int pmBID = hit.getSignalA().getPM().getID();
  auto sigMapA = hit.getSignalA().getRawSignals();
  auto sigMapB = hit.getSignalB().getRawSignals();

  double tDiffAvTHR1 = 0.0, tDiffAvAll = 0.0;
  int avCountTHR1 = 0, acCountAll = 0;

  // First calculate average timeDiff for THR1, THR2 and all
  for (int mtxPos = 1; mtxPos <= 4; mtxPos++)
  {
    auto searchA = sigMapA.find(mtxPos);
    auto searchB = sigMapB.find(mtxPos);
    if (searchA == sigMapA.end() || searchB == sigMapB.end())
    {
      continue;
    }

    auto leadsA = sigMapA.at(mtxPos).getPoints(JPetSigCh::Leading, JPetRawSignal::ByThrNum);
    auto trailsA = sigMapA.at(mtxPos).getPoints(JPetSigCh::Trailing, JPetRawSignal::ByThrNum);
    auto leadsB = sigMapB.at(mtxPos).getPoints(JPetSigCh::Leading, JPetRawSignal::ByThrNum);
    auto trailsB = sigMapB.at(mtxPos).getPoints(JPetSigCh::Trailing, JPetRawSignal::ByThrNum);

    if (leadsA.size() == 2 && leadsB.size() == 2 && trailsA.size() == 2 && trailsB.size() == 2)
    {
      // Constants for scin synchronization
      double c_tDiffTHR1 = calibTree.get(Form("%s.%d.%s%d", "scin", scinID, "hit_tdiff_thr1_scin_mtx_pos_", mtxPos), 0.0);
      double c_tDiffTHR2 = calibTree.get(Form("%s.%d.%s%d", "scin", scinID, "hit_tdiff_thr2_scin_mtx_pos_", mtxPos), 0.0);

      double tDiffTHR1 = leadsB.at(0).getTime() - leadsA.at(0).getTime() - c_tDiffTHR1;
      double tDiffTHR2 = leadsB.at(1).getTime() - leadsA.at(1).getTime() - c_tDiffTHR2;

      // Calculating averages
      tDiffAvTHR1 += tDiffTHR1;
      avCountTHR1++;
      tDiffAvAll += tDiffTHR1;
      tDiffAvAll += tDiffTHR2;
      acCountAll++;
      acCountAll++;
    }
  }

  // Final averages
  tDiffAvTHR1 = tDiffAvTHR1 / ((double)avCountTHR1);
  tDiffAvAll = tDiffAvAll / ((double)acCountAll);

  // Average reversed TOT - for all SiPMs
  double revTOTAll = 0.0;
  int revTOTCount = 0;

  // Calculating reversed TOT per SiPM mtx position and filling histograms
  for (int mtxPos = 1; mtxPos <= 4; mtxPos++)
  {
    auto searchA = sigMapA.find(mtxPos);
    auto searchB = sigMapB.find(mtxPos);
    if (searchA == sigMapA.end() || searchB == sigMapB.end())
    {
      continue;
    }

    auto leadsA = sigMapA.at(mtxPos).getPoints(JPetSigCh::Leading, JPetRawSignal::ByThrNum);
    auto trailsA = sigMapA.at(mtxPos).getPoints(JPetSigCh::Trailing, JPetRawSignal::ByThrNum);
    auto leadsB = sigMapB.at(mtxPos).getPoints(JPetSigCh::Leading, JPetRawSignal::ByThrNum);
    auto trailsB = sigMapB.at(mtxPos).getPoints(JPetSigCh::Trailing, JPetRawSignal::ByThrNum);

    // Checking time walk effect only for SiPM signals with both thresholds
    if (leadsA.size() == 2 && leadsB.size() == 2 && trailsA.size() == 2 && trailsB.size() == 2)
    {
      double c_totTHR1A_a = calibTree.get("sipm." + to_string(pmAID) + ".tot_factor_thr1_a", 1.0);
      double c_totTHR1A_b = calibTree.get("sipm." + to_string(pmAID) + ".tot_factor_thr1_b", 0.0);
      double c_totTHR2A_a = calibTree.get("sipm." + to_string(pmAID) + ".tot_factor_thr2_a", 1.0);
      double c_totTHR2A_b = calibTree.get("sipm." + to_string(pmAID) + ".tot_factor_thr2_b", 0.0);

      double c_totTHR1B_a = calibTree.get("sipm." + to_string(pmBID) + ".tot_factor_thr1_a", 1.0);
      double c_totTHR1B_b = calibTree.get("sipm." + to_string(pmBID) + ".tot_factor_thr1_b", 0.0);
      double c_totTHR2B_a = calibTree.get("sipm." + to_string(pmBID) + ".tot_factor_thr2_a", 1.0);
      double c_totTHR2B_b = calibTree.get("sipm." + to_string(pmBID) + ".tot_factor_thr2_b", 0.0);

      double totATHR1 = c_totTHR1A_a * (trailsA.at(0).getTime() - leadsA.at(0).getTime()) + c_totTHR1A_b;
      double totATHR2 = c_totTHR2A_a * (trailsA.at(1).getTime() - leadsA.at(1).getTime()) + c_totTHR2A_b;
      double totBTHR1 = c_totTHR1B_a * (trailsB.at(0).getTime() - leadsB.at(0).getTime()) + c_totTHR1B_b;
      double totBTHR2 = c_totTHR2B_a * (trailsB.at(1).getTime() - leadsB.at(1).getTime()) + c_totTHR2B_b;

      if (totATHR1 != 0.0 && totATHR2 != 0.0 && totBTHR1 != 0.0 && totBTHR2 != 0.0)
      {
        double revTOTSum = (1.0 / totBTHR1 + 1.0 / totBTHR2) - (1.0 / totATHR1 + 1.0 / totATHR2);
        revTOTAll += revTOTSum;
        revTOTCount++;

        // Filling histograms before application of corrections
        // if (saveHistos)
        // {
        //   stats.getHisto2D(Form("time_walk_thr1_mtx_%d", mtxPos))->Fill(tDiffAvTHR1, revTOTSum);
        //   stats.getHisto2D(Form("time_walk_thr2_mtx_%d", mtxPos))->Fill(tDiffAvAll, revTOTSum);
        // }

        // Using corrections
        double c_twTHR1_a = calibTree.get("time_walk.thr1_mtx" + to_string(mtxPos) + ".param_a", 1.0);
        double c_twTHR1_b = calibTree.get("time_walk.thr1_mtx" + to_string(mtxPos) + ".param_b", 0.0);
        double c_twTHR2_a = calibTree.get("time_walk.thr2_mtx" + to_string(mtxPos) + ".param_a", 1.0);
        double c_twTHR2_b = calibTree.get("time_walk.thr2_mtx" + to_string(mtxPos) + ".param_b", 0.0);

        double tDiffTHR1_corr = tDiffAvTHR1 - (revTOTSum * c_twTHR1_a + c_twTHR1_b);
        double tDiffTHR2_corr = tDiffAvAll - (revTOTSum * c_twTHR2_a + c_twTHR2_b);

        // if (saveHistos)
        // {
        //   stats.getHisto2D(Form("time_walk_thr1_mtx_%d_corr", mtxPos))->Fill(tDiffTHR1_corr, revTOTSum);
        //   stats.getHisto2D(Form("time_walk_thr2_mtx_%d_corr", mtxPos))->Fill(tDiffTHR2_corr, revTOTSum);
        // }
      }
    }
  }

  // Reversed TOT average
  revTOTAll = revTOTAll / ((double)revTOTCount);
  if (saveHistos)
  {
    int mtxPos = 1;
    // Before correction
    // stats.getHisto2D(Form("time_walk_sum_mtx_%d", mtxPos))->Fill(tDiffAvAll, revTOTAll);
    // Corrected
    double c_twSum_a = calibTree.get("time_walk.sum_mtx" + to_string(mtxPos) + ".param_a", 1.0);
    double c_twSum_b = calibTree.get("time_walk.sum_mtx" + to_string(mtxPos) + ".param_b", 0.0);
    double tDiffAvAll_corr = tDiffAvAll - (revTOTAll * c_twSum_a + c_twSum_b);
    // stats.getHisto2D(Form("time_walk_sum_mtx_%d_corr", mtxPos))->Fill(tDiffAvAll_corr, revTOTAll);
  }
}

/**
 * Method for determining type of event - back to back 2 gamma
 */
bool EventCategorizerTools::checkFor2Gamma(const JPetEvent& event, JPetStatistics& stats, bool saveHistos, double maxThetaDiff, double maxTimeDiff,
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

      if (checkFor2Gamma(firstHit, secondHit, stats, saveHistos, maxThetaDiff, maxTimeDiff, totCutAnniMin, totCutAnniMax, lorAngleMax, lorPosZMax,
                         sourcePos))
      {
        isEvent2Gamma = true;
      }
    }
  }
  return isEvent2Gamma;
}

/**
 * Method for selecting pairs of annihilation hits
 */
vector<JPetEvent> EventCategorizerTools::getLORs(const JPetEvent& event, JPetStatistics& stats, bool saveHistos, double maxThetaDiff,
                                                 double maxTimeDiff, double totCutAnniMin, double totCutAnniMax, double lorAngleMax,
                                                 double lorPosZMax, const TVector3& sourcePos)
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

      auto multi1 = firstHit.getSignalA().getRawSignals().size() + firstHit.getSignalB().getRawSignals().size();
      auto multi2 = secondHit.getSignalA().getRawSignals().size() + secondHit.getSignalB().getRawSignals().size();

      if (multi1 + multi2 < 8)
      {
        continue;
      }

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

      if (checkFor2Gamma(firstHit, secondHit, stats, saveHistos, maxThetaDiff, maxTimeDiff, totCutAnniMin, totCutAnniMax, lorAngleMax, lorPosZMax,
                         sourcePos))
      {
        JPetEvent lor;
        lor.addHit(firstHit);
        lor.addHit(secondHit);
        lor.addEventType(JPetEventType::k2Gamma);
        lors.push_back(lor);
      }
    }
  }
  return lors;
}

/**
 * Method for determining type of two hits - back to back 2 gamma
 */
bool EventCategorizerTools::checkFor2Gamma(const JPetHit& firstHit, const JPetHit& secondHit, JPetStatistics& stats, bool saveHistos,
                                           double maxThetaDiff, double maxTimeDiff, double totCutAnniMin, double totCutAnniMax, double lorAngleMax,
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
  TVector3 vecHit1_2D(firstHit.getPosX() - sourcePos.X(), 0.0, firstHit.getPosZ() - sourcePos.Z());
  TVector3 vecHit2_2D(secondHit.getPosX() - sourcePos.X(), 0.0, secondHit.getPosZ() - sourcePos.Z());
  TVector3 vecHit1_1D(firstHit.getPosX() - sourcePos.X(), 0.0, 0.0);
  TVector3 vecHit2_1D(secondHit.getPosX() - sourcePos.X(), 0.0, 0.0);

  double lorAngle1 = TMath::RadToDeg() * vecHit1_2D.Angle(vecHit1_1D);
  double lorAngle2 = TMath::RadToDeg() * vecHit2_2D.Angle(vecHit2_2D);

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

    stats.getHisto1D("2g_hit_tdiff")->Fill(firstHit.getTimeDiff());
    stats.getHisto1D("2g_hit_tdiff")->Fill(secondHit.getTimeDiff());
    stats.getHisto2D("2g_hit_tdiff_scin")->Fill(scin1ID, firstHit.getTimeDiff());
    stats.getHisto2D("2g_hit_tdiff_scin")->Fill(scin1ID, secondHit.getTimeDiff());
    stats.getHisto2D("2g_hit_tdiff_z_pos")->Fill(firstHit.getPosZ(), firstHit.getTimeDiff());
    stats.getHisto2D("2g_hit_tdiff_z_pos")->Fill(secondHit.getPosZ(), secondHit.getTimeDiff());

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

  if (180.0 - theta < maxThetaDiff)
  {
    thetaCut2 = true;
    if (saveHistos)
    {
      stats.getHisto1D("cut_stats_a2")->Fill(scin1ID);
      stats.getHisto1D("cut_stats_a2")->Fill(scin2ID);
      stats.getHisto1D("theta_cut_tof")->Fill(tof);
      stats.getHisto1D("theta_cut_tot")->Fill(tot1);
      stats.getHisto1D("theta_cut_tot")->Fill(tot2);
      stats.getHisto1D("theta_cut_hit_tdiff")->Fill(firstHit.getTimeDiff());
      stats.getHisto1D("theta_cut_hit_tdiff")->Fill(secondHit.getTimeDiff());
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
      stats.getHisto1D("tof_cut_hit_tdiff")->Fill(firstHit.getTimeDiff());
      stats.getHisto1D("tof_cut_hit_tdiff")->Fill(secondHit.getTimeDiff());
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
      stats.getHisto1D("tot_cut_hit_tdiff")->Fill(firstHit.getTimeDiff());
      stats.getHisto1D("tot_cut_hit_tdiff")->Fill(secondHit.getTimeDiff());
      stats.getHisto1D("tot_cut_theta")->Fill(theta);
    }
  }

  // Pair of hits that meet cut conditions are treated as coming from annihilation point
  if (saveHistos && tDiffCut && thetaCut2)
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

    stats.getHisto1D("ap_hit_tdiff")->Fill(firstHit.getTimeDiff());
    stats.getHisto1D("ap_hit_tdiff")->Fill(secondHit.getTimeDiff());
    stats.getHisto2D("ap_hit_tdiff_scin")->Fill(scin1ID, firstHit.getTimeDiff());
    stats.getHisto2D("ap_hit_tdiff_scin")->Fill(scin2ID, secondHit.getTimeDiff());
    stats.getHisto2D("ap_hit_tdiff_z_pos")->Fill(firstHit.getPosZ(), firstHit.getTimeDiff());
    stats.getHisto2D("ap_hit_tdiff_z_pos")->Fill(secondHit.getPosZ(), secondHit.getTimeDiff());

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
    double tot = calculateTOT(event.getHits().at(i));
    if (tot > deexTOTCutMin && tot < deexTOTCutMax)
    {
      if (saveHistos)
      {
        stats.getHisto1D("Deex_TOT_cut")->Fill(tot);
      }
      return true;
    }
  }
  return false;
}

/**
 * Method for determining type of event - scatter
 */
bool EventCategorizerTools::checkForScatter(const JPetEvent& event, JPetStatistics& stats, bool saveHistos, double scatterTOFTimeDiff)
{
  if (event.getHits().size() < 2)
  {
    return false;
  }
  for (uint i = 0; i < event.getHits().size(); i++)
  {
    for (uint j = i + 1; j < event.getHits().size(); j++)
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

      double scattAngle = calculateScatteringAngle(primaryHit, scatterHit);
      double scattTOF = calculateScatteringTime(primaryHit, scatterHit);
      double timeDiff = scatterHit.getTime() - primaryHit.getTime();

      if (saveHistos)
      {
        stats.getHisto1D("ScatterTOF_TimeDiff")->Fill(fabs(scattTOF - timeDiff));
      }

      if (fabs(scattTOF - timeDiff) < scatterTOFTimeDiff)
      {
        if (saveHistos)
        {
          stats.getHisto2D("ScatterAngle_PrimaryTOT")->Fill(scattAngle, calculateTOT(primaryHit));
          stats.getHisto2D("ScatterAngle_ScatterTOT")->Fill(scattAngle, calculateTOT(scatterHit));
        }
        return true;
      }
    }
  }
  return false;
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
  double revTOT_sigA = 0.0, revTOT_sigB = 0.0;
  for (auto& rawSig : hit.getSignalA().getRawSignals())
  {
    for (auto& thrTOT : rawSig.second.getTOTsVsThresholdNumber())
    {
      if (thrTOT.second != 0.0)
      {
        revTOT_sigA += 1.0 / thrTOT.second;
      }
    }
  }

  for (auto& rawSig : hit.getSignalB().getRawSignals())
  {
    for (auto& thrTOT : rawSig.second.getTOTsVsThresholdNumber())
    {
      if (thrTOT.second != 0.0)
      {
        revTOT_sigB += 1.0 / thrTOT.second;
      }
    }
  }

  return revTOT_sigB - revTOT_sigA;
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
 * Calculation point in 3D, where annihilation occured
 */
TVector3 EventCategorizerTools::calculateAnnihilationPoint(const JPetHit& hitA, const JPetHit& hitB)
{
  double tof = EventCategorizerTools::calculateTOF(hitA, hitB);
  return calculateAnnihilationPoint(hitA.getPos(), hitB.getPos(), tof);
}

TVector3 EventCategorizerTools::calculateAnnihilationPoint(const TVector3& hitA, const TVector3& hitB, double tof)
{
  TVector3 middleOfLOR = 0.5 * (hitA + hitB);
  TVector3 versorOnLOR = (hitB - hitA).Unit();

  double shift = 0.5 * tof * kLightVelocity_cm_ps;
  TVector3 annihilationPoint(middleOfLOR.X() + shift * versorOnLOR.X(), middleOfLOR.Y() + shift * versorOnLOR.Y(),
                             middleOfLOR.Z() + shift * versorOnLOR.Z());
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
bool EventCategorizerTools::stream2Gamma(const JPetEvent& event, JPetStatistics& stats, bool saveHistos, double b2bSlotThetaDiff, double b2bTimeDiff)
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
      // Checking for back to back
      double timeDiff = fabs(firstHit.getTime() - secondHit.getTime());
      double deltaLor = (secondHit.getTime() - firstHit.getTime()) * kLightVelocity_cm_ps / 2.;
      double theta1 = min(firstHit.getScin().getSlot().getTheta(), secondHit.getScin().getSlot().getTheta());
      double theta2 = max(firstHit.getScin().getSlot().getTheta(), secondHit.getScin().getSlot().getTheta());
      double thetaDiff = min(theta2 - theta1, 360.0 - theta2 + theta1);
      if (saveHistos)
      {
        stats.getHisto1D("2Gamma_TimeDiff")->Fill(timeDiff / 1000.0);
        stats.getHisto1D("2Gamma_DLOR")->Fill(deltaLor);
        stats.getHisto1D("2Gamma_ThetaDiff")->Fill(thetaDiff);
      }
      if (fabs(thetaDiff - 180.0) < b2bSlotThetaDiff && timeDiff < b2bTimeDiff)
      {
        if (saveHistos)
        {
          TVector3 annhilationPoint = calculateAnnihilationPoint(firstHit, secondHit);
          stats.getHisto1D("2Annih_TimeDiff")->Fill(timeDiff / 1000.0);
          stats.getHisto1D("2Annih_DLOR")->Fill(deltaLor);
          stats.getHisto1D("2Annih_ThetaDiff")->Fill(thetaDiff);
          stats.getHisto2D("2Annih_XY")->Fill(annhilationPoint.X(), annhilationPoint.Y());
          stats.getHisto1D("2Annih_Z")->Fill(annhilationPoint.Z());
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
bool EventCategorizerTools::stream3Gamma(const JPetEvent& event, JPetStatistics& stats, bool saveHistos, double d3SlotThetaMin, double d3TimeDiff,
                                         double d3PlaneCenterDist)
{
  if (event.getHits().size() < 3)
  {
    return false;
  }
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
        double timeDiff = fabs(thirdHit.getTime() - firstHit.getTime());
        double planeCenterDist = calculatePlaneCenterDistance(firstHit, secondHit, thirdHit);
        if (saveHistos)
        {
          stats.getHisto1D("3GammaTimeDiff")->Fill(timeDiff);
          stats.getHisto2D("3GammaThetas")->Fill(transformedX, transformedY);
          stats.getHisto1D("3GammaPlaneDist")->Fill(planeCenterDist);
        }
        if (transformedX > d3SlotThetaMin && timeDiff < d3TimeDiff && planeCenterDist < d3PlaneCenterDist)
        {
          if (saveHistos)
          {
            stats.getHisto1D("3AnnihPlaneDist")->Fill(planeCenterDist);
            stats.getHisto1D("3AnnihTimeDiff")->Fill(timeDiff);
          }
          return true;
        }
      }
    }
  }
  return false;
}
