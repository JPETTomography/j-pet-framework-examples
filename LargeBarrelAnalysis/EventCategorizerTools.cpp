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

// Calculation of the TOT of the hit, calculated as the sum of the TOTs on all of the thresholds (1-4) and on the both sides (A,B)
double EventCategorizerTools::calcTOT( JPetHit hit ) 
{
	double TOT = 0.;
	
	std::vector<JPetSigCh> sigALead = hit.getSignalA().getRecoSignal().getRawSignal().getPoints( JPetSigCh::Leading, JPetRawSignal::ByThrNum );
	std::vector<JPetSigCh> sigBLead = hit.getSignalB().getRecoSignal().getRawSignal().getPoints( JPetSigCh::Leading, JPetRawSignal::ByThrNum );
	
	std::vector<JPetSigCh> sigATrail = hit.getSignalA().getRecoSignal().getRawSignal().getPoints( JPetSigCh::Trailing, JPetRawSignal::ByThrNum );
	std::vector<JPetSigCh> sigBTrail = hit.getSignalB().getRecoSignal().getRawSignal().getPoints( JPetSigCh::Trailing, JPetRawSignal::ByThrNum );

	for( unsigned i=1; i<=sigALead.size() && i<=sigATrail.size(); i++ )
	{
		TOT += ( sigATrail.at(i).getValue() - sigALead.at(i).getValue() )/1000;
	}
	for( unsigned i=1; i<=sigBLead.size() && i<=sigBTrail.size(); i++ )
	{
		TOT += ( sigBTrail.at(i).getValue() - sigBLead.at(i).getValue() )/1000;
	}
	
	return TOT;
}

// Calculation of scatter angle if the Hit1 represents primary Gamma registration, and Hit2 represents scatter gamma from Hit1. This function assumes that source of first gamma was in (0,0,0). Angle is calculated from scalar product
double EventCategorizerTools::calcScattAngle( JPetHit hit1, JPetHit hit2 ) 
{	
	double ScattAngle = hit1.getPos().Angle( hit2.getPos() - hit1.getPos() )*180/3.14159265;
	return ScattAngle;
}

// Calculation of time that potentially could be equal to time difference between two hits, if the first one was primary gamma, and Hit2 is coming from scatter of Hit1
double EventCategorizerTools::calcScattTime( JPetHit hit1, JPetHit hit2 ) 
{
	//light velocity in cm/s, returns time in ns
	return ( hit2.getPos() - hit1.getPos() ).Mag()/kLightVelocityCmS;  
}

// Calculation of distance between Hit1 and Hit2 positions
double EventCategorizerTools::calcDistance( JPetHit hit1, JPetHit hit2 ) 
{
	return ( hit1.getPos() - hit2.getPos() ).Mag();
}

// Calculation of angle between Hit1 and Hit2 positions assuming that the gamma quanta represented by Hits came from (0,0,0). Calculation from scalar product in 3D - taking all 3 coordinates
double EventCategorizerTools::calcAngle( JPetHit hit1, JPetHit hit2 ) 
{
	return hit1.getPos().Angle( hit2.getPos() )*180/3.14159265;
}

// Calculation of angle between Hit1 and Hit2 positions assuming that the gamma quanta represented by Hits came from (0,0,0). Calculation from scalar product in 2D - taking only 2 coordinates -> XY plane
double EventCategorizerTools::calcAngle2D( JPetHit hit1, JPetHit hit2 ) 
{
	TVector3 vec1( hit1.getPosX(), hit1.getPosY(), 0. );
	TVector3 vec2( hit2.getPosX(), hit2.getPosY(), 0. );
	return vec1.Angle( vec2 )*180/3.14159265;
}

// Calculation of relative angles between three hits - Result is vector of angles that contains: Angle between Hit1 and Hit2, Angle between Hit2 and Hit3, Angle between Hit3 and Hit1. Angles are calculated in 3D, form scalar product.
std::vector<double> EventCategorizerTools::calcAnglesFrom3Hit( JPetHit hit1, JPetHit hit2, JPetHit hit3 ) 
{
	std::vector<double> angles;
	double angle12 = calcAngle( hit1, hit2); // Angles - degrees
	double angle23 = calcAngle( hit2, hit3);
	double angle31 = calcAngle( hit3, hit1);
	angles.push_back( angle12 );
	angles.push_back( angle23 );
	angles.push_back( angle31 );
	std::sort( angles.begin(), angles.begin() +3 );
	return angles;
}

