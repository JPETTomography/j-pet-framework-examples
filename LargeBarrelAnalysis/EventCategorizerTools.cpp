/**
 *  @copyright Copyright 2020 The J-PET Framework Authors. All rights reserved.
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

#include <JPetMatrixSignal/JPetMatrixSignal.h>
#include "EventCategorizerTools.h"
#include "HitFinderTools.h"
#include <TMath.h>
#include <vector>

using namespace std;

/**
* Selecting pair of hits for calibrations based on TOT and Scin ID
*/
<<<<<<< HEAD:EventClassification/EventCategorizerTools.cpp
void EventCategorizerTools::selectForCalibration(
  const JPetEvent& event, JPetStatistics& stats, bool saveHistos,
  double deexTOTCutMin, double deexTOTCutMax, double anihTOTCutMin, double anihTOTCutMax
) {
  if (event.getHits().size() < 2) { return; }
  for (uint i = 0; i < event.getHits().size(); i++) {
    for (uint j = i + 1; j < event.getHits().size(); j++) {

      JPetHit anihHit;
      JPetHit deexHit;

      auto tot1 = calculateTOT(event.getHits().at(i));
      auto tot2 = calculateTOT(event.getHits().at(j));
      bool anih1 = false;
      bool anih2 = false;
      bool deex1 = false;
      bool deex2 = false;

      // Checking TOT of hits to classify them as annihilation of deexcitation
      if(tot1 > anihTOTCutMin && tot1 < anihTOTCutMax) { anih1 = true; }
      if(tot2 > anihTOTCutMin && tot2 < anihTOTCutMax) { anih2 = true; }
      if(tot1 > deexTOTCutMin && tot1 < deexTOTCutMax) { deex1 = true; }
      if(tot2 > deexTOTCutMin && tot2 < deexTOTCutMax) { deex2 = true; }

      if(anih1 && deex2) {
        anihHit = event.getHits().at(i);
        deexHit = event.getHits().at(j);
      } else if(anih2 && deex1) {
        anihHit = event.getHits().at(j);
        deexHit = event.getHits().at(i);
      } else {
        continue;
      }

      // Time differences
      auto tDiff_A_D = anihHit.getTime() - deexHit.getTime();
      auto tDiff_D_A = deexHit.getTime() - anihHit.getTime();

      // Getting IDs of strips
      auto aScinID = anihHit.getScin().getID();
      auto dScinID = deexHit.getScin().getID();

      // Filling histograms for specific scintillators
      if(saveHistos) {
        stats.getHisto1D(Form("tdiff_a_d_scin_%d", aScinID))->Fill(tDiff_A_D);
        stats.getHisto1D(Form("tdiff_d_a_scin_%d", dScinID))->Fill(tDiff_D_A);
      }
    }
=======
bool EventCategorizerTools::checkFor2Gamma(
  const JPetEvent& event, JPetStatistics& stats, bool saveHistos,
  double b2bSlotThetaDiff, double b2bTimeDiff
)
{
  if (event.getHits().size() < 2) {
    return false;
>>>>>>> 9a5825c693645ca7bcdafb33a422610e23ae4ae3:LargeBarrelAnalysis/EventCategorizerTools.cpp
  }
}


/**
* Method for determining type of event - back to back 2 gamma
*/
bool EventCategorizerTools::checkFor2Gamma(
  const JPetEvent& event, JPetStatistics& stats, bool saveHistos,
  double b2bSlotThetaDiff, double b2bTimeDiff, double anihTOTCutMin, double anihTOTCutMax
) {
  if (event.getHits().size() < 2) { return false; }

  for (uint i = 0; i < event.getHits().size(); i++) {
    for (uint j = i + 1; j < event.getHits().size(); j++) {

      JPetHit firstHit, secondHit;
      if (event.getHits().at(i).getTime() < event.getHits().at(j).getTime()) {
        firstHit = event.getHits().at(i);
        secondHit = event.getHits().at(j);
      } else {
        firstHit = event.getHits().at(j);
        secondHit = event.getHits().at(i);
      }

      // Checking for back to back
      double timeDiff = fabs(firstHit.getTime() - secondHit.getTime());
<<<<<<< HEAD:EventClassification/EventCategorizerTools.cpp
      double theta1 = min(firstHit.getScin().getSlot().getTheta(), secondHit.getScin().getSlot().getTheta());
      double theta2 = max(firstHit.getScin().getSlot().getTheta(), secondHit.getScin().getSlot().getTheta());
      double thetaDiff = min(theta2 - theta1, 360.0 - theta2 + theta1);

      if (fabs(thetaDiff - 180.0) < b2bSlotThetaDiff && timeDiff < b2bTimeDiff){
        if (saveHistos) {
          auto tot1 = firstHit.getEnergy()/((double) firstHit.getQualityOfEnergy());
          auto tot2 = secondHit.getEnergy()/((double) secondHit.getQualityOfEnergy());
          auto tof = EventCategorizerTools::calculateTOFByConvention(firstHit, secondHit);

          stats.getHisto1D("2g_tot")->Fill(tot1);
          stats.getHisto1D("2g_tot")->Fill(tot2);

          stats.getHisto1D("2g_tdiff")->Fill(timeDiff);
          stats.getHisto1D("2g_tof")->Fill(tof);

          stats.getHisto1D("2g_hit_tdiff")->Fill(firstHit.getTimeDiff());
          stats.getHisto1D("2g_hit_tdiff")->Fill(secondHit.getTimeDiff());

          if(tot1 > anihTOTCutMin && tot1 < anihTOTCutMax){
            stats.getHisto1D("2g_hit_tdiff_cut_tot")->Fill(firstHit.getTimeDiff());
          }
          if(tot2 > anihTOTCutMin && tot2 < anihTOTCutMax){
            stats.getHisto1D("2g_hit_tdiff_cut_tot")->Fill(secondHit.getTimeDiff());
          }
=======
      double deltaLor = (secondHit.getTime() - firstHit.getTime()) * kLightVelocity_cm_ps / 2.;
      double theta1 = min(firstHit.getBarrelSlot().getTheta(), secondHit.getBarrelSlot().getTheta());
      double theta2 = max(firstHit.getBarrelSlot().getTheta(), secondHit.getBarrelSlot().getTheta());
      double thetaDiff = min(theta2 - theta1, 360.0 - theta2 + theta1);
      if (saveHistos) {
        stats.fillHistogram("2Gamma_Zpos", firstHit.getPosZ());
        stats.fillHistogram("2Gamma_Zpos", secondHit.getPosZ());
        stats.fillHistogram("2Gamma_TimeDiff", timeDiff / 1000.0);
        stats.fillHistogram("2Gamma_DLOR", deltaLor);
        stats.fillHistogram("2Gamma_ThetaDiff", thetaDiff);
        stats.fillHistogram("2Gamma_Dist", calculateDistance(firstHit, secondHit));
      }
      if (fabs(thetaDiff - 180.0) < b2bSlotThetaDiff && timeDiff < b2bTimeDiff) {
        if (saveHistos) {
          TVector3 annhilationPoint = calculateAnnihilationPoint(firstHit, secondHit);
          stats.fillHistogram("Annih_TOF", calculateTOFByConvention(firstHit, secondHit));
          stats.fillHistogram("AnnihPoint_XY", annhilationPoint.X(), annhilationPoint.Y());
          stats.fillHistogram("AnnihPoint_ZX", annhilationPoint.Z(), annhilationPoint.X());
          stats.fillHistogram("AnnihPoint_ZY", annhilationPoint.Z(), annhilationPoint.Y());
          stats.fillHistogram("Annih_DLOR", deltaLor);
>>>>>>> 9a5825c693645ca7bcdafb33a422610e23ae4ae3:LargeBarrelAnalysis/EventCategorizerTools.cpp
        }
        return true;
      }
    }
  }
  // Calculating annihilation point is not yet possible since we do not have z postion of hits
  // double distance = calculateDistance(secondHit, firstHit);
  // TVector3 annhilationPoint = calculateAnnihilationPoint(firstHit, secondHit);
  // stats.getHisto1D("2Gamma_Zpos")->Fill(firstHit.getPosZ());
  // stats.getHisto1D("2Gamma_Zpos")->Fill(secondHit.getPosZ());
  // stats.getHisto1D("2Gamma_TimeDiff")->Fill(secondHit.getTime() - firstHit.getTime());
  // stats.getHisto1D("2Gamma_Dist")->Fill(distance);
  // stats.getHisto1D("Annih_TOF")->Fill(calculateTOF(firstHit, secondHit));
  // stats.getHisto2D("AnnihPoint_XY")->Fill(annhilationPoint.X(), annhilationPoint.Y());
  // stats.getHisto2D("AnnihPoint_XZ")->Fill(annhilationPoint.X(), annhilationPoint.Z());
  // stats.getHisto2D("AnnihPoint_YZ")->Fill(annhilationPoint.Y(), annhilationPoint.Z());
  return false;
}

/**
* Method for determining type of event - 3Gamma
*/
bool EventCategorizerTools::checkFor3Gamma(const JPetEvent& event, JPetStatistics& stats, bool saveHistos)
{
  if (event.getHits().size() < 3) return false;
  for (uint i = 0; i < event.getHits().size(); i++) {
    for (uint j = i + 1; j < event.getHits().size(); j++) {
      for (uint k = j + 1; k < event.getHits().size(); k++) {
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

        if (saveHistos) {
          stats.fillHistogram("3Gamma_Angles", transformedX, transformedY);
        }
      }
    }
  }
  return true;
}

/**
* Method for determining type of event - prompt
*/
bool EventCategorizerTools::checkForPrompt(
  const JPetEvent& event, JPetStatistics& stats, bool saveHistos,
<<<<<<< HEAD:EventClassification/EventCategorizerTools.cpp
  double deexTOTCutMin, double deexTOTCutMax
) {
=======
  double deexTOTCutMin, double deexTOTCutMax, std::string fTOTCalculationType)
{
>>>>>>> 9a5825c693645ca7bcdafb33a422610e23ae4ae3:LargeBarrelAnalysis/EventCategorizerTools.cpp
  for (unsigned i = 0; i < event.getHits().size(); i++) {
    double tot = HitFinderTools::calculateTOT(event.getHits().at(i), 
                                              HitFinderTools::getTOTCalculationType(fTOTCalculationType));
    if (tot > deexTOTCutMin && tot < deexTOTCutMax) {
      if (saveHistos) {
        stats.fillHistogram("Deex_TOT_cut", tot);
      }
      return true;
    }
  }
  return false;
}

/**
* Method for determining type of event - scatter
*/
bool EventCategorizerTools::checkForScatter(
<<<<<<< HEAD:EventClassification/EventCategorizerTools.cpp
  const JPetEvent& event, JPetStatistics& stats, bool saveHistos, double scatterTOFTimeDiff
) {
=======
  const JPetEvent& event, JPetStatistics& stats, bool saveHistos, double scatterTOFTimeDiff, 
  std::string fTOTCalculationType)
{
>>>>>>> 9a5825c693645ca7bcdafb33a422610e23ae4ae3:LargeBarrelAnalysis/EventCategorizerTools.cpp
  if (event.getHits().size() < 2) {
    return false;
  }
  for (uint i = 0; i < event.getHits().size(); i++) {
    for (uint j = i + 1; j < event.getHits().size(); j++) {
      JPetHit primaryHit, scatterHit;
      if (event.getHits().at(i).getTime() < event.getHits().at(j).getTime()) {
        primaryHit = event.getHits().at(i);
        scatterHit = event.getHits().at(j);
      } else {
        primaryHit = event.getHits().at(j);
        scatterHit = event.getHits().at(i);
      }

      double scattAngle = calculateScatteringAngle(primaryHit, scatterHit);
      double scattTOF = calculateScatteringTime(primaryHit, scatterHit);
      double timeDiff = scatterHit.getTime() - primaryHit.getTime();

      if (saveHistos) {
        stats.fillHistogram("ScatterTOF_TimeDiff", fabs(scattTOF - timeDiff));
      }

      if (fabs(scattTOF - timeDiff) < scatterTOFTimeDiff) {
        if (saveHistos) {
          stats.fillHistogram("ScatterAngle_PrimaryTOT", scattAngle, HitFinderTools::calculateTOT(primaryHit, 
                                                        HitFinderTools::getTOTCalculationType(fTOTCalculationType)));
          stats.fillHistogram("ScatterAngle_ScatterTOT", scattAngle, HitFinderTools::calculateTOT(scatterHit, 
                                                        HitFinderTools::getTOTCalculationType(fTOTCalculationType)));
        }
        return true;
      }
    }
  }
  return false;
}

/**
<<<<<<< HEAD:EventClassification/EventCategorizerTools.cpp
* Calculation of the total TOT of the hit - Time over Threshold:
* the sum of the TOTs on all of the thresholds (1-4) and on the both sides (A,B)
*/
double EventCategorizerTools::calculateTOT(const JPetHit& hit)
{
  // double tot = 0.0;
  //
  // auto rawSignalsA = hit.getSignalA().getRawSignals();
  // auto rawSignalsB = hit.getSignalB().getRawSignals();
  //
  // for(auto rawSig: rawSignalsA){
  //   auto sigALead = rawSig.second.getPoints(JPetSigCh::Leading, JPetRawSignal::ByThrNum);
  //   auto sigATrail = rawSig.second.getPoints(JPetSigCh::Trailing, JPetRawSignal::ByThrNum);
  //   if (sigALead.size() > 0 && sigATrail.size() > 0){
  //     for (unsigned i = 0; i < sigALead.size() && i < sigATrail.size(); i++){
  //       tot += (sigATrail.at(i).getTime() - sigALead.at(i).getTime());
  //     }
  //   }
  // }
  //
  // for(auto rawSig: rawSignalsB){
  //   auto sigBLead = rawSig.second.getPoints(JPetSigCh::Leading, JPetRawSignal::ByThrNum);
  //   auto sigBTrail = rawSig.second.getPoints(JPetSigCh::Trailing, JPetRawSignal::ByThrNum);
  //   if (sigBLead.size() > 0 && sigBTrail.size() > 0){
  //     for (unsigned i = 0; i < sigBLead.size() && i < sigBTrail.size(); i++){
  //       tot += (sigBTrail.at(i).getTime() - sigBLead.at(i).getTime());
  //     }
  //   }
  // }
  auto multi = hit.getSignalA().getRawSignals().size() + hit.getSignalB().getRawSignals().size();
  return hit.getEnergy()/((double) multi);
}

/**
=======
>>>>>>> 9a5825c693645ca7bcdafb33a422610e23ae4ae3:LargeBarrelAnalysis/EventCategorizerTools.cpp
* Calculation of distance between two hits
*/
double EventCategorizerTools::calculateDistance(const JPetHit& hit1, const JPetHit& hit2)
{
  return (hit1.getPos() - hit2.getPos()).Mag();
}

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
  TVector3 versorOnLOR = (hitB - hitA).Unit()  ;

  double shift = 0.5 * tof  * kLightVelocity_cm_ps;
  TVector3 annihilationPoint(
    middleOfLOR.X() + shift * versorOnLOR.X(),
    middleOfLOR.Y() + shift * versorOnLOR.Y(),
    middleOfLOR.Z() + shift * versorOnLOR.Z()
  );
  return annihilationPoint;
}

double EventCategorizerTools::calculateTOFByConvention(const JPetHit& hitA, const JPetHit& hitB)
{
  if (hitA.getScin().getSlot().getTheta() < hitB.getScin().getSlot().getTheta()) {
    return calculateTOF(hitA, hitB);
  } else {
    return calculateTOF(hitB, hitA);
  }
}

double EventCategorizerTools::calculateTOF(const JPetHit& hitA, const JPetHit& hitB)
{
  return EventCategorizerTools::calculateTOF(hitA.getTime(), hitB.getTime());
}

double EventCategorizerTools::calculateTOF(double time1, double time2)
{
  return (time1 - time2);
}

/**
* Calculating distance from the center of the decay plane
*/
double EventCategorizerTools::calculatePlaneCenterDistance(
  const JPetHit& firstHit, const JPetHit& secondHit, const JPetHit& thirdHit)
{
  TVector3 crossProd = (secondHit.getPos() - firstHit.getPos()).Cross(thirdHit.getPos() - secondHit.getPos());
  double distCoef = -crossProd.X() * secondHit.getPosX() - crossProd.Y() * secondHit.getPosY() - crossProd.Z() * secondHit.getPosZ();
  if (crossProd.Mag() != 0) {
    return fabs(distCoef) / crossProd.Mag();
  } else {
    ERROR("One of the hit has zero position vector - unable to calculate distance from the center of the surface");
    return -1.;
  }
}
<<<<<<< HEAD:EventClassification/EventCategorizerTools.cpp

/**
* Method for determining type of event for streaming - 2 gamma
* @todo: the selection criteria b2b distance from center needs to be checked
* and implemented again
*/
bool EventCategorizerTools::stream2Gamma(
  const JPetEvent& event, JPetStatistics& stats, bool saveHistos,
  double b2bSlotThetaDiff, double b2bTimeDiff
)
{
  if (event.getHits().size() < 2) {
    return false;
  }
  for (uint i = 0; i < event.getHits().size(); i++) {
    for (uint j = i + 1; j < event.getHits().size(); j++) {
      JPetHit firstHit, secondHit;
      if (event.getHits().at(i).getTime() < event.getHits().at(j).getTime()) {
        firstHit = event.getHits().at(i);
        secondHit = event.getHits().at(j);
      } else {
        firstHit = event.getHits().at(j);
        secondHit = event.getHits().at(i);
      }
      // Checking for back to back
      double timeDiff = fabs(firstHit.getTime() - secondHit.getTime());
      double deltaLor = (secondHit.getTime() - firstHit.getTime()) * kLightVelocity_cm_ps / 2.;
      double theta1 = min(firstHit.getScin().getSlot().getTheta(), secondHit.getScin().getSlot().getTheta());
      double theta2 = max(firstHit.getScin().getSlot().getTheta(), secondHit.getScin().getSlot().getTheta());
      double thetaDiff = min(theta2 - theta1, 360.0 - theta2 + theta1);
      if (saveHistos) {
        stats.getHisto1D("2Gamma_TimeDiff")->Fill(timeDiff / 1000.0);
        stats.getHisto1D("2Gamma_DLOR")->Fill(deltaLor);
        stats.getHisto1D("2Gamma_ThetaDiff")->Fill(thetaDiff);
      }
      if (fabs(thetaDiff - 180.0) < b2bSlotThetaDiff && timeDiff < b2bTimeDiff) {
        if (saveHistos) {
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
bool EventCategorizerTools::stream3Gamma(
  const JPetEvent& event, JPetStatistics& stats, bool saveHistos,
  double d3SlotThetaMin, double d3TimeDiff, double d3PlaneCenterDist
)
{
  if (event.getHits().size() < 3) {
    return false;
  }
  for (uint i = 0; i < event.getHits().size(); i++) {
    for (uint j = i + 1; j < event.getHits().size(); j++) {
      for (uint k = j + 1; k < event.getHits().size(); k++) {
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
        if (saveHistos) {
          stats.getHisto1D("3GammaTimeDiff")->Fill(timeDiff);
          stats.getHisto2D("3GammaThetas")->Fill(transformedX, transformedY);
          stats.getHisto1D("3GammaPlaneDist")->Fill(planeCenterDist);
        }
        if (transformedX > d3SlotThetaMin && timeDiff < d3TimeDiff && planeCenterDist < d3PlaneCenterDist) {
          if (saveHistos) {
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
=======
>>>>>>> 9a5825c693645ca7bcdafb33a422610e23ae4ae3:LargeBarrelAnalysis/EventCategorizerTools.cpp
