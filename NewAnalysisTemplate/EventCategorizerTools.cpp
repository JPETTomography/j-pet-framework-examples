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
 *  @file EventCategorizerTools.cpp
 */

#include "EventCategorizerTools.h"
#include <algorithm>
#include <iterator>


double EventCategorizerTools::CalcTOT( JPetHit Hit )
{
	double TOT = 0;
	  std::map<int, double> lead_timesA, lead_timesB, trail_timesA, trail_timesB;
	  lead_timesA = Hit.getSignalA().getRecoSignal().getRawSignal().getTimesVsThresholdNumber(JPetSigCh::Leading);
	  lead_timesB = Hit.getSignalB().getRecoSignal().getRawSignal().getTimesVsThresholdNumber(JPetSigCh::Leading);
	  trail_timesA = Hit.getSignalA().getRecoSignal().getRawSignal().getTimesVsThresholdNumber(JPetSigCh::Trailing);
	  trail_timesB = Hit.getSignalB().getRecoSignal().getRawSignal().getTimesVsThresholdNumber(JPetSigCh::Trailing);	  
	  for(int i=1;i<=Hit.getSignalA().getRecoSignal().getRawSignal().getNumberOfPoints(JPetSigCh::Leading);i++){
                    auto leadSearch = lead_timesA.find(i);
                    auto trailSearch = trail_timesA.find(i);
                    if (leadSearch != lead_timesA.end()
                        && trailSearch != trail_timesA.end())
                            TOT+=(trailSearch->second - leadSearch->second)/1000;
                }
          for(int i=1;i<=Hit.getSignalB().getRecoSignal().getRawSignal().getNumberOfPoints(JPetSigCh::Leading);i++){
                    auto leadSearch = lead_timesB.find(i);
                    auto trailSearch = trail_timesB.find(i);
                    if (leadSearch != lead_timesB.end()
                        && trailSearch != trail_timesB.end())
                            TOT+=(trailSearch->second - leadSearch->second)/1000;
                }
	return TOT;
}

double EventCategorizerTools::CalcScattAngle( JPetHit Hit1, JPetHit Hit2 )
{
	double scalarProd = Hit1.getPosX()*(Hit2.getPosX()-Hit1.getPosX()) + Hit1.getPosY()*(Hit2.getPosY()-Hit1.getPosY()) + Hit1.getPosZ()*(Hit2.getPosZ()-Hit1.getPosZ());
	double magProd = sqrt( ( pow(Hit1.getPosX(),2)	// Pos in cm
			+pow(Hit1.getPosY(),2)
			+pow(Hit1.getPosZ(),2) )*( pow(Hit2.getPosX()-Hit1.getPosX(),2)
			+pow(Hit2.getPosY()-Hit1.getPosY(),2)
			+pow(Hit2.getPosZ()-Hit1.getPosZ(),2) ) );
	double ScattAngle = acos( scalarProd/magProd )*180/3.14159265;
	return ScattAngle;
}


double EventCategorizerTools::CalcScattTime( JPetHit Hit1, JPetHit Hit2 )
{
	float dist1_Scatt = sqrt(pow(Hit1.getPosX()-Hit2.getPosX(),2)  // Pos in cm
			+pow(Hit1.getPosY()-Hit2.getPosY(),2)
			+pow(Hit1.getPosZ()-Hit2.getPosZ(),2));
	return dist1_Scatt/29.979246;  //light velocity in cm/s, returns time in ns
}

double EventCategorizerTools::CalcDistance( JPetHit Hit1, JPetHit Hit2 )
{
	float dist1 = sqrt(pow(Hit1.getPosX()-Hit2.getPosX(),2)  // Pos in cm
			+pow(Hit1.getPosY()-Hit2.getPosY(),2)
			+pow(Hit1.getPosZ()-Hit2.getPosZ(),2));
	return dist1;
}

double EventCategorizerTools::CalcAngle( JPetHit Hit1, JPetHit Hit2 )
{
	double scalarProd = Hit1.getPosX()*Hit2.getPosX() + Hit1.getPosY()*Hit2.getPosY() + Hit1.getPosZ()*Hit2.getPosZ();
	double magProd = sqrt( ( pow(Hit1.getPosX(),2)	// Pos in cm
			+pow(Hit1.getPosY(),2)
			+pow(Hit1.getPosZ(),2) )*( pow(Hit2.getPosX(),2)
			+pow(Hit2.getPosY(),2)
			+pow(Hit2.getPosZ(),2) ) );
	double Angle = acos( scalarProd/magProd )*180/3.14159265;
	return Angle;
}

double EventCategorizerTools::CalcAngle2D( JPetHit Hit1, JPetHit Hit2 )
{
	double scalarProd = Hit1.getPosX()*Hit2.getPosX() + Hit1.getPosY()*Hit2.getPosY();
	double magProd = sqrt( ( pow(Hit1.getPosX(),2)	// Pos in cm
			+pow(Hit1.getPosY(),2) )*
			( pow(Hit2.getPosX(),2)
			+pow(Hit2.getPosY(),2) ) );
	double Angle = acos( scalarProd/magProd )*180/3.14159265;
	return Angle;
}

std::vector<double> EventCategorizerTools::CalcAnglesFrom3Hit( JPetHit Hit1, JPetHit Hit2, JPetHit Hit3 )
{
	std::vector<double> angles;
	double angle12 = CalcAngle( Hit1, Hit2);
	double angle23 = CalcAngle( Hit2, Hit3);
	double angle31 = CalcAngle( Hit3, Hit1);
	angles.push_back( angle12 );
	angles.push_back( angle23 );
	angles.push_back( angle31 );
	std::sort( angles.begin(), angles.begin() +3 );
	return angles;
}

std::vector<double> EventCategorizerTools::CalcAngles2DFrom3Hit( JPetHit Hit1, JPetHit Hit2, JPetHit Hit3 )
{
	std::vector<double> angles;
	double angle12 = CalcAngle2D( Hit1, Hit2);
	double angle23 = CalcAngle2D( Hit2, Hit3);
	double angle31 = CalcAngle2D( Hit3, Hit1);
	angles.push_back( angle12 );
	angles.push_back( angle23 );
	angles.push_back( angle31 );
	std::sort( angles.begin(), angles.begin() +3 );
	return angles;
}

double EventCategorizerTools::CalcDistanceOfSurfaceAndZero( JPetHit Hit1, JPetHit Hit2, JPetHit Hit3 )
{
	TVector3 vec1( Hit2.getPosX() - Hit1.getPosX(), Hit2.getPosY() - Hit1.getPosY(), Hit2.getPosZ() - Hit1.getPosZ() );
	TVector3 vec2( Hit3.getPosX() - Hit2.getPosX(), Hit3.getPosY() - Hit2.getPosY(), Hit3.getPosZ() - Hit2.getPosZ() );
	TVector3 crossProd  = vec1.Cross(vec2);
	double Dcoeef = -crossProd(0)*Hit2.getPosX() -crossProd(1)*Hit2.getPosY() -crossProd(2)*Hit2.getPosZ();
	double distanceFromZero = fabs(Dcoeef) / crossProd.Mag();
	return distanceFromZero;
}

