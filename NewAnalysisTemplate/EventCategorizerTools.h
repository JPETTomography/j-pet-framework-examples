/**
 *  @copyright Copyright 2016 The J-PET Framework Authors. All rights reserved.
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
 *  @file EventCategorizerTools.h
 */

#ifndef EVENTCATEGORIZERTOOLS_H
#define EVENTCATEGORIZERTOOLS_H
#include <vector>
#include <JPetHit/JPetHit.h>
#include <JPetStatistics/JPetStatistics.h>
#include <TRotation.h>

class EventCategorizerTools
{
public:

	static double CalcTOT( JPetHit Hit );
	static double CalcScattAngle( JPetHit Hit1, JPetHit Hit2 );
	static double CalcScattTime( JPetHit Hit1, JPetHit Hit2 );
	
	static double CalcDistance( JPetHit Hit1, JPetHit Hit2 );
	static double CalcAngle( JPetHit Hit1, JPetHit Hit2 );
	static double CalcAngle2D( JPetHit Hit1, JPetHit Hit2 );
	static std::vector<double> CalcAnglesFrom3Hit( JPetHit Hit1, JPetHit Hit2, JPetHit Hit3 );
	static std::vector<double> CalcAngles2DFrom3Hit( JPetHit Hit1, JPetHit Hit2, JPetHit Hit3 );
	static double CalcDistanceOfSurfaceAndZero( JPetHit Hit1, JPetHit Hit2, JPetHit Hit3 );

	static TVector3 RecoPosition( const JPetHit & Hit1, const JPetHit & Hit2);
	
	static TVector3 RecoPosition3Hit( JPetHit Hit1, JPetHit Hit2, JPetHit Hit3 );
	static TVector3 FindIntersection( TVector3 Hit1Pos, TVector3 Hit2Pos, TVector3 Hit3Pos, double t21, double t31 );
	static double FindMinFromQuadraticFit( std::vector<double> Arg, std::vector<double> Val );
	static double FindMinFromDerrivative( std::vector<double> Arg, std::vector<double> Val );
	static std::vector< std::vector<double> > FindIntersectionPointsOfCircles( TVector3 Hit1Pos, TVector3 Hit2Pos, TVector3 Hit3Pos, double R1, double R2, double R3, double R13, double R21, double R32 );
	static int CheckIfScattered( JPetHit Hit1, JPetHit Hit2, double ErrorInterval );
	
	static TVector3 AlekReconstruct( JPetHit Hit1, JPetHit Hit2, JPetHit Hit3 );
	static TVector3 AlekReconstruct2( JPetHit Hit1, JPetHit Hit2, JPetHit Hit3 );
	
	static double CalcDistFromCentres( TVector3 Sol1, TVector3 Gamma1, TVector3 Gamma2, TVector3 Gamma3 );
	static double NormalizeTime( JPetHit Hit1 );

	
};


#endif
