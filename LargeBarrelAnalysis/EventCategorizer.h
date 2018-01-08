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
 *  @file EventCategorizer.h
 */

#ifndef EVENTCATEGORIZER_H
#define EVENTCATEGORIZER_H

#include <vector>
#include <map>
#include <JPetUserTask/JPetUserTask.h>
#include <JPetHit/JPetHit.h>
#include <JPetEvent/JPetEvent.h>
#include <TRotation.h>
//#include <JPetGeomMapping/JPetGeomMapping.h>

class JPetWriter;

#ifdef __CINT__
#	define override
#endif

class EventCategorizer : public JPetUserTask{
public:  
	void RecoPos( JPetHit Hit1, JPetHit Hit2, JPetHit Hit3 );
	EventCategorizer(const char * name);
	virtual ~EventCategorizer(){}
	virtual bool init() override;
	virtual bool exec() override;
	virtual bool terminate() override;

protected:
	void saveEvents(const std::vector<JPetEvent>& event);
	bool fSaveControlHistos = true;
	//JPetGeomMapping* fBarrelMap;
};

double CalcTOT( JPetHit Hit );
double CalcScattAngle( JPetHit Hit1, JPetHit Hit2 );
double CalcDistanceBetweenHits( JPetHit Hit1, JPetHit Hit2 );
double CalcAngle( JPetHit Hit1, JPetHit Hit2 );
double CalcDistanceOfSurfaceAndZero( JPetHit Hit1, JPetHit Hit2, JPetHit Hit3 );
int CheckIfScattered( JPetHit Hit1, JPetHit Hit2, double ErrorInterval, JPetStatistics& Stats );
double NormalizeTime( JPetHit Hit1 );
double NormalizeTimeToPoint( JPetHit Hit1, TVector3 Point );

TVector3 RecoPos2Hit( const JPetHit & Hit1, const JPetHit & Hit2);

TVector3 FindIntersection2( TVector3 Hit1Pos, TVector3 Hit2Pos, TVector3 Hit3Pos, double t21, double t31 );
double FindMinFromDerrivative( std::vector<double> Arg, std::vector<double> Val );
std::vector< std::vector<double> > FindIntersectionPointsOfCircles( TVector3 Hit1Pos, TVector3 Hit2Pos, TVector3 Hit3Pos, double R1, double R2, double R3, double R13, double R21, double R32 );
TVector3 AlekReconstruct( TVector3 Hit1Pos, TVector3 Hit2Pos, TVector3 Hit3Pos, double t1, double t2, double t3 );
TVector3 AlekReconstruct2( TVector3 Hit1Pos, TVector3 Hit2Pos, TVector3 Hit3Pos, double t1, double t2, double t3 );
double CalcDistFromCentres( TVector3 Sol1, TVector3 Gamma1, TVector3 Gamma2, TVector3 Gamma3 );
double CalcDistAdd( JPetHit Hit1 );

#endif /*  !EVENTCATEGORIZER_H */