TVector3 EventCategorizerTools::RecoPosition3Hit( JPetHit Hit1, JPetHit Hit2, JPetHit Hit3 )
{
	TVector3 vecSurface;
			     
	vecSurface.SetX( (Hit2.getPosY() - Hit1.getPosY() )*(Hit3.getPosZ() - Hit1.getPosZ() ) - (Hit2.getPosZ() - Hit1.getPosZ() )*(Hit3.getPosY() - Hit1.getPosY() ) );
	vecSurface(1) = (Hit2.getPosZ() - Hit1.getPosZ() )*(Hit3.getPosX() - Hit1.getPosX() ) - (Hit2.getPosX() - Hit1.getPosX() )*(Hit3.getPosZ() - Hit1.getPosZ() );
	vecSurface(2) = (Hit2.getPosX() - Hit1.getPosX() )*(Hit3.getPosY() - Hit1.getPosY() ) - (Hit2.getPosY() - Hit1.getPosY() )*(Hit3.getPosX() - Hit1.getPosX() );
	
	TVector3 vecPerpendicular( -vecSurface(1), vecSurface(0), 0 );
	vecPerpendicular = vecPerpendicular.Unit();
	
	double Theta = -acos( vecSurface(2)/vecSurface.Mag() );
	
	TVector3 RotationX( cos(Theta)+vecPerpendicular(0)*vecPerpendicular(0)*(1-cos(Theta)), vecPerpendicular(0)*vecPerpendicular(1)*(1-cos(Theta)), vecPerpendicular(1)*sin(Theta) );
	TVector3 RotationY( vecPerpendicular(0)*vecPerpendicular(1)*(1-cos(Theta)), cos(Theta)+vecPerpendicular(1)*vecPerpendicular(1)*(1-cos(Theta)), -vecPerpendicular(0)*sin(Theta) );
	TVector3 RotationZ( -vecPerpendicular(1)*sin(Theta), vecPerpendicular(0)*sin(Theta), cos(Theta) );

	TVector3 vecHit1( Hit1.getPosX(), Hit1.getPosY(), Hit1.getPosZ() );
	TVector3 vecHit2( Hit2.getPosX(), Hit2.getPosY(), Hit2.getPosZ() );
	TVector3 vecHit3( Hit3.getPosX(), Hit3.getPosY(), Hit3.getPosZ() );
	
	TVector3 P1( RotationX*vecHit1, RotationY*vecHit1, RotationZ*vecHit1 );
	TVector3 P2( RotationX*vecHit2, RotationY*vecHit2, RotationZ*vecHit2 );
	TVector3 P3( RotationX*vecHit3, RotationY*vecHit3, RotationZ*vecHit3 );

	TVector3 Point = FindIntersection( P1, P2, P3, Hit2.getTime()/1000 - Hit1.getTime()/1000, Hit3.getTime()/1000 - Hit1.getTime()/1000 );
	
	if( Point(0) == 100. && Point(1) == 100. )
	  return Point;
	
	TVector3 RotationXr( cos(-Theta)+vecPerpendicular(0)*vecPerpendicular(0)*(1-cos(-Theta)), vecPerpendicular(0)*vecPerpendicular(1)*(1-cos(-Theta)), vecPerpendicular(1)*sin(-Theta) );
	TVector3 RotationYr( vecPerpendicular(0)*vecPerpendicular(1)*(1-cos(-Theta)), cos(-Theta)+vecPerpendicular(1)*vecPerpendicular(1)*(1-cos(-Theta)), -vecPerpendicular(0)*sin(-Theta) );
	TVector3 RotationZr( -vecPerpendicular(1)*sin(-Theta), vecPerpendicular(0)*sin(-Theta), cos(-Theta) );
	
	TVector3 Pointr( RotationXr*Point, RotationYr*Point, RotationZr*Point );
	
	return Pointr;
	
}