// Calculation of relative angles between three hits - Result is vector of angles that contains: Angle between Hit1 and Hit2, Angle between Hit2 and Hit3, Angle between Hit3 and Hit1. Angles are calculated in 2D, form scalar product.
std::vector<double> EventCategorizerTools::calcAngles2DFrom3Hit( JPetHit hit1, JPetHit hit2, JPetHit hit3 ) 
{
	std::vector<double> angles;
	double angle12 = calcAngle2D( hit1, hit2); // Angles - degrees
	double angle23 = calcAngle2D( hit2, hit3);
	double angle31 = calcAngle2D( hit3, hit1);
	angles.push_back( angle12 );
	angles.push_back( angle23 );
	angles.push_back( angle31 );
	std::sort( angles.begin(), angles.begin() +3 );
	return angles;
}

// Calculation of distance between plane based on Hit positions and (0,0,0)
double EventCategorizerTools::calcDistanceOfSurfaceAndZero( JPetHit hit1, JPetHit hit2, JPetHit hit3 ) 
{
	TVector3 crossProd  = ( hit2.getPos() - hit1.getPos() ).Cross( hit3.getPos() - hit2.getPos() );
	double Dcoeef = -crossProd(0)*hit2.getPosX() - crossProd(1)*hit2.getPosY() - crossProd(2)*hit2.getPosZ();
	return fabs(Dcoeef) / crossProd.Mag();
}

// Reconstruction of position for decay into 3 candidate
TVector3 EventCategorizerTools::recoPosition3Hit( JPetHit hit1, JPetHit hit2, JPetHit hit3 )
{
	TVector3 vecSurface;
			     
	vecSurface.SetX( (hit2.getPosY() - hit1.getPosY() )*(hit3.getPosZ() - hit1.getPosZ() ) - (hit2.getPosZ() - hit1.getPosZ() )*(hit3.getPosY() - hit1.getPosY() ) );
	vecSurface.SetY( (hit2.getPosZ() - hit1.getPosZ() )*(hit3.getPosX() - hit1.getPosX() ) - (hit2.getPosX() - hit1.getPosX() )*(hit3.getPosZ() - hit1.getPosZ() ) );
	vecSurface.SetZ( (hit2.getPosX() - hit1.getPosX() )*(hit3.getPosY() - hit1.getPosY() ) - (hit2.getPosY() - hit1.getPosY() )*(hit3.getPosX() - hit1.getPosX() ) );
	
	TVector3 vecPerpendicular( -vecSurface.Y(), vecSurface.X(), 0 );
	vecPerpendicular = vecPerpendicular.Unit();
	
	double theta = -acos( vecSurface.Z()/vecSurface.Mag() );
	
	TVector3 rotationX( cos(theta)+vecPerpendicular.X()*vecPerpendicular.X()*(1-cos(theta)), vecPerpendicular.X()*vecPerpendicular.Y()*(1-cos(theta)), vecPerpendicular.Y()*sin(theta) );
	TVector3 rotationY( vecPerpendicular.X()*vecPerpendicular.Y()*(1-cos(theta)), cos(theta)+vecPerpendicular.Y()*vecPerpendicular.Y()*(1-cos(theta)), -vecPerpendicular.X()*sin(theta) );
	TVector3 rotationZ( -vecPerpendicular.Y()*sin(theta), vecPerpendicular.X()*sin(theta), cos(theta) );
	
	TVector3 rotatedhit1Pos( rotationX*hit1.getPos(), rotationY*hit1.getPos(), rotationZ*hit1.getPos() );
	TVector3 rotatedhit2Pos( rotationX*hit2.getPos(), rotationY*hit2.getPos(), rotationZ*hit2.getPos() );
	TVector3 rotatedhit3Pos( rotationX*hit3.getPos(), rotationY*hit3.getPos(), rotationZ*hit3.getPos() );

	// Calculation of reconstructed position using self-implemented perturbative algorithm of finding intersection of three circles 
	TVector3 recoPoint = findIntersection( rotatedhit1Pos, rotatedhit2Pos, rotatedhit3Pos, hit2.getTime()/1000 - hit1.getTime()/1000, hit3.getTime()/1000 - hit1.getTime()/1000 );
	
	// if condition is fulfilled that means, reconstruction was unsuccessfull 
	if( recoPoint.X() == 100. && recoPoint.Y() == 100. )
		return recoPoint;
	
	TVector3 rotationXrev( cos(-theta)+vecPerpendicular.X()*vecPerpendicular.X()*(1-cos(-theta)), vecPerpendicular.X()*vecPerpendicular.Y()*(1-cos(-theta)), vecPerpendicular.Y()*sin(-theta) );
	TVector3 rotationYrev( vecPerpendicular.X()*vecPerpendicular.Y()*(1-cos(-theta)), cos(-theta)+vecPerpendicular.Y()*vecPerpendicular.Y()*(1-cos(-theta)), -vecPerpendicular.X()*sin(-theta) );
	TVector3 rotationZrev( -vecPerpendicular.Y()*sin(-theta), vecPerpendicular.X()*sin(-theta), cos(-theta) );
	
	TVector3 rotatedRecoPoint( rotationXrev*recoPoint, rotationYrev*recoPoint, rotationZrev*recoPoint );
	
	return rotatedRecoPoint;
	
}

