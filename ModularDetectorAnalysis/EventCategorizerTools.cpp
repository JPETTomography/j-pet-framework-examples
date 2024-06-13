/**
 *  @copyright Copyright 2024 The J-PET Framework Authors. All rights reserved.
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
#include <Hits/JPetPhysRecoHit/JPetPhysRecoHit.h>
#include <Math/DistFunc.h>
#include <TMath.h>
#include <TRandom.h>
#include <vector>

using namespace std;

/**
 * Method for determining type of event - back to back 2 gamma
 */
bool EventCategorizerTools::checkFor2Gamma(const JPetEvent& event, JPetStatistics& stats, bool saveHistos, double maxThetaDiff, double maxTimeDiff,
                                           double totCutAnniMin, double totCutAnniMax, const TVector3& sourcePos, ScatterTestType testType,
                                           double scatterTestValue, double scatterTimeMin, double scatterTimeMax, double scatterAngleMin,
                                           double scatterAngleMax)
{
  bool isEvent2Gamma = false;
  if (event.getHits().size() < 2)
  {
    return isEvent2Gamma;
  }

  for (uint i = 0; i < event.getHits().size(); i++)
  {
    auto firstHit = dynamic_cast<const JPetPhysRecoHit*>(event.getHits().at(i));
    if (!firstHit)
    {
      continue;
    }

    for (uint j = i + 1; j < event.getHits().size(); j++)
    {
      auto secondHit = dynamic_cast<const JPetPhysRecoHit*>(event.getHits().at(j));
      if (!secondHit)
      {
        continue;
      }

      // Change order or hits, if needed
      if (event.getHits().at(i)->getTime() > event.getHits().at(j)->getTime())
      {
        firstHit = dynamic_cast<const JPetPhysRecoHit*>(event.getHits().at(j));
        secondHit = dynamic_cast<const JPetPhysRecoHit*>(event.getHits().at(i));
      }

      // Skip if scatter
      if (checkForScatter(firstHit, secondHit, stats, true, testType, scatterTestValue, scatterTimeMin, scatterTimeMax, scatterAngleMin,
                          scatterAngleMax))
      {
        continue;
      }

      if (checkFor2Gamma(firstHit, secondHit, stats, saveHistos, maxThetaDiff, maxTimeDiff, totCutAnniMin, totCutAnniMax, sourcePos))
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
bool EventCategorizerTools::checkFor2Gamma(const JPetPhysRecoHit* firstHit, const JPetPhysRecoHit* secondHit, JPetStatistics& stats, bool saveHistos,
                                           double maxThetaDiff, double maxTimeDiff, double totCutAnniMin, double totCutAnniMax,
                                           const TVector3& sourcePos)
{
  int scin1ID = firstHit->getScin().getID();
  int scin2ID = secondHit->getScin().getID();

  TVector3 firstVec = firstHit->getPos() - sourcePos;
  TVector3 secondVec = secondHit->getPos() - sourcePos;
  double theta = TMath::RadToDeg() * firstVec.Angle(secondVec);

  // Registration time difference, always positive
  double timeDiff = fabs(firstHit->getTime() - secondHit->getTime());

  auto tot1 = firstHit->getToT();
  auto tot2 = secondHit->getToT();

  // TOF calculated by convention
  double tof = calculateTOFByConvention(firstHit, secondHit);

  // LOR angle
  TVector3 vechit1_2D(firstHit->getPosX() - sourcePos.X(), 0.0, firstHit->getPosZ() - sourcePos.Z());
  TVector3 vechit2_2D(secondHit->getPosX() - sourcePos.X(), 0.0, secondHit->getPosZ() - sourcePos.Z());
  TVector3 vechit1_1D(firstHit->getPosX() - sourcePos.X(), 0.0, 0.0);
  TVector3 vechit2_1D(secondHit->getPosX() - sourcePos.X(), 0.0, 0.0);

  // Pre-cuts histograms
  if (saveHistos)
  {
    stats.fillHistogram("2g_tot", tot1);
    stats.fillHistogram("2g_tot", tot2);
    stats.fillHistogram("2g_tot_z_pos", firstHit->getPosZ(), tot1);
    stats.fillHistogram("2g_tot_z_pos", secondHit->getPosZ(), tot2);
    stats.fillHistogram("2g_tot_scin", scin1ID, tot1);
    stats.fillHistogram("2g_tot_scin", scin2ID, tot2);

    stats.fillHistogram("2g_tof", tof);
    stats.fillHistogram("2g_tof_scin", scin1ID, tof);
    stats.fillHistogram("2g_tof_scin", scin2ID, tof);

    stats.fillHistogram("2g_ab_tdiff", firstHit->getTimeDiff());
    stats.fillHistogram("2g_ab_tdiff", secondHit->getTimeDiff());
    stats.fillHistogram("2g_ab_tdiff_scin", scin1ID, firstHit->getTimeDiff());
    stats.fillHistogram("2g_ab_tdiff_scin", scin2ID, secondHit->getTimeDiff());

    stats.fillHistogram("2g_ab_tdiff_tot", firstHit->getTimeDiff(), tot1);
    stats.fillHistogram("2g_ab_tdiff_tot", secondHit->getTimeDiff(), tot2);

    stats.fillHistogram("2g_theta", theta);
    stats.fillHistogram("2g_theta_scin", scin1ID, theta);
    stats.fillHistogram("2g_theta_scin", scin2ID, theta);
    stats.fillHistogram("2g_theta_z_pos", firstHit->getPosZ(), theta);
    stats.fillHistogram("2g_theta_z_pos", secondHit->getPosZ(), theta);

    stats.fillHistogram("cut_stats_none", scin1ID);
    stats.fillHistogram("cut_stats_none", scin2ID);
  }

  // Checking selection conditions
  bool thetaCut = checkRelativeAngles(firstHit->getPos(), secondHit->getPos(), maxThetaDiff);
  bool tDiffCut = false, totCut = false;

  if (thetaCut)
  {
    if (saveHistos)
    {
      stats.fillHistogram("cut_stats_angle", scin1ID);
      stats.fillHistogram("cut_stats_angle", scin2ID);
      stats.fillHistogram("theta_cut_tof", tof);
      stats.fillHistogram("theta_cut_tot", tot1);
      stats.fillHistogram("theta_cut_tot", tot2);
      stats.fillHistogram("theta_cut_ab_tdiff", firstHit->getTimeDiff());
      stats.fillHistogram("theta_cut_ab_tdiff", secondHit->getTimeDiff());
    }
  }

  // Time difference cut
  if (timeDiff < maxTimeDiff)
  {
    tDiffCut = true;
    if (saveHistos)
    {
      stats.fillHistogram("tof_cut_tot", tot1);
      stats.fillHistogram("tof_cut_tot", tot2);
      stats.fillHistogram("tof_cut_ab_tdiff", firstHit->getTimeDiff());
      stats.fillHistogram("tof_cut_ab_tdiff", secondHit->getTimeDiff());
      stats.fillHistogram("tof_cut_theta", theta);
    }
  }

  // ToT cut
  if (tot1 > totCutAnniMin && tot1 < totCutAnniMax && tot2 > totCutAnniMin && tot2 < totCutAnniMax)
  {
    totCut = true;
    if (saveHistos)
    {
      stats.fillHistogram("cut_stats_tot", scin1ID);
      stats.fillHistogram("cut_stats_tot", scin2ID);
      stats.fillHistogram("tot_cut_tof", tof);
      stats.fillHistogram("tot_cut_ab_tdiff", firstHit->getTimeDiff());
      stats.fillHistogram("tot_cut_ab_tdiff", secondHit->getTimeDiff());
      stats.fillHistogram("tot_cut_theta", theta);
    }
  }

  // Pair of hits that meet cut conditions are treated as coming from annihilation point
  // Returning event as 2 gamma if meets cut conditions
  if (totCut && tDiffCut && thetaCut)
  {
    if (saveHistos)
    {
      TVector3 annhilationPoint = calculateAnnihilationPoint(firstHit, secondHit);

      stats.fillHistogram("ap_tot", tot1);
      stats.fillHistogram("ap_tot", tot2);
      stats.fillHistogram("ap_tot_scin", scin1ID, tot1);
      stats.fillHistogram("ap_tot_scin", scin2ID, tot2);

      stats.fillHistogram("ap_ab_tdiff", firstHit->getTimeDiff());
      stats.fillHistogram("ap_ab_tdiff", secondHit->getTimeDiff());
      stats.fillHistogram("ap_ab_tdiff_scin", scin1ID, firstHit->getTimeDiff());
      stats.fillHistogram("ap_ab_tdiff_scin", scin2ID, secondHit->getTimeDiff());

      stats.fillHistogram("ap_ab_tdiff_tot", firstHit->getTimeDiff(), tot1);
      stats.fillHistogram("ap_ab_tdiff_tot", secondHit->getTimeDiff(), tot2);

      stats.fillHistogram("ap_tof", tof);
      stats.fillHistogram("ap_tof_scin", scin1ID, tof);
      stats.fillHistogram("ap_tof_scin", scin2ID, tof);

      stats.fillHistogram("ap_theta", theta);
      stats.fillHistogram("ap_theta_scin", scin1ID, theta);
      stats.fillHistogram("ap_theta_scin", scin2ID, theta);

      stats.fillHistogram("ap_xy", annhilationPoint.X(), annhilationPoint.Y());
      stats.fillHistogram("ap_zx", annhilationPoint.Z(), annhilationPoint.X());
      stats.fillHistogram("ap_zy", annhilationPoint.Z(), annhilationPoint.Y());
      stats.fillHistogram("ap_pos", annhilationPoint.Z(), annhilationPoint.X(), annhilationPoint.Y());
      stats.fillHistogram("ap_xy_zoom", annhilationPoint.X(), annhilationPoint.Y());
      stats.fillHistogram("ap_zx_zoom", annhilationPoint.Z(), annhilationPoint.X());
      stats.fillHistogram("ap_zy_zoom", annhilationPoint.Z(), annhilationPoint.Y());
      stats.fillHistogram("ap_pos_zoom", annhilationPoint.Z(), annhilationPoint.X(), annhilationPoint.Y());
    }
    return true;
  }
  return false;
}

/**
 * Method for determining type of event - 3Gamma
 */
bool EventCategorizerTools::checkFor3Gamma(const JPetEvent& event, double minRelAngleCut, JPetStatistics& stats, bool saveHistos)
{
  if (event.getHits().size() < 3)
  {
    return false;
  }

  double is3Gamma = false;

  // Iteration over the hits in the event
  for (uint i = 0; i < event.getHits().size(); i++)
  {
    for (uint j = i + 1; j < event.getHits().size(); j++)
    {
      for (uint k = j + 1; k < event.getHits().size(); k++)
      {
        auto firstHit = dynamic_cast<const JPetPhysRecoHit*>(event.getHits().at(i));
        auto secondHit = dynamic_cast<const JPetPhysRecoHit*>(event.getHits().at(j));
        auto thirdHit = dynamic_cast<const JPetPhysRecoHit*>(event.getHits().at(k));

        vector<double> relativeAngles;
        relativeAngles.push_back(TMath::RadToDeg() * firstHit->getPos().Angle(secondHit->getPos()));
        relativeAngles.push_back(TMath::RadToDeg() * secondHit->getPos().Angle(thirdHit->getPos()));
        relativeAngles.push_back(TMath::RadToDeg() * thirdHit->getPos().Angle(firstHit->getPos()));
        sort(relativeAngles.begin(), relativeAngles.end());

        double transformedX = relativeAngles.at(1) + relativeAngles.at(0);
        double transformedY = relativeAngles.at(1) - relativeAngles.at(0);

        // TODO Estimate annihilation position from trilateration method (commented code)

        if (saveHistos)
        {
          stats.fillHistogram("3g_rel_angles", transformedX, transformedY);
        }

        if (transformedX > minRelAngleCut)
        {
          is3Gamma = true;
          if (saveHistos)
          {
            stats.fillHistogram("3g_rel_angles_sel", transformedX, transformedY);
          }
        }
      }
    }
  }
  return is3Gamma;
}

bool EventCategorizerTools::checkFor2GammaLifetime(const JPetEvent& event, JPetStatistics& stats, bool saveHistos, double maxThetaDiff,
                                                   double maxTimeDiff, double totCutAnniMin, double totCutAnniMax, double totCutDeexMin,
                                                   double totCutDeexMax, const TVector3& sourcePos, ScatterTestType testType, double scatterTestValue,
                                                   double scatterTimeMin, double scatterTimeMax, double scatterAngleMin, double scatterAngleMax)
{
  if (event.getHits().size() < 3)
  {
    return false;
  }

  vector<const JPetPhysRecoHit*> prompts;
  vector<pair<const JPetPhysRecoHit*, const JPetPhysRecoHit*>> annihilaions;

  // First check if any of the hits in the event is prompt based on TOT selection
  for (uint i = 0; i < event.getHits().size(); i++)
  {
    auto promptHit = dynamic_cast<const JPetPhysRecoHit*>(event.getHits().at(i));
    if (checkToT(promptHit, totCutDeexMin, totCutDeexMax))
    {
      prompts.push_back(promptHit);
    }
  }
  if (prompts.size() == 0)
  {
    return false;
  }

  // Then looking for annihilation back to back pairs
  for (uint i = 0; i < event.getHits().size(); i++)
  {
    auto firstHit = dynamic_cast<const JPetPhysRecoHit*>(event.getHits().at(i));
    if (!firstHit)
    {
      continue;
    }

    for (uint j = i + 1; j < event.getHits().size(); j++)
    {
      auto secondHit = dynamic_cast<const JPetPhysRecoHit*>(event.getHits().at(j));
      if (!secondHit)
      {
        continue;
      }

      // Change order or hits, if needed
      if (event.getHits().at(i)->getTime() > event.getHits().at(j)->getTime())
      {
        firstHit = dynamic_cast<const JPetPhysRecoHit*>(event.getHits().at(j));
        secondHit = dynamic_cast<const JPetPhysRecoHit*>(event.getHits().at(i));
      }

      // Skip if scatter
      if (checkForScatter(firstHit, secondHit, stats, false, testType, scatterTestValue, scatterTimeMin, scatterTimeMax, scatterAngleMin,
                          scatterAngleMax))
      {
        continue;
      }

      if (checkFor2Gamma(firstHit, secondHit, stats, false, maxThetaDiff, maxTimeDiff, totCutAnniMin, totCutAnniMax, sourcePos))
      {
        annihilaions.push_back(make_pair(firstHit, secondHit));
      }
    }
  }

  if (annihilaions.size() == 0)
  {
    return false;
  }

  bool isLifetimeEvent = false;

  // Iterating over all combinations of found pairs and prompt photons
  for (auto pair2g : annihilaions)
  {
    for (auto prompt : prompts)
    {
      // Check if neighter of the two annihilation photons are scattered from prompt photon
      if (checkForScatter(prompt, pair2g.first, stats, false, testType, scatterTestValue, scatterTimeMin, scatterTimeMax, scatterAngleMin,
                          scatterAngleMax) ||
          checkForScatter(prompt, pair2g.second, stats, false, testType, scatterTestValue, scatterTimeMin, scatterTimeMax, scatterAngleMin,
                          scatterAngleMax))
      {
        continue;
      }

      isLifetimeEvent = true;

      // Calculate the annihilation point position
      TVector3 annhilationPoint = calculateAnnihilationPoint(pair2g.first, pair2g.second);

      // Caculate event times - annihilation
      auto tof = calculateTOFByConvention(pair2g.first, pair2g.second);
      double annihTime1 = pair2g.first->getTime() - tof;
      double annihTime2 = pair2g.second->getTime() - tof;
      double promptTime = prompt->getTime() - tof;
      // Calculate lifetime
      double lifetime = (annihTime1 + annihTime2) / 2.0 - promptTime;

      if (saveHistos)
      {
        stats.fillHistogram("lifetime_2g_prompt", lifetime);
        stats.fillHistogram("lifetime_2g_prompt_zoom", lifetime);

        stats.fillHistogram("lifetime_ap_xy", annhilationPoint.X(), annhilationPoint.Y());
        stats.fillHistogram("lifetime_ap_zx", annhilationPoint.Z(), annhilationPoint.X());
        stats.fillHistogram("lifetime_ap_zy", annhilationPoint.Z(), annhilationPoint.Y());
        stats.fillHistogram("lifetime_ap_pos", annhilationPoint.Z(), annhilationPoint.X(), annhilationPoint.Y());
        stats.fillHistogram("lifetime_ap_xy_zoom", annhilationPoint.X(), annhilationPoint.Y());
        stats.fillHistogram("lifetime_ap_zx_zoom", annhilationPoint.Z(), annhilationPoint.X());
        stats.fillHistogram("lifetime_ap_zy_zoom", annhilationPoint.Z(), annhilationPoint.Y());
        stats.fillHistogram("lifetime_ap_pos_zoom", annhilationPoint.Z(), annhilationPoint.X(), annhilationPoint.Y());
      }
    }
  }

  return isLifetimeEvent;
}

bool EventCategorizerTools::checkToT(const JPetPhysRecoHit* hit, double minToT, double maxToT)
{
  return (hit->getToT() > minToT && hit->getToT() < maxToT);
}

bool EventCategorizerTools::checkRelativeAngles(const TVector3& pos1, const TVector3& pos2, double maxThetaDiff)
{
  double theta = TMath::RadToDeg() * pos1.Angle(pos2);
  return (180.0 - theta < maxThetaDiff);
}

/**
 * Checking if pair of hits meet scattering condition
 */
bool EventCategorizerTools::checkForScatter(const JPetBaseHit* primaryHit, const JPetBaseHit* scatterHit, JPetStatistics& stats, bool saveHistos,
                                            ScatterTestType testType, double scatterTestValue, double scatterTimeMin, double scatterTimeMax,
                                            double scatterAngleMin, double scatterAngleMax)
{
  bool isScatter = false;

  double dist = calculateDistance(primaryHit, scatterHit);
  double timeDiff = scatterHit->getTime() - primaryHit->getTime();
  double testTimeRel = timeDiff - dist / kLightVelocity_cm_ps;
  double testTimeAbs = fabs(testTimeRel);
  double testDistRel = timeDiff * kLightVelocity_cm_ps - dist;
  double testDistAbs = fabs(testDistRel);
  double scatterAngle = calculateScatteringAngle(primaryHit, scatterHit);

  if (saveHistos)
  {
    stats.fillHistogram("scatter_test_time_rel", testTimeRel);
    stats.fillHistogram("scatter_test_time_abs", testTimeAbs);
    stats.fillHistogram("scatter_test_dist_rel", testDistRel);
    stats.fillHistogram("scatter_test_dist_abs", testDistAbs);

    stats.fillHistogram("scatter_angle_time", testTimeRel, scatterAngle);
    stats.fillHistogram("scatter_angle_time_small", testTimeRel, scatterAngle);
  }

  if (testType == EventCategorizerTools::kSimpleParam)
  {
    isScatter = testTimeAbs < scatterTestValue;
  }

  if (testType == EventCategorizerTools::kMinMaxParams)
  {
    isScatter = !(scatterTimeMin < testTimeRel && testTimeRel < scatterTimeMax && scatterAngleMin < scatterAngle && scatterAngle < scatterAngleMax);
  }

  if (saveHistos)
  {
    if (isScatter)
    {
      stats.fillHistogram("scatter_test_rel_pass", testTimeRel);
      stats.fillHistogram("scatter_test_abs_pass", testTimeAbs);
      stats.fillHistogram("scatter_angle_time_pass", testTimeRel, scatterAngle);
    }
    else
    {
      stats.fillHistogram("scatter_test_rel_fail", testTimeRel);
      stats.fillHistogram("scatter_test_abs_fail", testTimeAbs);
      stats.fillHistogram("scatter_angle_time_fail", testTimeRel, scatterAngle);
    }
  }

  return isScatter;
}

/**
 * Calculation of distance between two hits
 */
double EventCategorizerTools::calculateDistance(const JPetBaseHit* hit1, const JPetBaseHit* hit2) { return (hit1->getPos() - hit2->getPos()).Mag(); }

/**
 * Calculation of time that light needs to travel the distance between primary gamma
 * and scattered gamma. Return value in picoseconds.
 */
double EventCategorizerTools::calculateScatteringTime(const JPetBaseHit* hit1, const JPetBaseHit* hit2)
{
  return calculateDistance(hit1, hit2) / kLightVelocity_cm_ps;
}

/**
 * Calculation of scatter angle between primary hit and scattered hit.
 * This function assumes that source of first gamma was in (0,0,0).
 * Angle is calculated from scalar product, return value in degrees.
 */
double EventCategorizerTools::calculateScatteringAngle(const JPetBaseHit* hit1, const JPetBaseHit* hit2)
{
  return TMath::RadToDeg() * hit1->getPos().Angle(hit2->getPos() - hit1->getPos());
}

double EventCategorizerTools::calculateTOFByConvention(const JPetBaseHit* hitA, const JPetBaseHit* hitB)
{
  if (hitA->getScin().getSlot().getTheta() < hitB->getScin().getSlot().getTheta())
  {
    return calculateTOF(hitA, hitB);
  }
  else
  {
    return calculateTOF(hitB, hitA);
  }
}

double EventCategorizerTools::calculateTOF(const JPetBaseHit* hitA, const JPetBaseHit* hitB)
{
  return EventCategorizerTools::calculateTOF(hitA->getTime(), hitB->getTime());
}

double EventCategorizerTools::calculateTOF(double time1, double time2) { return (time1 - time2); }

/**
 * @brief Calculation of an annihilation point based on LOR and TOFof two hits.
 *
 * Line of Response between two hits is used to estimate annihilation point by calculating its
 * middle and shifting it along LOR based on Time of Flight value toward one hit or the other.
 */
TVector3 EventCategorizerTools::calculateAnnihilationPoint(const JPetBaseHit* hit1, const JPetBaseHit* hit2)
{
  TVector3 middleOfLOR = 0.5 * (hit1->getPos() + hit2->getPos());
  TVector3 versorOnLOR = (hit2->getPos() - hit1->getPos()).Unit();

  double tof = EventCategorizerTools::calculateTOF(hit1, hit2);
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
/*TVector3 EventCategorizerTools::calculateAnnihilationPoint(const JPetBaseHit& hit1, const JPetBaseHit& hit2, const JPetBaseHit& hit3)
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
}*/

/**
 * Calculating distance from the center of the decay plane
 */
/*double EventCategorizerTools::calculatePlaneCenterDistance(const JPetBaseHit& firstHit, const JPetBaseHit& secondHit, const JPetBaseHit& thirdHit)
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
}*/

/**
 * Method for determining type of event for streaming - 3 gamma annihilation
 */
/*bool EventCategorizerTools::stream3Gamma(const JPetEvent& event, JPetStatistics& stats, bool saveHistos, double d3SlotthetaMin, double d3TimeDiff,
                                         double d3PlaneCenterDist, double maxScatter)
{
  if (event.getHits().size() < 3)
  {
    return false;
  }
  for (uint i = 0; i < event.getHits().size(); i++)
  {
    JPetBaseHit firstHit = event.getHits().at(i);

    for (uint j = i + 1; j < event.getHits().size(); j++)
    {
      JPetBaseHit secondHit = event.getHits().at(j);

      if (checkForScatter(firstHit, secondHit, stats, saveHistos, maxScatter))
      {
        continue;
      }

      for (uint k = j + 1; k < event.getHits().size(); k++)
      {
        JPetBaseHit thirdHit = event.getHits().at(k);

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
}*/

/**
 * Helper method for estimating anihilation point
 */
/*TVector3 EventCategorizerTools::findIntersection(TVector3 hit1Pos, TVector3 hit2Pos, TVector3 hit3Pos, double t21, double t31)
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
}*/

/*vector<vector<double>> EventCategorizerTools::findIntersectiosOfCircles(TVector3 hit1Pos, TVector3 hit2Pos, TVector3 hit3Pos, double R1, double R2,
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
}*/

/*double EventCategorizerTools::findMinimumFromDerivative(std::vector<double> x_vec, std::vector<double> y_vec)
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
}*/