/*TVector3 EventCategorizerTools::FindIntersection( JPetHit Hit1, JPetHit Hit2, JPetHit Hit3 )
{
	
	TVector3 Hit1Pos( Hit1.getPosX(), Hit1.getPosY(), Hit1.getPosZ() );
	TVector3 Hit2Pos( Hit2.getPosX(), Hit2.getPosY(), Hit2.getPosZ() );
	TVector3 Hit3Pos( Hit3.getPosX(), Hit3.getPosY(), Hit3.getPosZ() );  
	double t21 = Hit2.getTime()/1000 - Hit1.getTime()/1000;
	double t31 = Hit3.getTime()/1000 - Hit1.getTime()/1000;
  
  
	double R21 = sqrt( pow(Hit2Pos(0) - Hit1Pos(0),2 ) + pow(Hit2Pos(1) - Hit1Pos(1),2 ) );
	double R32 = sqrt( pow(Hit3Pos(0) - Hit2Pos(0),2 ) + pow(Hit3Pos(1) - Hit2Pos(1),2 ) );
	double R13 = sqrt( pow(Hit1Pos(0) - Hit3Pos(0),2 ) + pow(Hit1Pos(1) - Hit3Pos(1),2 ) );
	
	double TDiffTOR1 = 0.;
	double TDiffTOR2 = t21;
	double TDiffTOR3 = t31;
	
	TDiffTOR2 = 29.979246*TDiffTOR2;
	TDiffTOR3 = 29.979246*TDiffTOR3;
	
	double R0 = 0.;
	
	if( R0 < (R21 - TDiffTOR2 )/2  )
	  R0 = (R21 - TDiffTOR2 )/2;
	if( R0 < (R32 - TDiffTOR2 - TDiffTOR3 )/2  )
	  R0 = (R32 - TDiffTOR2 - TDiffTOR3 )/2;
	if( R0 < (R13 - TDiffTOR3 )/2  )
	  R0 = (R13 - TDiffTOR3 )/2;
	
	double R1 = 0.;
	double R2 = 0.;
	double R3 = 0.;
	std::vector<double> temp, temp2;
	std::vector< std::vector<double> > Points;
	temp.push_back(0.);
	temp.push_back(0.);
	Points.push_back(temp);
	Points.push_back(temp);
	Points.push_back(temp);
	Points.push_back(temp);
	Points.push_back(temp);
	Points.push_back(temp);
	double Distance = 0.;
	double MinDistance = 0.;
	double PreviousDistance = 10000000.;
	
	temp.clear();
	
	int test = 1;
	while( test )
	{
		R1 = TDiffTOR1 + R0+1;
		R2 = TDiffTOR2 + R0+1;
		R3 = TDiffTOR2 + R0+1;
		
		Points[0][0] = (Hit1Pos(0) + Hit2Pos(0) )/2 + (pow(R1,2) - pow(R2,2) )*( Hit2Pos(0) - Hit1Pos(0) )/2/pow( R21,2 ) + 0.5*( Hit2Pos(1) - Hit1Pos(1) )*sqrt( 2 * (pow(R1,2) + pow(R2,2) )/pow( R21,2 ) - pow( pow(R1,2) - pow(R2,2), 2 )/pow( R21, 4 ) - 1 );
		Points[0][1] = (Hit1Pos(1) + Hit2Pos(1) )/2 + (pow(R1,2) - pow(R2,2) )*( Hit2Pos(1) - Hit1Pos(1) )/2/pow( R21,2 ) + 0.5*( Hit1Pos(0) - Hit2Pos(0) )*sqrt( 2 * (pow(R1,2) + pow(R2,2) )/pow( R21,2 ) - pow( pow(R1,2) - pow(R2,2), 2 )/pow( R21, 4 ) - 1 );
		Points[1][0] = (Hit1Pos(0) + Hit2Pos(0) )/2 + (pow(R1,2) - pow(R2,2) )*( Hit2Pos(0) - Hit1Pos(0) )/2/pow( R21,2 ) - 0.5*( Hit2Pos(1) - Hit1Pos(1) )*sqrt( 2 * (pow(R1,2) + pow(R2,2) )/pow( R21,2 ) - pow( pow(R1,2) - pow(R2,2), 2 )/pow( R21, 4 ) - 1 );
		Points[1][1] = (Hit1Pos(1) + Hit2Pos(1) )/2 + (pow(R1,2) - pow(R2,2) )*( Hit2Pos(1) - Hit1Pos(1) )/2/pow( R21,2 ) - 0.5*( Hit1Pos(0) - Hit2Pos(0) )*sqrt( 2 * (pow(R1,2) + pow(R2,2) )/pow( R21,2 ) - pow( pow(R1,2) - pow(R2,2), 2 )/pow( R21, 4 ) - 1 );
		
		Points[2][0] = (Hit2Pos(0) + Hit3Pos(0) )/2 + (pow(R2,2) - pow(R3,2) )*( Hit3Pos(0) - Hit2Pos(0) )/2/pow( R32,2 ) + 0.5*( Hit3Pos(1) - Hit2Pos(1) )*sqrt( 2 * (pow(R2,2) + pow(R3,2) )/pow( R32,2 ) - pow( pow(R2,2) - pow(R3,2), 2 )/pow( R32, 4 ) - 1 );
		Points[2][1] = (Hit2Pos(1) + Hit3Pos(1) )/2 + (pow(R2,2) - pow(R3,2) )*( Hit3Pos(1) - Hit2Pos(1) )/2/pow( R32,2 ) + 0.5*( Hit2Pos(0) - Hit3Pos(0) )*sqrt( 2 * (pow(R2,2) + pow(R3,2) )/pow( R32,2 ) - pow( pow(R2,2) - pow(R3,2), 2 )/pow( R32, 4 ) - 1 );
		Points[3][0] = (Hit2Pos(0) + Hit3Pos(0) )/2 + (pow(R2,2) - pow(R3,2) )*( Hit3Pos(0) - Hit2Pos(0) )/2/pow( R32,2 ) - 0.5*( Hit3Pos(1) - Hit2Pos(1) )*sqrt( 2 * (pow(R2,2) + pow(R3,2) )/pow( R32,2 ) - pow( pow(R2,2) - pow(R3,2), 2 )/pow( R32, 4 ) - 1 );
		Points[3][1] = (Hit2Pos(1) + Hit3Pos(1) )/2 + (pow(R2,2) - pow(R3,2) )*( Hit3Pos(1) - Hit2Pos(1) )/2/pow( R32,2 ) - 0.5*( Hit2Pos(0) - Hit3Pos(0) )*sqrt( 2 * (pow(R2,2) + pow(R3,2) )/pow( R32,2 ) - pow( pow(R2,2) - pow(R3,2), 2 )/pow( R32, 4 ) - 1 );
		
		Points[4][0] = (Hit1Pos(0) + Hit3Pos(0) )/2 + (pow(R3,2) - pow(R1,2) )*( Hit1Pos(0) - Hit3Pos(0) )/2/pow( R13,2 ) + 0.5*( Hit1Pos(1) - Hit3Pos(1) )*sqrt( 2 * (pow(R3,2) + pow(R1,2) )/pow( R13,2 ) - pow( pow(R3,2) - pow(R1,2), 2 )/pow( R13, 4 ) - 1 );
		Points[4][1] = (Hit1Pos(1) + Hit3Pos(1) )/2 + (pow(R3,2) - pow(R1,2) )*( Hit1Pos(1) - Hit3Pos(1) )/2/pow( R13,2 ) + 0.5*( Hit3Pos(0) - Hit1Pos(0) )*sqrt( 2 * (pow(R3,2) + pow(R1,2) )/pow( R13,2 ) - pow( pow(R3,2) - pow(R1,2), 2 )/pow( R13, 4 ) - 1 );
		Points[5][0] = (Hit1Pos(0) + Hit3Pos(0) )/2 + (pow(R3,2) - pow(R1,2) )*( Hit1Pos(0) - Hit3Pos(0) )/2/pow( R13,2 ) - 0.5*( Hit1Pos(1) - Hit3Pos(1) )*sqrt( 2 * (pow(R3,2) + pow(R1,2) )/pow( R13,2 ) - pow( pow(R3,2) - pow(R1,2), 2 )/pow( R13, 4 ) - 1 );
		Points[5][1] = (Hit1Pos(1) + Hit3Pos(1) )/2 + (pow(R3,2) - pow(R1,2) )*( Hit1Pos(1) - Hit3Pos(1) )/2/pow( R13,2 ) - 0.5*( Hit3Pos(0) - Hit1Pos(0) )*sqrt( 2 * (pow(R3,2) + pow(R1,2) )/pow( R13,2 ) - pow( pow(R3,2) - pow(R1,2), 2 )/pow( R13, 4 ) - 1 );
		
		MinDistance = 1000000.;
		for( unsigned i=0; i<2; i++ )
		{
			for( unsigned j=0; j<2; j++ )
			{
				for( unsigned k=0; k<2; k++ )
				{
					Distance = sqrt( pow(Points[i][0] - Points[j+2][0], 2) + pow(Points[i][1] - Points[j+2][1],2 ) ) + sqrt( pow(Points[i][0] - Points[k+4][0], 2) + pow(Points[i][1] - Points[k+4][1],2 ) ) + sqrt( pow(Points[k+4][0] - Points[j+2][0], 2) + pow(Points[k+4][1] - Points[j+2][1],2 ) );
					if( Distance < MinDistance )
					{
						MinDistance = Distance;
						temp.clear();
						temp.push_back( Points[i][0] );
						temp.push_back( Points[i][1] );
						temp.push_back( Points[2+j][0] );
						temp.push_back( Points[2+j][1] );
						temp.push_back( Points[4+k][0] );
						temp.push_back( Points[4+k][1] );
					}
				}	
			}
		}
		test++;
		if( test % 50 == 0 )
		{
			if( MinDistance == 1000000. )
			{
				temp.clear();
				temp.push_back(100.);
				temp.push_back(100.);
				temp.push_back(100.);
				temp.push_back(100.);
				temp.push_back(100.);
				temp.push_back(100.);
				break;
			}
		}
		if( MinDistance > PreviousDistance )
			test = 0;
		else
		{
			PreviousDistance = MinDistance;
			temp2 = temp;
		}
		R0 += 1;
	}
	TVector3 RecoPoint( (temp[0]+temp[2]+temp[4])/3, (temp[1]+temp[3]+temp[5])/3, Hit1Pos(2) );
	
	return RecoPoint;
}*/
TVector3 EventCategorizerTools::FindIntersection( TVector3 Hit1Pos, TVector3 Hit2Pos, TVector3 Hit3Pos, double t21, double t31 )
{
	/*TVector3 Hit1Pos( Hit1.getPosX(), Hit1.getPosY(), Hit1.getPosZ() );
	TVector3 Hit2Pos( Hit2.getPosX(), Hit2.getPosY(), Hit2.getPosZ() );
	TVector3 Hit3Pos( Hit3.getPosX(), Hit3.getPosY(), Hit3.getPosZ() );  
	double t21 = Hit2.getTime()/1000 - Hit1.getTime()/1000;
	double t31 = Hit3.getTime()/1000 - Hit1.getTime()/1000;*/
  
	double R21 = sqrt( pow(Hit2Pos(0) - Hit1Pos(0),2 ) + pow(Hit2Pos(1) - Hit1Pos(1),2 ) );
	double R32 = sqrt( pow(Hit3Pos(0) - Hit2Pos(0),2 ) + pow(Hit3Pos(1) - Hit2Pos(1),2 ) );
	double R13 = sqrt( pow(Hit1Pos(0) - Hit3Pos(0),2 ) + pow(Hit1Pos(1) - Hit3Pos(1),2 ) );
	
	double TDiffTOR1 = 0.;
	double TDiffTOR2 = t21;
	double TDiffTOR3 = t31;
	
	TDiffTOR2 = 29.979246*TDiffTOR2;
	TDiffTOR3 = 29.979246*TDiffTOR3;
	
	double R0 = 0.;
	
	if( R0 < (R21 - TDiffTOR2 )/2  )
	  R0 = (R21 - TDiffTOR2 )/2;
	if( R0 < (R32 - TDiffTOR2 - TDiffTOR3 )/2  )
	  R0 = (R32 - TDiffTOR2 - TDiffTOR3 )/2;
	if( R0 < (R13 - TDiffTOR3 )/2  )
	  R0 = (R13 - TDiffTOR3 )/2;
	
	double R1 = 0.;
	double R2 = 0.;
	double R3 = 0.;
	std::vector<double> temp, temp2;
	std::vector< std::vector<double> > Points;
	temp.push_back(0.);
	temp.push_back(0.);
	Points.push_back(temp);
	Points.push_back(temp);
	Points.push_back(temp);
	Points.push_back(temp);
	Points.push_back(temp);
	Points.push_back(temp);
	double Distance = 0.;
	double MinDistance = 0.;
	double PreviousDistance = 10000000.;
	
	temp.clear();
	
	int test = 1;
	while( test )
	{
		R1 = TDiffTOR1 + R0+1;
		R2 = TDiffTOR2 + R0+1;
		R3 = TDiffTOR2 + R0+1;
		Points = FindIntersectionPointsOfCircles( Hit1Pos, Hit2Pos, Hit3Pos, R1, R2, R3, R13, R21, R32 );
		
		MinDistance = 1000000.;
		for( unsigned i=0; i<2; i++ )
		{
			for( unsigned j=0; j<2; j++ )
			{
				for( unsigned k=0; k<2; k++ )
				{
					Distance = sqrt( pow(Points[i][0] - Points[j+2][0], 2) + pow(Points[i][1] - Points[j+2][1],2 ) ) + sqrt( pow(Points[i][0] - Points[k+4][0], 2) + pow(Points[i][1] - Points[k+4][1],2 ) ) + sqrt( pow(Points[k+4][0] - Points[j+2][0], 2) + pow(Points[k+4][1] - Points[j+2][1],2 ) );
					if( Distance < MinDistance )
					{
						MinDistance = Distance;
						temp.clear();
						temp.push_back( Points[i][0] );
						temp.push_back( Points[i][1] );
						temp.push_back( Points[2+j][0] );
						temp.push_back( Points[2+j][1] );
						temp.push_back( Points[4+k][0] );
						temp.push_back( Points[4+k][1] );
					}
				}	
			}
		}
		test++;
		if( test % 50 == 0 )
		{
			if( MinDistance == 1000000. )
			{
				temp.clear();
				temp.push_back(100.);
				temp.push_back(100.);
				temp.push_back(100.);
				temp.push_back(100.);
				temp.push_back(100.);
				temp.push_back(100.);
				break;
			}
		}
		if( MinDistance > PreviousDistance )
			test = 0;
		else
		{
			PreviousDistance = MinDistance;
			temp2 = temp;
		}
		R0 += 1;
	}
	std::vector<double> R0s, Distances;
	if( MinDistance != 1000000. )
		test = 1;
	
	double MinnDistance = 1000000.;
	while( test )
	{
		R1 = TDiffTOR1 + R0+1;
		R2 = TDiffTOR2 + R0+1;
		R3 = TDiffTOR2 + R0+1;
		Points = FindIntersectionPointsOfCircles( Hit1Pos, Hit2Pos, Hit3Pos, R1, R2, R3, R13, R21, R32 );

		MinDistance = 1000000.;
		for( unsigned i=0; i<2; i++ )
		{
			for( unsigned j=0; j<2; j++ )
			{
				for( unsigned k=0; k<2; k++ )
				{
					Distance = sqrt( pow(Points[i][0] - Points[j+2][0], 2) + pow(Points[i][1] - Points[j+2][1],2 ) ) + sqrt( pow(Points[i][0] - Points[k+4][0], 2) + pow(Points[i][1] - Points[k+4][1],2 ) ) + sqrt( pow(Points[k+4][0] - Points[j+2][0], 2) + pow(Points[k+4][1] - Points[j+2][1],2 ) );
					if( Distance < MinDistance )
					{
						MinDistance = Distance;
						temp.clear();
						temp.push_back( Points[i][0] );
						temp.push_back( Points[i][1] );
						temp.push_back( Points[2+j][0] );
						temp.push_back( Points[2+j][1] );
						temp.push_back( Points[4+k][0] );
						temp.push_back( Points[4+k][1] );
					}
				}	
			}
		}
		if( MinDistance != 1000000. )
		{
			R0s.push_back( R0 );
			Distances.push_back( MinDistance );
		}
		
		test++;
		if( test % 50 == 0 )
		{
			if( MinDistance == 1000000. )
			{
				temp.clear();
				temp.push_back(100.);
				temp.push_back(100.);
				temp.push_back(100.);
				temp.push_back(100.);
				temp.push_back(100.);
				temp.push_back(100.);
				break;
			}
			test = 0;
		}
		if( MinDistance < MinnDistance )
		  MinnDistance = MinDistance;
		
		PreviousDistance = MinDistance;
		temp2 = temp;
		R0 -= 0.1;
	}
	
	if( MinnDistance != 1000000. )
	{
		double R0Min; 
		double minEle = *std::min_element( std::begin(Distances), std::end(Distances) );
		if( minEle == Distances[0] )
		{
			R0Min = R0s[0];
		}
		else if( minEle == Distances[ Distances.size() - 1 ] )
		{
			R0Min = R0s[ R0s.size() -1 ];	  
		}
		else
		{
			//R0Min = FindMinFromQuadraticFit( R0s, Distances );
			R0Min = FindMinFromDerrivative( R0s, Distances );
		}
		R1 = TDiffTOR1 + R0Min+1;
		R2 = TDiffTOR2 + R0Min+1;
		R3 = TDiffTOR2 + R0Min+1;
		Points = FindIntersectionPointsOfCircles( Hit1Pos, Hit2Pos, Hit3Pos, R1, R2, R3, R13, R21, R32 );
		
		MinDistance = 1000000.;
		for( unsigned i=0; i<2; i++ )
		{
			for( unsigned j=0; j<2; j++ )
			{
				for( unsigned k=0; k<2; k++ )
				{
					Distance = sqrt( pow(Points[i][0] - Points[j+2][0], 2) + pow(Points[i][1] - Points[j+2][1],2 ) ) + sqrt( pow(Points[i][0] - Points[k+4][0], 2) + pow(Points[i][1] - Points[k+4][1],2 ) ) + sqrt( pow(Points[k+4][0] - Points[j+2][0], 2) + pow(Points[k+4][1] - Points[j+2][1],2 ) );
					if( Distance < MinDistance )
					{
						MinDistance = Distance;
						temp.clear();
						temp.push_back( Points[i][0] );
						temp.push_back( Points[i][1] );
						temp.push_back( Points[2+j][0] );
						temp.push_back( Points[2+j][1] );
						temp.push_back( Points[4+k][0] );
						temp.push_back( Points[4+k][1] );
					}
				}	
			}
		}
	
	}
	
	TVector3 RecoPoint( (temp[0]+temp[2]+temp[4])/3, (temp[1]+temp[3]+temp[5])/3, Hit1Pos(2) );
	
	return RecoPoint;
}