// Self-implemented perturbative algorithm of finding intersection of three circles
TVector3 EventCategorizerTools::findIntersection( TVector3 hit1Pos, TVector3 hit2Pos, TVector3 hit3Pos, double t21, double t31 )
{
	// Method analyzing three circles in 2D. Each hitPosition is the centre of the circle. 
  
	// Calculating the differences of hitPositions in order to find minimal radius, so that the circles will intersect which each other
	double radius21 = sqrt( pow(hit2Pos(0) - hit1Pos(0),2 ) + pow(hit2Pos(1) - hit1Pos(1),2 ) );
	double radius32 = sqrt( pow(hit3Pos(0) - hit2Pos(0),2 ) + pow(hit3Pos(1) - hit2Pos(1),2 ) );
	double radius13 = sqrt( pow(hit1Pos(0) - hit3Pos(0),2 ) + pow(hit1Pos(1) - hit3Pos(1),2 ) );
	
	// Setting differences of radius so that, they fulfill the differences that come from real time differences between hits. Times are relative for the first hit
	double tDiffTOR1 = 0.;
	double tDiffTOR2 = kLightVelocityCmS*t21;
	double tDiffTOR3 = kLightVelocityCmS*t31;
		
	// Minimal radius added to each circle. This is the value that is variated in the proccess of finding intersection
	double radius0 = 0.;
	
	// Finding minmal radius that all the circle will intersect with each other, knowing the differences of hitPositions and time differences
	// radius1 = radius0
	// radius2 = radius0 + tDiffTOR2
	// radius3 = radius0 + tDiffTOR3
	// Intersection condition -> radius21 <= radius2 + radius1  -->   radius21 <= 2*radius0 + tDiffTOR2    --->    radius0 >= (radius21 - tDiffTOR2)/2
	// if the radius0 < (radius21 - tDiffTOR2)/2 that means radius0 is too small, and is set to be (radius21 - tDiffTOR2)/2. Similiar rest of the conditions
	// because they all should be fulfilled in order to circles to intersect
	if( radius0 < (radius21 - tDiffTOR2 )/2  )
		radius0 = (radius21 - tDiffTOR2 )/2;
	if( radius0 < (radius32 - tDiffTOR2 - tDiffTOR3 )/2  )
		radius0 = (radius32 - tDiffTOR2 - tDiffTOR3 )/2;
	if( radius0 < (radius13 - tDiffTOR3 )/2  )
		radius0 = (radius13 - tDiffTOR3 )/2;
	
	// radius of circles, initialization 
	double radius1 = 0.;
	double radius2 = 0.;
	double radius3 = 0.;
	
	// Containers for checking if algorithm is close to solution
	std::vector<double> bestPointInIteration, bestPointFromPreviousIteration;
	
	// Container for points of intersection for each value of radius0
	std::vector< std::vector<double> > points;
	bestPointInIteration.push_back(0.);
	bestPointInIteration.push_back(0.);
	points.push_back(bestPointInIteration);
	points.push_back(bestPointInIteration);
	points.push_back(bestPointInIteration);
	points.push_back(bestPointInIteration);
	points.push_back(bestPointInIteration);
	points.push_back(bestPointInIteration);
	bestPointInIteration.clear();
	
	double distance = 0.;
	double minDistance = 0.;
	double previousDistance = 10000000.;
	
	// Iteration number is for control purpose, because algorithm itself should find quickly solution in first loop, but for some cases -- strange scatterings -- slgorithm is unable to reconstruct position
	// Here control is 50 iterations. If algorithm is unable to find solution in 50 iterations it returns "bad point" = (100, 100 ,100)
	int iteration = 1;
	
	// There are three loops in the proccess of reconstruction:
	// First with step +1 cm to the radius0, looking briefly for radius, where minimal distance between three given intersection points is starting to grow
	// Second with step -0.1 cm to the radius0 from the last radius0 from the first loop, where similiar procedure is done but with opposite way and with smaller step
	// Third loop is for reconstruction of radius0 so the distance between three given intersection points will be the smallest
	while( iteration )
	{
		// Initialization of radius of each circle
		radius1 = tDiffTOR1 + radius0 + 1;
		radius2 = tDiffTOR2 + radius0 + 1;
		radius3 = tDiffTOR2 + radius0 + 1;
		
		// Looking for intersection points 
		points = findIntersectionPointsOfCircles( hit1Pos, hit2Pos,hit3Pos, radius1, radius2, radius3, radius13, radius21, radius32 );
		
		minDistance = 1000000.;
		
		// Finding minimal distance between calculated intersection points
		for( unsigned i=0; i<2; i++ )
		{
			for( unsigned j=0; j<2; j++ )
			{
				for( unsigned k=0; k<2; k++ )
				{
					distance = sqrt( pow(points[i][0] - points[j+2][0], 2) + pow(points[i][1] - points[j+2][1],2 ) ) + sqrt( pow(points[i][0] - points[k+4][0], 2) + pow(points[i][1] - points[k+4][1],2 ) ) + sqrt( pow(points[k+4][0] - points[j+2][0], 2) + pow(points[k+4][1] - points[j+2][1],2 ) );
					if( distance < minDistance )
					{
						minDistance = distance;
						bestPointInIteration.clear();
						bestPointInIteration.push_back( points[i][0] );
						bestPointInIteration.push_back( points[i][1] );
						bestPointInIteration.push_back( points[2+j][0] );
						bestPointInIteration.push_back( points[2+j][1] );
						bestPointInIteration.push_back( points[4+k][0] );
						bestPointInIteration.push_back( points[4+k][1] );
					}
				}	
			}
		}
		iteration++;
		
		// If iteration number will be equal to 50 all procedure is stopped, because with great probability the algorithm would not reconstruct good position
		if( iteration % 50 == 0 )
		{
			if( minDistance == 1000000. )
			{
				bestPointInIteration.clear();
				bestPointInIteration.push_back(100.);
				bestPointInIteration.push_back(100.);
				bestPointInIteration.push_back(100.);
				bestPointInIteration.push_back(100.);
				bestPointInIteration.push_back(100.);
				bestPointInIteration.push_back(100.);
				break;
			}
		}
		if( minDistance > previousDistance )
			iteration = 0;
		else
		{
			previousDistance = minDistance;
			bestPointFromPreviousIteration = bestPointInIteration;
		}
		radius0 += 1;
	}
	
	std::vector<double> radius0s, distances;
	if( minDistance != 1000000. )
		iteration = 1;
	
	double minnDistance = 1000000.;
	while( iteration )
	{
		radius1 = tDiffTOR1 + radius0 + 1;
		radius2 = tDiffTOR2 + radius0 + 1;
		radius3 = tDiffTOR2 + radius0 + 1;
		points = findIntersectionPointsOfCircles( hit1Pos, hit2Pos,hit3Pos, radius1, radius2, radius3, radius13, radius21, radius32 );

		minDistance = 1000000.;
		for( unsigned i=0; i<2; i++ )
		{
			for( unsigned j=0; j<2; j++ )
			{
				for( unsigned k=0; k<2; k++ )
				{
					distance = sqrt( pow(points[i][0] - points[j+2][0], 2) + pow(points[i][1] - points[j+2][1],2 ) ) + sqrt( pow(points[i][0] - points[k+4][0], 2) + pow(points[i][1] - points[k+4][1],2 ) ) + sqrt( pow(points[k+4][0] - points[j+2][0], 2) + pow(points[k+4][1] - points[j+2][1],2 ) );
					if( distance < minDistance )
					{
						minDistance = distance;
						bestPointInIteration.clear();
						bestPointInIteration.push_back( points[i][0] );
						bestPointInIteration.push_back( points[i][1] );
						bestPointInIteration.push_back( points[2+j][0] );
						bestPointInIteration.push_back( points[2+j][1] );
						bestPointInIteration.push_back( points[4+k][0] );
						bestPointInIteration.push_back( points[4+k][1] );
					}
				}	
			}
		}
		if( minDistance != 1000000. )
		{
			radius0s.push_back( radius0 );
			distances.push_back( minDistance );
		}
		
		iteration++;
		if( iteration % 50 == 0 )
		{
			if( minDistance == 1000000. )
			{
				bestPointInIteration.clear();
				bestPointInIteration.push_back(100.);
				bestPointInIteration.push_back(100.);
				bestPointInIteration.push_back(100.);
				bestPointInIteration.push_back(100.);
				bestPointInIteration.push_back(100.);
				bestPointInIteration.push_back(100.);
				break;
			}
			iteration = 0;
		}
		if( minDistance < minnDistance )
		  minnDistance = minDistance;
		
		previousDistance = minDistance;
		bestPointFromPreviousIteration = bestPointInIteration;
		radius0 -= 0.1;
	}
	
	if( minnDistance != 1000000. )
	{
		double radius0Min; 
		
		// Looking at the distances of intersection points, as the function of radius0
		double minEle = *std::min_element( std::begin(distances), std::end(distances) );
		if( minEle == distances[0] )
		{
			radius0Min = radius0s[0];
		}
		else if( minEle == distances[ distances.size() - 1 ] )
		{
			radius0Min = radius0s[ radius0s.size() -1 ];	  
		}
		else
		{
			//radius0Min = findMinFromQuadraticFit( radius0s, distances );
			radius0Min = findMinFromDerrivative( radius0s, distances );
		}
		radius1 = tDiffTOR1 + radius0Min + 1;
		radius2 = tDiffTOR2 + radius0Min + 1;
		radius3 = tDiffTOR2 + radius0Min + 1;
		points = findIntersectionPointsOfCircles( hit1Pos, hit2Pos,hit3Pos, radius1, radius2, radius3, radius13, radius21, radius32 );
		
		minDistance = 1000000.;
		for( unsigned i=0; i<2; i++ )
		{
			for( unsigned j=0; j<2; j++ )
			{
				for( unsigned k=0; k<2; k++ )
				{
					distance = sqrt( pow(points[i][0] - points[j+2][0], 2) + pow(points[i][1] - points[j+2][1],2 ) ) + sqrt( pow(points[i][0] - points[k+4][0], 2) + pow(points[i][1] - points[k+4][1],2 ) ) + sqrt( pow(points[k+4][0] - points[j+2][0], 2) + pow(points[k+4][1] - points[j+2][1],2 ) );
					if( distance < minDistance )
					{
						minDistance = distance;
						bestPointInIteration.clear();
						bestPointInIteration.push_back( points[i][0] );
						bestPointInIteration.push_back( points[i][1] );
						bestPointInIteration.push_back( points[2+j][0] );
						bestPointInIteration.push_back( points[2+j][1] );
						bestPointInIteration.push_back( points[4+k][0] );
						bestPointInIteration.push_back( points[4+k][1] );
					}
				}	
			}
		}
	
	}
	
	TVector3 recoPoint( (bestPointInIteration[0]+bestPointInIteration[2]+bestPointInIteration[4])/3, (bestPointInIteration[1]+bestPointInIteration[3]+bestPointInIteration[5])/3, hit1Pos(2) );
	
	return recoPoint;
}

