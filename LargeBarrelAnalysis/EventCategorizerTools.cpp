/**
 *  @copyright Copyright 2017 The J-PET Framework Authors. All rights reserved.
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
#include <TMath.h>
#include <vector>

using namespace std;

/**
* Method for determining type of event - back to back 2 gamma
*/
bool EventCategorizerTools::checkFor2Gamma(const JPetEvent& event, JPetStatistics& stats,
  bool saveHistos, double b2bSlotThetaDiff)
{
  if (event.getHits().size() < 2) return false;
  for(uint i = 0; i < event.getHits().size(); i++){
    for(uint j = i+1; j < event.getHits().size(); j++){
      JPetHit firstHit, secondHit;
      if(event.getHits().at(i).getTime() < event.getHits().at(j).getTime()){
        firstHit = event.getHits().at(i);
        secondHit = event.getHits().at(j);
      }else{
        firstHit = event.getHits().at(j);
        secondHit = event.getHits().at(i);
      }
      // Checking for back to back
      double thetaDiff = fabs(firstHit.getBarrelSlot().getTheta() - secondHit.getBarrelSlot().getTheta());
      double minTheta = 180.0-b2bSlotThetaDiff;
      double maxTheta = 180.0+b2bSlotThetaDiff;
      if(thetaDiff > minTheta && thetaDiff < maxTheta){
        if(saveHistos){
          double distance = calculateDistance(secondHit, firstHit);
          TVector3 annhilationPoint = calculateAnnihilationPoint(firstHit, secondHit);
          stats.getHisto1D("2Gamma_Zpos")->Fill(firstHit.getPosZ());
          stats.getHisto1D("2Gamma_Zpos")->Fill(secondHit.getPosZ());
          stats.getHisto1D("2Gamma_TimeDiff")->Fill(secondHit.getTime()-firstHit.getTime());
          stats.getHisto1D("2Gamma_Dist")->Fill(distance);
          stats.getHisto1D("Annih_TOF")->Fill(calculateTOF(firstHit, secondHit));
          stats.getHisto2D("AnnihPoint_XY")->Fill(annhilationPoint.X(), annhilationPoint.Y());
          stats.getHisto2D("AnnihPoint_XZ")->Fill(annhilationPoint.X(), annhilationPoint.Z());
          stats.getHisto2D("AnnihPoint_YZ")->Fill(annhilationPoint.Y(), annhilationPoint.Z());
        }
        return true;
      }
    }
  }
  return false;
}