double EventCategorizerTools::FindMinFromQuadraticFit( std::vector<double> Arg, std::vector<double> Val )
{
	double X=0., X2=0., X3=0., X4=0., Y=0., XY=0., X2Y=0.;
	for( unsigned i=0; i<Arg.size(); i++ )
	{
		X += Arg[i];
		X2 += Arg[i]*Arg[i];
		X3 += Arg[i]*Arg[i]*Arg[i];
		X4 += Arg[i]*Arg[i]*Arg[i]*Arg[i];
		Y += Val[i];
		XY += Arg[i]*Val[i];
		X2Y += Arg[i]*Arg[i]*Val[i];
	}
	double W = Arg.size()*X2*X4 + X*X3*X2 + X2*X*X3 - X2*X2*X2 - X3*X3*Arg.size() - X*X*X4;
	//double Wa0 = Y*X2*X4 + XY*X3*X2 + X2Y*X*X3 - X2*X2*X2Y - X3*X3*Y - X*XY*X4;
	double Wa1 = Arg.size()*XY*X4 + Y*X3*X2 + X2*X*X2Y - X2*XY*X2 - X3*X2Y*Arg.size() - X*Y*X4;
	double Wa2 = Arg.size()*X2*X2Y + X*XY*X2 + Y*X*X3 - Y*X2*X2 - XY*X3*Arg.size() - X*X*X2Y;
	
	double a1,a2;
	
	if( W )
	{
		a1 = Wa1/W;
		a2 = Wa2/W;
	}
	else
	{
		std::cout << "What?, Error, big one, impossibru.." << std::endl; 
		return -1;
	}
	
	return -a1/2/a2;  
}