// Finding minimum of function specified by Argument Arg and Values Val by fitting quadratic function in range close to minimum
double EventCategorizerTools::findMinFromQuadraticFit( std::vector<double> arg, std::vector<double> val ) 
{
	double X=0., X2=0., X3=0., X4=0., Y=0., XY=0., X2Y=0.;
	for( unsigned i=0; i<arg.size(); i++ )
	{
		X += arg[i];
		X2 += arg[i]*arg[i];
		X3 += arg[i]*arg[i]*arg[i];
		X4 += arg[i]*arg[i]*arg[i]*arg[i];
		Y += val[i];
		XY += arg[i]*val[i];
		X2Y += arg[i]*arg[i]*val[i];
	}
	double W = arg.size()*X2*X4 + X*X3*X2 + X2*X*X3 - X2*X2*X2 - X3*X3*arg.size() - X*X*X4;
	double Wa1 = arg.size()*XY*X4 + Y*X3*X2 + X2*X*X2Y - X2*XY*X2 - X3*X2Y*arg.size() - X*Y*X4;
	double Wa2 = arg.size()*X2*X2Y + X*XY*X2 + Y*X*X3 - Y*X2*X2 - XY*X3*arg.size() - X*X*X2Y;
	
	double a1,a2;
	
	if( W )
	{
		a1 = Wa1/W;
		a2 = Wa2/W;
	}
	else
	{
		std::cout << "What?, Error, big one, Quadratic, impossibru.." << std::endl; 
		return -1;
	}
	
	return -a1/2/a2;  
}