/**
* Method for determining type of event - 3Gamma
*/
bool EventCategorizerTools::checkFor3Gamma(const JPetEvent& event, JPetStatistics& stats, bool saveHistos)
{
  if (event.getHits().size() < 3) return false;
  for(uint i = 0; i < event.getHits().size(); i++){
    for(uint j = i+1; j < event.getHits().size(); j++){
      for(uint k = j+1; k < event.getHits().size(); k++){
        JPetHit firstHit = event.getHits().at(i);
        JPetHit secondHit = event.getHits().at(j);
        JPetHit thirdHit = event.getHits().at(k);

        vector<double> thetaAngles;
        thetaAngles.push_back(firstHit.getBarrelSlot().getTheta());
        thetaAngles.push_back(secondHit.getBarrelSlot().getTheta());
        thetaAngles.push_back(thirdHit.getBarrelSlot().getTheta());
        sort(thetaAngles.begin(), thetaAngles.end());

        vector<double> relativeAngles;
        relativeAngles.push_back(thetaAngles.at(1)-thetaAngles.at(0));
        relativeAngles.push_back(thetaAngles.at(2)-thetaAngles.at(1));
        relativeAngles.push_back(360.0-thetaAngles.at(2)+thetaAngles.at(0));
        sort(relativeAngles.begin(), relativeAngles.end());
        double transformedX = relativeAngles.at(1)+relativeAngles.at(0);
        double transformedY = relativeAngles.at(1)-relativeAngles.at(0);

        if(saveHistos)
          stats.getHisto2D("3Gamma_Angles")->Fill(transformedX, transformedY);
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
  double deexTOTCutMin, double deexTOTCutMax)
{
  for(unsigned i = 0; i < event.getHits().size(); i++){
    double tot = calculateTOT(event.getHits().at(i));
    if(tot > deexTOTCutMin && tot < deexTOTCutMax){
      if(saveHistos) stats.getHisto1D("Deex_TOT_cut")->Fill(tot);
      return true;
    }
  }
  return false;
}

/**
* Method for determining type of event - scatter
*/
bool EventCategorizerTools::checkForScatter(
  const JPetEvent& event,
  JPetStatistics& stats,
  bool saveHistos,
  double scatterTOFTimeDiff)
{
  if (event.getHits().size() < 2) return false;
  for(uint i = 0; i < event.getHits().size(); i++){
    for(uint j = i+1; j < event.getHits().size(); j++){
      JPetHit primaryHit, scatterHit;
      if(event.getHits().at(i).getTime() < event.getHits().at(j).getTime()){
        primaryHit = event.getHits().at(i);
        scatterHit = event.getHits().at(j);
      }else{
        primaryHit = event.getHits().at(j);
        scatterHit = event.getHits().at(i);
      }

      double scattAngle = calculateScatteringAngle(primaryHit, scatterHit);
      double scattTOF = calculateScatteringTime(primaryHit, scatterHit)/1000.0;
      double timeDiff = scatterHit.getTime() - primaryHit.getTime();

      if(saveHistos)
        stats.getHisto1D("ScatterTOF_TimeDiff")->Fill(fabs(scattTOF-timeDiff));

      if(fabs(scattTOF-timeDiff) < scatterTOFTimeDiff){
        if(saveHistos) {
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
* the sum of the TOTs on all of the thresholds (1-4) and on the both sides (A,B)
*/
double EventCategorizerTools::calculateTOT(const JPetHit& hit)
{
  double tot = 0.0;

  std::vector<JPetSigCh> sigALead = hit.getSignalA().getRecoSignal()
    .getRawSignal().getPoints(JPetSigCh::Leading, JPetRawSignal::ByThrNum);
  std::vector<JPetSigCh> sigBLead = hit.getSignalB().getRecoSignal()
    .getRawSignal().getPoints(JPetSigCh::Leading, JPetRawSignal::ByThrNum);
  std::vector<JPetSigCh> sigATrail = hit.getSignalA().getRecoSignal()
    .getRawSignal().getPoints(JPetSigCh::Trailing, JPetRawSignal::ByThrNum);
  std::vector<JPetSigCh> sigBTrail = hit.getSignalB().getRecoSignal()
    .getRawSignal().getPoints(JPetSigCh::Trailing, JPetRawSignal::ByThrNum);

  if(sigALead.size() > 0 && sigATrail.size() > 0)
    for(unsigned i = 0; i < sigALead.size() && i < sigATrail.size(); i++)
      tot += (sigATrail.at(i).getValue() - sigALead.at(i).getValue());
  if(sigBLead.size() > 0 && sigBTrail.size() > 0)
    for(unsigned i = 0; i < sigBLead.size() && i < sigBTrail.size(); i++)
      tot += (sigBTrail.at(i).getValue() - sigBLead.at(i).getValue());

  return tot;
}

/**
* Calculation of distance between two hits
*/
double EventCategorizerTools::calculateDistance(const JPetHit& hit1, const JPetHit& hit2)
{
  return (hit1.getPos() - hit2.getPos()).Mag();
}

/**
* Calculation of time that light needs to travel the distance between primary gamma
* and scattered gamma. Return value in nanoseconds.
*/
double EventCategorizerTools::calculateScatteringTime(const JPetHit& hit1, const JPetHit& hit2)
{
  return calculateDistance(hit1, hit2)/kLightVelocity_cm_ns;
}

/**
* Calculation of scatter angle between primary hit and scattered hit.
* This function assumes that source of first gamma was in (0,0,0).
* Angle is calculated from scalar product, return value in degrees.
*/
double EventCategorizerTools::calculateScatteringAngle(const JPetHit& hit1, const JPetHit& hit2)
{
  return TMath::RadToDeg()*hit1.getPos().Angle(hit2.getPos() - hit1.getPos());
}

/**
* Calculation point in 3D, where annihilation occured
*/
TVector3 EventCategorizerTools::calculateAnnihilationPoint(const JPetHit& firstHit, const JPetHit& latterHit)
{
  double LORlength = calculateDistance(firstHit, latterHit);

  TVector3 middleOfLOR;
  middleOfLOR.SetX((firstHit.getPosX()+latterHit.getPosX())/2.0);
  middleOfLOR.SetY((firstHit.getPosY()+latterHit.getPosY())/2.0);
  middleOfLOR.SetZ((firstHit.getPosZ()+latterHit.getPosZ())/2.0);

  TVector3 versorOnLOR;
  versorOnLOR.SetX(fabs((latterHit.getPosX()-firstHit.getPosX())/LORlength));
  versorOnLOR.SetY(fabs((latterHit.getPosY()-firstHit.getPosY())/LORlength));
  versorOnLOR.SetZ(fabs((latterHit.getPosZ()-firstHit.getPosZ())/LORlength));

  TVector3 annihilationPoint;
  annihilationPoint.SetX(middleOfLOR.X()-versorOnLOR.X()*calculateTOF(firstHit, latterHit)*kLightVelocity_cm_ns/1000.0);
  annihilationPoint.SetY(middleOfLOR.Y()-versorOnLOR.Y()*calculateTOF(firstHit, latterHit)*kLightVelocity_cm_ns/1000.0);
  annihilationPoint.SetZ(middleOfLOR.Z()-versorOnLOR.Z()*calculateTOF(firstHit, latterHit)*kLightVelocity_cm_ns/1000.0);

  return annihilationPoint;
}

/**
* Calculation Time of flight
*/
double EventCategorizerTools::calculateTOF(const JPetHit& firstHit, const JPetHit& latterHit)
{
  double TOF = kUndefinedValue;
  if(firstHit.getTime() > latterHit.getTime()) {
    ERROR("First hit time should be earlier than later hit");
    return TOF;
  }
  TOF = firstHit.getTime()-latterHit.getTime();
  if(firstHit.getBarrelSlot().getTheta() < latterHit.getBarrelSlot().getTheta())
    return TOF;
  else return -1.0*TOF;
}