double EventCategorizerTools::FindMinFromDerrivative( std::vector<double> Arg, std::vector<double> Val )
{
	double Derr1 = Val[1] - Val[0];
	unsigned StopInd = 0;
	for( unsigned i=1; i<Val.size()-1; i++ )
	{
		if( Derr1 < 0 )
		{
			Derr1 = Val[i+1] - Val[i];
			if(Derr1 > 0)
			{
				StopInd = i;
				break;
			}
		}
	}
	double a = ( Val[StopInd+1] - Val[StopInd] - (Val[StopInd] - Val[StopInd-1]) )/( (Arg[StopInd+1] + Arg[StopInd])/2 - (Arg[StopInd] + Arg[StopInd-1])/2 );
	double b = Val[StopInd+1] - Val[StopInd] - a*(Arg[StopInd+1] + Arg[StopInd])/2;
	if( a )
		return -b/a;
	else
	{
		std::cout << "Error, wtf, error, no idea?!?" << std::endl;
		return 0;
	}
	
}

std::vector< std::vector<double> > EventCategorizerTools::FindIntersectionPointsOfCircles( TVector3 Hit1Pos, TVector3 Hit2Pos, TVector3 Hit3Pos, double R1, double R2, double R3, double R13, double R21, double R32 )
{
	std::vector< std::vector<double> > Points;
	std::vector<double> temp;
	temp.push_back(0);
	temp.push_back(0);
	Points.push_back(temp);
	Points.push_back(temp);
	Points.push_back(temp);
	Points.push_back(temp);
	Points.push_back(temp);
	Points.push_back(temp);
	
	Points[0][0] = (Hit1Pos(0) + Hit2Pos(0) )/2 + (pow(R1,2) - pow(R2,2) )*( Hit2Pos(0) - Hit1Pos(0) )/2/pow( R21,2 ) + 0.5*( Hit2Pos(1) - Hit1Pos(1) )*sqrt( 2 * (pow(R1,2) + pow(R2,2) )/pow( R21,2 ) - pow( pow(R1,2) - pow(R2,2), 2 )/pow( R21, 4 ) - 1 );
	Points[0][1] = (Hit1Pos(1) + Hit2Pos(1) )/2 + (pow(R1,2) - pow(R2,2) )*( Hit2Pos(1) - Hit1Pos(1) )/2/pow( R21,2 ) + 0.5*( Hit1Pos(0) - Hit2Pos(0) )*sqrt( 2 * (pow(R1,2) + pow(R2,2) )/pow( R21,2 ) - pow( pow(R1,2) - pow(R2,2), 2 )/pow( R21, 4 ) - 1 );
	Points[1][0] = (Hit1Pos(0) + Hit2Pos(0) )/2 + (pow(R1,2) - pow(R2,2) )*( Hit2Pos(0) - Hit1Pos(0) )/2/pow( R21,2 ) - 0.5*( Hit2Pos(1) - Hit1Pos(1) )*sqrt( 2 * (pow(R1,2) + pow(R2,2) )/pow( R21,2 ) - pow( pow(R1,2) - pow(R2,2), 2 )/pow( R21, 4 ) - 1 );
	Points[1][1] = (Hit1Pos(1) + Hit2Pos(1) )/2 + (pow(R1,2) - pow(R2,2) )*( Hit2Pos(1) - Hit1Pos(1) )/2/pow( R21,2 ) - 0.5*( Hit1Pos(0) - Hit2Pos(0) )*sqrt( 2 * (pow(R1,2) + pow(R2,2) )/pow( R21,2 ) - pow( pow(R1,2) - pow(R2,2), 2 )/pow( R21, 4 ) - 1 );
	
	Points[2][0] = (Hit2Pos(0) + Hit3Pos(0) )/2 + (pow(R2,2) - pow(R3,2) )*( Hit3Pos(0) - Hit2Pos(0) )/2/pow( R32,2 ) + 0.5*( Hit3Pos(1) - Hit2Pos(1) )*sqrt( 2 * (pow(R2,2) + pow(R3,2) )/pow( R32,2 ) - pow( pow(R2,2) - pow(R3,2), 2 )/pow( R32, 4 ) - 1 );
	Points[2][1] = (Hit2Pos(1) + Hit3Pos(1) )/2 + (pow(R2,2) - pow(R3,2) )*( Hit3Pos(1) - Hit2Pos(1) )/2/pow( R32,2 ) + 0.5*( Hit2Pos(0) - Hit3Pos(0) )*sqrt( 2 * (pow(R2,2) + pow(R3,2) )/pow( R32,2 ) - pow( pow(R2,2) - pow(R3,2), 2 )/pow( R32, 4 ) - 1 );
	Points[3][0] = (Hit2Pos(0) + Hit3Pos(0) )/2 + (pow(R2,2) - pow(R3,2) )*( Hit3Pos(0) - Hit2Pos(0) )/2/pow( R32,2 ) - 0.5*( Hit3Pos(1) - Hit2Pos(1) )*sqrt( 2 * (pow(R2,2) + pow(R3,2) )/pow( R32,2 ) - pow( pow(R2,2) - pow(R3,2), 2 )/pow( R32, 4 ) - 1 );
	Points[3][1] = (Hit2Pos(1) + Hit3Pos(1) )/2 + (pow(R2,2) - pow(R3,2) )*( Hit3Pos(1) - Hit2Pos(1) )/2/pow( R32,2 ) - 0.5*( Hit2Pos(0) - Hit3Pos(0) )*sqrt( 2 * (pow(R2,2) + pow(R3,2) )/pow( R32,2 ) - pow( pow(R2,2) - pow(R3,2), 2 )/pow( R32, 4 ) - 1 );
	
	Points[4][0] = (Hit1Pos(0) + Hit3Pos(0) )/2 + (pow(R3,2) - pow(R1,2) )*( Hit1Pos(0) - Hit3Pos(0) )/2/pow( R13,2 ) + 0.5*( Hit1Pos(1) - Hit3Pos(1) )*sqrt( 2 * (pow(R3,2) + pow(R1,2) )/pow( R13,2 ) - pow( pow(R3,2) - pow(R1,2), 2 )/pow( R13, 4 ) - 1 );
	Points[4][1] = (Hit1Pos(1) + Hit3Pos(1) )/2 + (pow(R3,2) - pow(R1,2) )*( Hit1Pos(1) - Hit3Pos(1) )/2/pow( R13,2 ) + 0.5*( Hit3Pos(0) - Hit1Pos(0) )*sqrt( 2 * (pow(R3,2) + pow(R1,2) )/pow( R13,2 ) - pow( pow(R3,2) - pow(R1,2), 2 )/pow( R13, 4 ) - 1 );
	Points[5][0] = (Hit1Pos(0) + Hit3Pos(0) )/2 + (pow(R3,2) - pow(R1,2) )*( Hit1Pos(0) - Hit3Pos(0) )/2/pow( R13,2 ) - 0.5*( Hit1Pos(1) - Hit3Pos(1) )*sqrt( 2 * (pow(R3,2) + pow(R1,2) )/pow( R13,2 ) - pow( pow(R3,2) - pow(R1,2), 2 )/pow( R13, 4 ) - 1 );
	Points[5][1] = (Hit1Pos(1) + Hit3Pos(1) )/2 + (pow(R3,2) - pow(R1,2) )*( Hit1Pos(1) - Hit3Pos(1) )/2/pow( R13,2 ) - 0.5*( Hit3Pos(0) - Hit1Pos(0) )*sqrt( 2 * (pow(R3,2) + pow(R1,2) )/pow( R13,2 ) - pow( pow(R3,2) - pow(R1,2), 2 )/pow( R13, 4 ) - 1 );
	
	return Points;
}


