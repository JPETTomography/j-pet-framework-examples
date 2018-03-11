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
bool EventCategorizerTools::checkFor2Gamma(const JPetEvent& event, JPetStatistics& stats, bool saveHistos)
{
  if (event.getHits().size() < 2) return false;
  for(uint i = 0; i < event.getHits().size(); i++){
    for(uint j = i+1; j < event.getHits().size(); j++){
      JPetHit& firstHit = event.getHits().at(i);
      JPetHit& secondHit = event.getHits().at(j);
      // Checking for back to back
      double thetaDiff = fabs(firstHit.getBarrelSlot().getTheta() - secondHit.getBarrelSlot().getTheta());
      if(thetaDiff > 177.0 && thetaDiff < 183.0){
        if(saveHistos){
          double distance = (secondHit.getPos()-firstHit.getPos()).Mag();
          Point3D annhilationPoint = calculateAnnihilationPoint(firstHit, secondHit);
          stats.getHisto1D("2Gamma_Zpos")->Fill(firstHit.getPosZ());
          stats.getHisto1D("2Gamma_Zpos")->Fill(secondHit.getPosZ());
          stats.getHisto1D("2Gamma_TimeDiff")->Fill(secondHit.getTime()-firstHit.getTime());
          stats.getHisto1D("2Gamma_Dist")->Fill(distance);
          stats.getHisto1D("Annih_TOF")->Fill(calculateTOF(firstHit, secondHit));
          stats.getHisto2D("AnnihPoint_XY")->Fill(annhilationPoint.x, annhilationPoint.y);
          stats.getHisto2D("AnnihPoint_XZ")->Fill(annhilationPoint.x, annhilationPoint.z);
          stats.getHisto2D("AnnihPoint_YZ")->Fill(annhilationPoint.y, annhilationPoint.z);
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
        JPetHit& firstHit = event.getHits().at(i);
        JPetHit& secondHit = event.getHits().at(j);
        JPetHit& thirdHit = event.getHits().at(k);

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
bool EventCategorizerTools::checkForPrompt(const JPetEvent&, JPetStatistics&, bool)
{
  // Placeholder for proper method
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
        stats.getHisto1D("ScatterTOF_TimeDiff")->Fill(scattTOF-timeDiff);

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

	for(uint i = 0; i < sigALead.size() && i < sigATrail.size(); i++)
		tot += (sigATrail.at(i).getValue() - sigALead.at(i).getValue());
	for( unsigned i = 0; i < sigBLead.size() && i < sigBTrail.size(); i++)
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
Point3D EventCategorizerTools::calculateAnnihilationPoint(const JPetHit& firstHit, const JPetHit& latterHit)
{
  Point3D annihilationPoint;
  annihilationPoint.x = kUndefined::point;
  annihilationPoint.y = kUndefined::point;
  annihilationPoint.z = kUndefined::point;

  Point3D firstHitPoint;
  firstHitPoint.x = firstHit.getPosX();
  firstHitPoint.y = firstHit.getPosY();
  firstHitPoint.z = firstHit.getPosZ();

  Point3D latterHitPoint;
  latterHitPoint.x = latterHit.getPosX();
  latterHitPoint.y = latterHit.getPosY();
  latterHitPoint.z = latterHit.getPosZ();

  Point3D middleOfLOR;
  middleOfLOR.x = ( firstHitPoint.x + latterHitPoint.x ) / 2.0;
  middleOfLOR.y = ( firstHitPoint.y + latterHitPoint.y ) / 2.0;
  middleOfLOR.z = ( firstHitPoint.z + latterHitPoint.z ) / 2.0;

  double LORlength = sqrt( pow(( firstHitPoint.x - latterHitPoint.x),2) +
		    pow((firstHitPoint.y - latterHitPoint.y),2) +
		    pow((firstHitPoint.z - latterHitPoint.z),2) );

  double versorOnLOR_x, versorOnLOR_y, versorOnLOR_z;  //towards first hit
  versorOnLOR_x =  fabs( (latterHitPoint.x - firstHitPoint.x)/LORlength );
  versorOnLOR_y = fabs( (latterHitPoint.y - firstHitPoint.y)/LORlength );
  versorOnLOR_z = fabs( (latterHitPoint.z - firstHitPoint.z)/LORlength );

  double velocity = 30.0; //cm/ns
  velocity/=1000; //cm/ps

  annihilationPoint.x = middleOfLOR.x - versorOnLOR_x* calculateTOF(firstHit, latterHit)*velocity;
  annihilationPoint.y = middleOfLOR.y - versorOnLOR_y* calculateTOF(firstHit, latterHit)*velocity;
  annihilationPoint.z = middleOfLOR.z - versorOnLOR_z* calculateTOF(firstHit, latterHit)*velocity;

  return annihilationPoint;
}

/**
* Calculation Time of flight
*/
double EventCategorizerTools::calculateTOF(const JPetHit& firstHit, const JPetHit& latterHit)
{
  double TOF = kUndefined::tof;
  if( firstHit.getTime() > latterHit.getTime() )
  {
    ERROR("First hit time should be earlier than later hit");
    return TOF;
  }
  TOF = firstHit.getTime()-latterHit.getTime();

  double firstHitSlotAngle = firstHit.getBarrelSlot().getTheta();
  double latterHitSlotAngle = latterHit.getBarrelSlot().getTheta();

  if( firstHitSlotAngle < latterHitSlotAngle )
    return TOF;
  else
    return -1.0*TOF;

  return TOF;
}