// Finding minimum of function specified by Argument Arg and Values Val by finding 0 of derrivative estimation of this function, and fitting linear function to derrivative
double EventCategorizerTools::findMinFromDerrivative( std::vector<double> arg, std::vector<double> val ) 
{
	double derr1 = val[1] - val[0];
	
	// Initializing indicator where to stop searching for minimum -> Where Derrivative is greater than zero
	unsigned stopInd = 0;
	for( unsigned i=1; i<val.size()-1; i++ )
	{
		if( derr1 < 0 )
		{
			derr1 = val[i+1] - val[i];
			if( derr1 > 0 )
			{
				stopInd = i;
				break;
			}
		}
	}
	
	// Calculation argument where derrivative would be zero from linear fit of two points -> last point where value is below zero, and next that is greater than zero
	double a = ( val[stopInd+1] - val[stopInd] - (val[stopInd] - val[stopInd-1]) )/( (arg[stopInd+1] + arg[stopInd])/2 - (arg[stopInd] + arg[stopInd-1])/2 );
	double b = val[stopInd+1] - val[stopInd] - a*(arg[stopInd+1] + arg[stopInd])/2;
	if( a )
		return -b/a;
	else
	{
		std::cout << "Error, wth, error MinDerrivative, no idea?!?" << std::endl;
		return 0;
	}
	
}