int EventCategorizerTools::CheckIfScattered( JPetHit Hit1, JPetHit Hit2, double ErrorInterval )
{
	double TDiff = fabs(Hit2.getTime()/1000 - Hit1.getTime()/1000);
	TVector3 vec1( Hit2.getPosX() - Hit1.getPosX(), Hit2.getPosY() - Hit1.getPosY(), Hit2.getPosZ() - Hit1.getPosZ() );
	double Distance = vec1.Mag();
	double LightVel = 29.979246; 
	double ScattTime = Distance/LightVel;
	if( fabs( TDiff - ScattTime ) < ErrorInterval )
		return 1;
	else
		return 0;
	
}

TVector3 EventCategorizerTools::AlekReconstruct( JPetHit Hit1, JPetHit Hit2, JPetHit Hit3 )
{
  
  TVector3 Hit1Pos( Hit1.getPosX(), Hit1.getPosY(), Hit1.getPosZ() );
  TVector3 Hit2Pos( Hit2.getPosX(), Hit2.getPosY(), Hit2.getPosZ() );
  TVector3 Hit3Pos( Hit3.getPosX(), Hit3.getPosY(), Hit3.getPosZ() );  
  double t1 = Hit1.getTime()/1000;
  double t2 = Hit2.getTime()/1000;
  double t3 = Hit3.getTime()/1000;
  
  // find the decay plane
  TVector3 normal = ((Hit2Pos-Hit1Pos).Cross( Hit3Pos-Hit1Pos )).Unit();
  
  Double_t cvel = 29.979246;
  
  double times[3];
  times[0] = t1;
  times[1] = t2;
  times[2] = t3;
  // prepare transformation to the decay plane
  TVector3 z(0.,0.,1.);
  TVector3 rotAxis = normal.Cross( z );
  double angle = z.Angle( normal ); // radians
  
  TRotation rot;
  rot.Rotate(angle, rotAxis);
  
  // transform gamma hits to decay plane
  std::cout << "\n2D:" << std::endl;
  TVector3 gammas2D[3];
 /* for(int i=0;i<3;i++){
    gammas2D[i] = rot * getGammaHit(i);
  }*/
    gammas2D[0] = rot * Hit1Pos;
    gammas2D[1] = rot * Hit2Pos;
    gammas2D[2] = rot * Hit3Pos;
  
  // solve in 2D
  int combs[][2] = {{0,1}, {1,2}, {0,2}};
  double M[3][3];
  double D[3];
  
  // fill the matrix and constants vector
  int i,j;
  for(int k=0;k<3;++k){ // k - rows
    i = combs[k][0];
    j = combs[k][1];
    M[k][0] = 2.*( gammas2D[i].X() - gammas2D[j].X() );
    M[k][1] = 2.*( gammas2D[i].Y() - gammas2D[j].Y() );
    M[k][2] = 2.*cvel*cvel*( times[j] - times[i] );       
    D[k] = pow(gammas2D[i].X(),2.)
      - pow(gammas2D[j].X(),2.)
      + pow(gammas2D[i].Y(),2.)
      - pow(gammas2D[j].Y(),2.)
      - cvel*cvel*pow(times[i],2.)
      + cvel*cvel*pow(times[j],2.);
  }

  /*
  for(int k=0;k<3;++k){ // k - rows
    std::cout << "m1 = " << M[k][0] << std::endl;
    std::cout << "m2 = " << M[k][1] << std::endl;
    std::cout << "m3 = " << M[k][2] << std::endl;
    std::cout << "D = " << D[k] << std::endl;
  }
  */
  
  // use analytical solutions: x = Ex*t+Fx, y=Ey*t+Fy
  double Ex,Ey,Fx,Fy;
  Ex = ( M[0][2]*M[1][1]-M[0][1]*M[1][2] )/( M[0][1]*M[1][0]-M[0][0]*M[1][1] );
  Fx = ( M[0][1]*D[1]-M[1][1]*D[0] )/( M[0][1]*M[1][0]-M[0][0]*M[1][1] );
  
  Ey = ( M[0][0]*M[1][2] - M[0][2]*M[1][0] )/( M[0][1]*M[1][0]-M[0][0]*M[1][1] );
  Fy = ( M[1][0]*D[0] - M[0][0]*D[1] )/( M[0][1]*M[1][0]-M[0][0]*M[1][1] );       
  
  // find t - using ready analytical solutions
  double a,b,cc,delta;
  
  
  TVector3 testy(1000,1000,1000);
  TVector3 sol_hit[2];
  Double_t sol_time[2];
  
  a = Ex*Ex + Ey*Ey - cvel*cvel;
  b = 2.*( Ex*(Fx-gammas2D[0].X()) + Ey*(Fy-gammas2D[0].Y()) + cvel*cvel*times[0] );
  cc = pow(Fx-gammas2D[0].X(), 2.) + pow(Fy-gammas2D[0].Y(), 2.) - cvel*cvel*pow(times[0], 2.);
  delta = b*b - 4.*a*cc;
  
    if( delta < 0. )
    {
      return testy;
    }
    
    sol_time[0] = (-1.*b - sqrt(delta))/(2.*a);
    sol_time[1] = (-1.*b + sqrt(delta))/(2.*a);
    
    for(int i = 0; i<2;++i){
      TVector3 sol2Dv( Ex*sol_time[i]+Fx, Ey*sol_time[i]+Fy, gammas2D[0].Z() );
      
      // transform the solution back to 3D
      sol_hit[i] =  rot.Inverse() * sol2Dv;
      
    }
    
    // check solution 2 for reasonability
   /* if( errFlag == 0 ){
      if( sol_time[1] < -20000. || sol_time[1] > 20000.  ){
	errFlag = 2;
      }else if( sol_hit[1].Perp() > Hit2Pos.Perp() ||
		fabs( sol_hit[1].Z() ) > fStripLength/2. ){
	errFlag = 3;
      }else if( TMath::IsNaN( sol_time[1] ) ||
		TMath::IsNaN( sol_hit[1].X() ) ||
		TMath::IsNaN( sol_hit[1].Y() ) ||
		TMath::IsNaN( sol_hit[1].Z() )
		){
	errFlag = 4;
      }
    }*/
    
  
  return sol_hit[1];
}


TVector3 EventCategorizerTools::AlekReconstruct2( JPetHit Hit1, JPetHit Hit2, JPetHit Hit3 )
{
  
  TVector3 Hit1Pos( Hit1.getPosX(), Hit1.getPosY(), Hit1.getPosZ() );
  TVector3 Hit2Pos( Hit2.getPosX(), Hit2.getPosY(), Hit2.getPosZ() );
  TVector3 Hit3Pos( Hit3.getPosX(), Hit3.getPosY(), Hit3.getPosZ() );  
  double t1 = Hit1.getTime()/1000;
  double t2 = Hit2.getTime()/1000;
  double t3 = Hit3.getTime()/1000;
  // find the decay plane
  TVector3 normal = ((Hit2Pos-Hit1Pos).Cross( Hit3Pos-Hit1Pos )).Unit();
  
  Double_t cvel = 29.979246;
  
  double times[3];
  times[0] = t1;
  times[1] = t2;
  times[2] = t3;
  // prepare transformation to the decay plane
  TVector3 z(0.,0.,1.);
  TVector3 rotAxis = normal.Cross( z );
  double angle = z.Angle( normal ); // radians
  
  TRotation rot;
  rot.Rotate(angle, rotAxis);
  
  // transform gamma hits to decay plane
  std::cout << "\n2D:" << std::endl;
  TVector3 gammas2D[3];
 /* for(int i=0;i<3;i++){
    gammas2D[i] = rot * getGammaHit(i);
  }*/
    gammas2D[0] = rot * Hit1Pos;
    gammas2D[1] = rot * Hit2Pos;
    gammas2D[2] = rot * Hit3Pos;
  
  // solve in 2D
  int combs[][2] = {{0,1}, {1,2}, {0,2}};
  double M[3][3];
  double D[3];
  
  // fill the matrix and constants vector
  int i,j;
  for(int k=0;k<3;++k){ // k - rows
    i = combs[k][0];
    j = combs[k][1];
    M[k][0] = 2.*( gammas2D[i].X() - gammas2D[j].X() );
    M[k][1] = 2.*( gammas2D[i].Y() - gammas2D[j].Y() );
    M[k][2] = 2.*cvel*cvel*( times[j] - times[i] );       
    D[k] = pow(gammas2D[i].X(),2.)
      - pow(gammas2D[j].X(),2.)
      + pow(gammas2D[i].Y(),2.)
      - pow(gammas2D[j].Y(),2.)
      - cvel*cvel*pow(times[i],2.)
      + cvel*cvel*pow(times[j],2.);
  }

  /*
  for(int k=0;k<3;++k){ // k - rows
    std::cout << "m1 = " << M[k][0] << std::endl;
    std::cout << "m2 = " << M[k][1] << std::endl;
    std::cout << "m3 = " << M[k][2] << std::endl;
    std::cout << "D = " << D[k] << std::endl;
  }
  */
  
  // use analytical solutions: x = Ex*t+Fx, y=Ey*t+Fy
  double Ex,Ey,Fx,Fy;
  Ex = ( M[0][2]*M[1][1]-M[0][1]*M[1][2] )/( M[0][1]*M[1][0]-M[0][0]*M[1][1] );
  Fx = ( M[0][1]*D[1]-M[1][1]*D[0] )/( M[0][1]*M[1][0]-M[0][0]*M[1][1] );
  
  Ey = ( M[0][0]*M[1][2] - M[0][2]*M[1][0] )/( M[0][1]*M[1][0]-M[0][0]*M[1][1] );
  Fy = ( M[1][0]*D[0] - M[0][0]*D[1] )/( M[0][1]*M[1][0]-M[0][0]*M[1][1] );       
  
  // find t - using ready analytical solutions
  double a,b,cc,delta;
  
  
  TVector3 testy(1000,1000,1000);
  TVector3 sol_hit[2];
  Double_t sol_time[2];
  
  a = Ex*Ex + Ey*Ey - cvel*cvel;
  b = 2.*( Ex*(Fx-gammas2D[0].X()) + Ey*(Fy-gammas2D[0].Y()) + cvel*cvel*times[0] );
  cc = pow(Fx-gammas2D[0].X(), 2.) + pow(Fy-gammas2D[0].Y(), 2.) - cvel*cvel*pow(times[0], 2.);
  delta = b*b - 4.*a*cc;
  
    if( delta < 0. )
    {
      return testy;
    }
    
    sol_time[0] = (-1.*b - sqrt(delta))/(2.*a);
    sol_time[1] = (-1.*b + sqrt(delta))/(2.*a);
    
    TVector3 Sol1( Ex*sol_time[0]+Fx, Ey*sol_time[0]+Fy, 0 );
    TVector3 Sol2( Ex*sol_time[1]+Fx, Ey*sol_time[1]+Fy, 0 );
    int BttSol = ( CalcDistFromCentres( Sol1, gammas2D[0], gammas2D[1], gammas2D[2] ) < CalcDistFromCentres( Sol2, gammas2D[0], gammas2D[1], gammas2D[2] ) ? 0 : 1 );
    
    for(int i = 0; i<2;++i){
      TVector3 sol2Dv( Ex*sol_time[i]+Fx, Ey*sol_time[i]+Fy, gammas2D[0].Z() );
      
      // transform the solution back to 3D
      sol_hit[i] =  rot.Inverse() * sol2Dv;
      
    }
    
  
  return sol_hit[BttSol];
}