// Finding intersection points of three circles -> 6 points returned
std::vector< std::vector<double> > EventCategorizerTools::findIntersectionPointsOfCircles( TVector3 hit1Pos, TVector3 hit2Pos, TVector3 hit3Pos, double R1, double R2, double R3, double R13, double R21, double R32 )
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
	
	Points[0][0] = (hit1Pos(0) + hit2Pos(0) )/2 + (pow(R1,2) - pow(R2,2) )*( hit2Pos(0) - hit1Pos(0) )/2/pow( R21,2 ) + 0.5*( hit2Pos(1) - hit1Pos(1) )*sqrt( 2 * (pow(R1,2) + pow(R2,2) )/pow( R21,2 ) - pow( pow(R1,2) - pow(R2,2), 2 )/pow( R21, 4 ) - 1 );
	Points[0][1] = (hit1Pos(1) + hit2Pos(1) )/2 + (pow(R1,2) - pow(R2,2) )*( hit2Pos(1) - hit1Pos(1) )/2/pow( R21,2 ) + 0.5*( hit1Pos(0) - hit2Pos(0) )*sqrt( 2 * (pow(R1,2) + pow(R2,2) )/pow( R21,2 ) - pow( pow(R1,2) - pow(R2,2), 2 )/pow( R21, 4 ) - 1 );
	Points[1][0] = (hit1Pos(0) + hit2Pos(0) )/2 + (pow(R1,2) - pow(R2,2) )*( hit2Pos(0) - hit1Pos(0) )/2/pow( R21,2 ) - 0.5*( hit2Pos(1) - hit1Pos(1) )*sqrt( 2 * (pow(R1,2) + pow(R2,2) )/pow( R21,2 ) - pow( pow(R1,2) - pow(R2,2), 2 )/pow( R21, 4 ) - 1 );
	Points[1][1] = (hit1Pos(1) + hit2Pos(1) )/2 + (pow(R1,2) - pow(R2,2) )*( hit2Pos(1) - hit1Pos(1) )/2/pow( R21,2 ) - 0.5*( hit1Pos(0) - hit2Pos(0) )*sqrt( 2 * (pow(R1,2) + pow(R2,2) )/pow( R21,2 ) - pow( pow(R1,2) - pow(R2,2), 2 )/pow( R21, 4 ) - 1 );
	
	Points[2][0] = (hit2Pos(0) + hit3Pos(0) )/2 + (pow(R2,2) - pow(R3,2) )*( hit3Pos(0) - hit2Pos(0) )/2/pow( R32,2 ) + 0.5*( hit3Pos(1) - hit2Pos(1) )*sqrt( 2 * (pow(R2,2) + pow(R3,2) )/pow( R32,2 ) - pow( pow(R2,2) - pow(R3,2), 2 )/pow( R32, 4 ) - 1 );
	Points[2][1] = (hit2Pos(1) + hit3Pos(1) )/2 + (pow(R2,2) - pow(R3,2) )*( hit3Pos(1) - hit2Pos(1) )/2/pow( R32,2 ) + 0.5*( hit2Pos(0) - hit3Pos(0) )*sqrt( 2 * (pow(R2,2) + pow(R3,2) )/pow( R32,2 ) - pow( pow(R2,2) - pow(R3,2), 2 )/pow( R32, 4 ) - 1 );
	Points[3][0] = (hit2Pos(0) + hit3Pos(0) )/2 + (pow(R2,2) - pow(R3,2) )*( hit3Pos(0) - hit2Pos(0) )/2/pow( R32,2 ) - 0.5*( hit3Pos(1) - hit2Pos(1) )*sqrt( 2 * (pow(R2,2) + pow(R3,2) )/pow( R32,2 ) - pow( pow(R2,2) - pow(R3,2), 2 )/pow( R32, 4 ) - 1 );
	Points[3][1] = (hit2Pos(1) + hit3Pos(1) )/2 + (pow(R2,2) - pow(R3,2) )*( hit3Pos(1) - hit2Pos(1) )/2/pow( R32,2 ) - 0.5*( hit2Pos(0) - hit3Pos(0) )*sqrt( 2 * (pow(R2,2) + pow(R3,2) )/pow( R32,2 ) - pow( pow(R2,2) - pow(R3,2), 2 )/pow( R32, 4 ) - 1 );
	
	Points[4][0] = (hit1Pos(0) + hit3Pos(0) )/2 + (pow(R3,2) - pow(R1,2) )*( hit1Pos(0) - hit3Pos(0) )/2/pow( R13,2 ) + 0.5*( hit1Pos(1) - hit3Pos(1) )*sqrt( 2 * (pow(R3,2) + pow(R1,2) )/pow( R13,2 ) - pow( pow(R3,2) - pow(R1,2), 2 )/pow( R13, 4 ) - 1 );
	Points[4][1] = (hit1Pos(1) + hit3Pos(1) )/2 + (pow(R3,2) - pow(R1,2) )*( hit1Pos(1) - hit3Pos(1) )/2/pow( R13,2 ) + 0.5*( hit3Pos(0) - hit1Pos(0) )*sqrt( 2 * (pow(R3,2) + pow(R1,2) )/pow( R13,2 ) - pow( pow(R3,2) - pow(R1,2), 2 )/pow( R13, 4 ) - 1 );
	Points[5][0] = (hit1Pos(0) + hit3Pos(0) )/2 + (pow(R3,2) - pow(R1,2) )*( hit1Pos(0) - hit3Pos(0) )/2/pow( R13,2 ) - 0.5*( hit1Pos(1) - hit3Pos(1) )*sqrt( 2 * (pow(R3,2) + pow(R1,2) )/pow( R13,2 ) - pow( pow(R3,2) - pow(R1,2), 2 )/pow( R13, 4 ) - 1 );
	Points[5][1] = (hit1Pos(1) + hit3Pos(1) )/2 + (pow(R3,2) - pow(R1,2) )*( hit1Pos(1) - hit3Pos(1) )/2/pow( R13,2 ) - 0.5*( hit3Pos(0) - hit1Pos(0) )*sqrt( 2 * (pow(R3,2) + pow(R1,2) )/pow( R13,2 ) - pow( pow(R3,2) - pow(R1,2), 2 )/pow( R13, 4 ) - 1 );
	
	return Points;
}