double EventCategorizerTools::CalcDistFromCentres( TVector3 Sol1, TVector3 Gamma1, TVector3 Gamma2, TVector3 Gamma3 )
{
	double distance1 = 0.;
	distance1 = sqrt( pow(Sol1(0) - Gamma1(0),2) + pow(Sol1(1) - Gamma1(1),2) + pow(Sol1(2) - Gamma1(2),2) ) + sqrt( pow(Sol1(0) - Gamma2(0),2) + pow(Sol1(1) - Gamma2(1),2) + pow(Sol1(2) - Gamma2(2),2) ) + sqrt( pow(Sol1(0) - Gamma3(0),2) + pow(Sol1(1) - Gamma3(1),2) + pow(Sol1(2) - Gamma3(2),2) );		
	return distance1;
}

double EventCategorizerTools::NormalizeTime( JPetHit Hit1 )
{
	TVector3 vec1( Hit1.getPosX(), Hit1.getPosY(), Hit1.getPosZ() );
	double Length0 = vec1.Mag();
	return Hit1.getTime()/1000 - (Length0)/29.979246;
}

TVector3 EventCategorizerTools::RecoPosition( const JPetHit & Hit1, const JPetHit & Hit2)
{
	TVector3 ReconstructedPosition;
	//double tof = fabs( JPetHitUtils::getTimeAtThr(Hit1, 1) - JPetHitUtils::getTimeAtThr(Hit2, 1) )/1000;
	double tof = fabs( Hit1.getTime() - Hit2.getTime() )/1000;
	double VecLength = sqrt( pow(Hit1.getPosX()-Hit2.getPosX(),2)	// Pos in cm
			+pow(Hit1.getPosY()-Hit2.getPosY(),2)
			+pow(Hit1.getPosZ()-Hit2.getPosZ(),2) );
	double middleX = (Hit1.getPosX()+Hit2.getPosX() )/2;
	double middleY = (Hit1.getPosY()+Hit2.getPosY() )/2;
	double middleZ = (Hit1.getPosZ()+Hit2.getPosZ() )/2;
	double Fraction = 2*tof*29.979246/VecLength;
	if( Hit1.getTime() >= Hit2.getTime() )
	{
		ReconstructedPosition(0) = middleX + Fraction*( Hit1.getPosX()-middleX );
		ReconstructedPosition(1) = middleY + Fraction*( Hit1.getPosY()-middleY );
		ReconstructedPosition(2) = middleZ + Fraction*( Hit1.getPosZ()-middleZ );
	}
	else
	{
		ReconstructedPosition(0) = middleX + Fraction*( Hit2.getPosX()-middleX );
		ReconstructedPosition(1) = middleY + Fraction*( Hit2.getPosY()-middleY );
		ReconstructedPosition(2) = middleZ + Fraction*( Hit2.getPosZ()-middleZ );
	}
	return ReconstructedPosition;
}