// Test that checks if the two hits can come from scattering - Hit1 primary gamma, Hit2 - scattered gamma. ErrorInterval specifies resolution of time calculations
int EventCategorizerTools::checkIfScattered( JPetHit hit1, JPetHit hit2, double errorInterval ) 
{
	double tDiff = fabs(hit2.getTime()/1000 - hit1.getTime()/1000);
	double distance = ( hit2.getPos() - hit1.getPos() ).Mag();
	double scattTime = distance/kLightVelocityCmS;
	if( fabs( tDiff - scattTime ) < errorInterval )
		return 1;
	else
		return 0;
	
}

TVector3 EventCategorizerTools::decayInto3PosReco_AlekVersion( JPetHit hit1, JPetHit hit2, JPetHit hit3 ) // Copy of the procedure that Alek uses to reconstruct position from 3 Hits
{
  
  TVector3 hit1Pos( hit1.getPosX(), hit1.getPosY(), hit1.getPosZ() );
  TVector3 hit2Pos( hit2.getPosX(), hit2.getPosY(), hit2.getPosZ() );
  TVector3 hit3Pos( hit3.getPosX(), hit3.getPosY(), hit3.getPosZ() );  
  double t1 = hit1.getTime()/1000;
  double t2 = hit2.getTime()/1000;
  double t3 = hit3.getTime()/1000;
  
  // find the decay plane
  TVector3 normal = ((hit2Pos-hit1Pos).Cross( hit3Pos-hit1Pos )).Unit();
  
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
    gammas2D[0] = rot * hit1Pos;
    gammas2D[1] = rot * hit2Pos;
    gammas2D[2] = rot * hit3Pos;
  
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
    M[k][2] = 2.*kLightVelocityCmS*kLightVelocityCmS*( times[j] - times[i] );       
    D[k] = pow(gammas2D[i].X(),2.)
      - pow(gammas2D[j].X(),2.)
      + pow(gammas2D[i].Y(),2.)
      - pow(gammas2D[j].Y(),2.)
      - kLightVelocityCmS*kLightVelocityCmS*pow(times[i],2.)
      + kLightVelocityCmS*kLightVelocityCmS*pow(times[j],2.);
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
  
  a = Ex*Ex + Ey*Ey - kLightVelocityCmS*kLightVelocityCmS;
  b = 2.*( Ex*(Fx-gammas2D[0].X()) + Ey*(Fy-gammas2D[0].Y()) + kLightVelocityCmS*kLightVelocityCmS*times[0] );
  cc = pow(Fx-gammas2D[0].X(), 2.) + pow(Fy-gammas2D[0].Y(), 2.) - kLightVelocityCmS*kLightVelocityCmS*pow(times[0], 2.);
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

// Copy of the procedure that Alek uses to reconstruct position from 3 Hits with some addition of finding the best solution
TVector3 EventCategorizerTools::decayInto3PosReco_AlekVersion_withAddition( JPetHit hit1, JPetHit hit2, JPetHit hit3 ) 
{
  
  TVector3 hit1Pos( hit1.getPosX(), hit1.getPosY(), hit1.getPosZ() );
  TVector3 hit2Pos( hit2.getPosX(), hit2.getPosY(), hit2.getPosZ() );
  TVector3 hit3Pos( hit3.getPosX(), hit3.getPosY(), hit3.getPosZ() );  
  double t1 = hit1.getTime()/1000;
  double t2 = hit2.getTime()/1000;
  double t3 = hit3.getTime()/1000;
  // find the decay plane
  TVector3 normal = ((hit2Pos-hit1Pos).Cross( hit3Pos-hit1Pos )).Unit();
  
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
    gammas2D[0] = rot * hit1Pos;
    gammas2D[1] = rot * hit2Pos;
    gammas2D[2] = rot * hit3Pos;
  
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
    M[k][2] = 2.*kLightVelocityCmS*kLightVelocityCmS*( times[j] - times[i] );       
    D[k] = pow(gammas2D[i].X(),2.)
      - pow(gammas2D[j].X(),2.)
      + pow(gammas2D[i].Y(),2.)
      - pow(gammas2D[j].Y(),2.)
      - kLightVelocityCmS*kLightVelocityCmS*pow(times[i],2.)
      + kLightVelocityCmS*kLightVelocityCmS*pow(times[j],2.);
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
  
  a = Ex*Ex + Ey*Ey - kLightVelocityCmS*kLightVelocityCmS;
  b = 2.*( Ex*(Fx-gammas2D[0].X()) + Ey*(Fy-gammas2D[0].Y()) + kLightVelocityCmS*kLightVelocityCmS*times[0] );
  cc = pow(Fx-gammas2D[0].X(), 2.) + pow(Fy-gammas2D[0].Y(), 2.) - kLightVelocityCmS*kLightVelocityCmS*pow(times[0], 2.);
  delta = b*b - 4.*a*cc;
  
    if( delta < 0. )
    {
      return testy;
    }
    
    sol_time[0] = (-1.*b - sqrt(delta))/(2.*a);
    sol_time[1] = (-1.*b + sqrt(delta))/(2.*a);
    
    TVector3 sol1( Ex*sol_time[0]+Fx, Ey*sol_time[0]+Fy, 0 );
    TVector3 sol2( Ex*sol_time[1]+Fx, Ey*sol_time[1]+Fy, 0 );
    int bttSol = ( calcDistFromCentres( sol1, gammas2D[0], gammas2D[1], gammas2D[2] ) < calcDistFromCentres( sol2, gammas2D[0], gammas2D[1], gammas2D[2] ) ? 0 : 1 );
    
    for(int i = 0; i<2;++i){
      TVector3 sol2Dv( Ex*sol_time[i]+Fx, Ey*sol_time[i]+Fy, gammas2D[0].Z() );
      
      // transform the solution back to 3D
      sol_hit[i] =  rot.Inverse() * sol2Dv;
      
    }
    
  
  return sol_hit[bttSol];
}
// Test for better solution in reconstruction of position from 3 hits, that checkes which solution lies closer to the hit positions - Gamma1, Gamma2, Gamma3. Method for reconstruction method of Alek
double EventCategorizerTools::calcDistFromCentres( TVector3 sol1, TVector3 gamma1, TVector3 gamma2, TVector3 gamma3 )
{
	return (sol1 - gamma1).Mag() + (sol1 - gamma2).Mag() + (sol1 - gamma3).Mag();		
}

// Calculation of time of hit with substraction of TOF of the hit. Assuming that gamma came from (0,0,0)
double EventCategorizerTools::normalizeTime( JPetHit hit1 ) 
{
	double vecLength = hit1.getPos().Mag();
	return hit1.getTime()/1000 - vecLength/kLightVelocityCmS; // in ns
}

// Calculation of time of hit with substraction of TOF of the hit. Assuming that gamma came from Point
double EventCategorizerTools::normalizeTimeToPoint( JPetHit hit1, TVector3 point ) 
{
	double vecLength = ( hit1.getPos() - point ).Mag();
	return hit1.getTime()/1000 - vecLength/kLightVelocityCmS; // in ns
}

// Reconstruction of position from back-to-back hits
TVector3 EventCategorizerTools::recoPosition( const JPetHit & hit1, const JPetHit & hit2) 
{
	TVector3 reconstructedPosition;
	double tof = fabs( hit1.getTime() - hit2.getTime() )/1000;
	double vecLength = ( hit1.getPos() - hit2.getPos() ).Mag();
	double middleX = (hit1.getPosX()+hit2.getPosX() )/2;
	double middleY = (hit1.getPosY()+hit2.getPosY() )/2;
	double middleZ = (hit1.getPosZ()+hit2.getPosZ() )/2;
	double fraction = 2*tof*kLightVelocityCmS/vecLength;
	if( hit1.getTime() >= hit2.getTime() )
	{
		reconstructedPosition(0) = middleX + fraction*( hit1.getPosX()-middleX );
		reconstructedPosition(1) = middleY + fraction*( hit1.getPosY()-middleY );
		reconstructedPosition(2) = middleZ + fraction*( hit1.getPosZ()-middleZ );
	}
	else
	{
		reconstructedPosition(0) = middleX + fraction*( hit2.getPosX()-middleX );
		reconstructedPosition(1) = middleY + fraction*( hit2.getPosY()-middleY );
		reconstructedPosition(2) = middleZ + fraction*( hit2.getPosZ()-middleZ );
	}
	return reconstructedPosition;
}