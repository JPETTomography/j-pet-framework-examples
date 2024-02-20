#include "reconstructor.h"
#include <iostream>
#include <cassert>
#include <TMinuit.h>
#include <TMath.h>

Reconstructor * Reconstructor::_this = NULL;

Reconstructor * Reconstructor::GetInstance(){ 
  if( _this == NULL ){
    _this = new Reconstructor();
  }

  return _this; 
}


// global function for the "Old" version of Minuit
void gfcn(Int_t &npar, Double_t *gin, Double_t &f, Double_t *par, Int_t iflag){

  Reconstructor * rec = Reconstructor::GetInstance();
  f = rec->fcn( par );
  
}

int Reconstructor::kinematicFit(float & chisq){
  
  int status = 0;
  
  // n_fit_params = 12;
  
  // TMinuit *gMinuit = new TMinuit(n_fit_params);
  // gMinuit->SetFCN(gfcn);
  // gMinuit->SetPrintLevel(-1);
  // gMinuit->SetErrorDef( 1.0 );
  
  // double fStripWidth = 0.7;
  // double fStripHeight = 1.9;
    
  
  // // set parameters
  // for(int i=0;i<3;i++){ // 3 gammas
  //   p0[4*i] = 0.;
  //   p0[4*i+1] = 0.;
  //   p0[4*i+2] = 0.;
  //   p0[4*i+3] = 0.;
  //   sigmas[4*i] = 0.3*fStripWidth/(2*sqrt(3));
  //   sigmas[4*i+1] = 0.3*fStripHeight/(2*sqrt(3));
  //   sigmas[4*i+2] = 0.93;
  //   sigmas[4*i+3] = 0.08;

  //   double center = 0.0;
    
  //   gMinuit->mnparm(4*i, Form("x%d", i), center, sigmas[4*i], -0.5*fStripWidth, 0.5*fStripWidth, status);
  //   gMinuit->mnparm(4*i+1, Form("y%d", i), center, sigmas[4*i+1], -0.5*fStripHeight, 0.5*fStripHeight, status);
  //   /* gMinuit->mnparm(4*i+2, Form("z%d", i), center, sigmas[4*i+2], -3.*sigmas[4*i+2], 3.*sigmas[4*i+2], status); */
  //   /* gMinuit->mnparm(4*i+3, Form("t%d", i), center, sigmas[4*i+3], -3.*sigmas[4*i+3], 3.*sigmas[4*i+3], status); */
  //   gMinuit->mnparm(4*i+2, Form("z%d", i), center, sigmas[4*i+2], 0., 0., status);
  //   gMinuit->mnparm(4*i+3, Form("t%d", i), center, sigmas[4*i+3], 0., 0., status);

  // }

  // /* gMinuit->FixParameter(3); */
  // /* gMinuit->FixParameter(7); */
  // /* gMinuit->FixParameter(11); */
  
  // Double_t arglist[10];
  // arglist[0] = 2;
  // gMinuit->mnexcm( "SET STR", arglist, 2, status );

  // arglist[0] = 10000.;
  // arglist[1] = 0.1;
  // gMinuit->mnexcm("MINIMIZE", arglist, 2, status);

  // //
  // /* for(int i=0;i>n_fit_params;++i){ */
  // /*   gMinuit->FixParameter(i); */
  // /* } */
  // /* gMinuit->Release(3); */
  // /* gMinuit->Release(7); */
  // /* gMinuit->Release(11); */

  // /* gMinuit->mnexcm("MINIMIZE", arglist, 2, status); */
  // //
  
  // Double_t amin,edm,errdef;
  // Int_t nvpar,nparx,icstat;
  // gMinuit->mnstat(amin,edm,errdef,nvpar,nparx,icstat);

  // // *minval = amin;
  
  // // get resulting parameters
  // TString name;
  // Double_t val, err, llim, ulim;
  // Int_t  aaa;
  // double *x = new double[n_fit_params];
  // for(int i=0;i<n_fit_params;i++){
  //   gMinuit->mnpout( i, name, val, err, llim, ulim, aaa );
  //   x[i] = val;
  // }

  // chisq = getChisq( x );

  // setParameters( x );


  


  
  // delete[] x;
  // delete gMinuit;
  
  return status;
  
}

double Reconstructor::getChisq(const double * x) const{


  double chi = 0.0;
  for(int i=0;i<n_fit_params;++i){
    chi += pow( (x[i]-p0[i])/sigmas[i] , 2.);
  }

  return chi;
}

double Reconstructor::fcn(const double * x){

  double lambda = 100.0;
  double constraint = 0.0;

  setParameters( x );
  reconstruct();
  
  double X = sol_hit[1].X();
  double Y = sol_hit[1].Y();
  double Z = sol_hit[1].Z();
  
  for(int i=0;i<3;++i){
    constraint += pow( sqrt(X*X + Y*Y) - fChamberRadius, 2. ) ;/// 4.0;
    /* if( Z < -5.0 ){ */
    /*   constraint += pow( Z + 5.0, 2. ) ; */
    /* }else if( Z > 5.0 ){ */
    /*   constraint += pow( Z - 5.0, 2. ) ; */
    /* } */
    
  }

  return getChisq( x ) + lambda * constraint;
  
}

void Reconstructor::setParameters(const double * x){
  for(int i=0;i<3;i++){ // 3 gammas
    gammas_shift[i].SetXYZ(x[4*i+0], x[4*i+1], x[4*i+2]);
    gammas_time_shift[i] = x[4*i+3];
  }
}

/**
 * Returns the absolute position of the corrected hit
 * in the Barrel frame of reference
 */
TVector3 Reconstructor::getGammaHit(int i) const {
  
  double angle = gammas_polar_angle[i] * TMath::DegToRad();
      
  TVector3 shift_rotated = gammas_shift[i];
  shift_rotated.RotateZ( angle );

  return gammas_hit[i] + shift_rotated;
}

Double_t Reconstructor::getGammaTime(int i) const {
  return gammas_time[i] + gammas_time_shift[i];
}


void Reconstructor::fillParamsArray(Float_t * params){
  
  for(int i=0;i<3;i++){ // 3 gammas
    params[4*i] = getGammaHit(i).X();
    params[4*i+1] = getGammaHit(i).Y();
    params[4*i+2] = getGammaHit(i).Z();
    params[4*i+3] = getGammaTime(i);
  }
}

void Reconstructor::fillShiftsArray(Float_t * shifts){
  
  for(int i=0;i<3;i++){ // 3 gammas
    shifts[4*i] = gammas_shift[i].X();
    shifts[4*i+1] = gammas_shift[i].Y();
    shifts[4*i+2] = gammas_shift[i].Z();
    shifts[4*i+3] = gammas_time_shift[i];
  }
}


// end of stuff for the kinematic fit



const Double_t Reconstructor::cvel = 29.9792458; // cm/ns

/**
 * @brief Set the length of the barrel in centimeters
 *
 * The barrel length must be set for the reconstruction to work corrrectly
 */
void Reconstructor::setBarrelLength(double length){
  fStripLength = length;
}

/**
 * @brief Set the transverse dimensions [cm] of the scintillator strips
 *
 * @param width  dimension of the strip cross-section perpendicular to the barrel radius
 * @param height dimension of the strip cross-section parallel to the barrel radius
 *
 * These dimensions must be set for the kinematic fit to work correctly.
 * General o-Ps->3gamma reconstruction does not use these values.
 */
void Reconstructor::setStripDimensions(double width, double height){
  fStripWidth = width;
  fStripHeight = height;
}

void Reconstructor::setGamma(ushort i, const JPetBaseHit* hit){
  assert(i <= 2);
  gammas_hit[i] = hit->getPos();
  gammas_time[i] = hit->getTime() / 1000.;
  //!changed...modular
  gammas_polar_angle[i] = hit->getScin().getSlot().getTheta();
  // initialize structures for the kinematic fit
  gammas_time_shift[i] = 0;
  gammas_shift[i].SetXYZ(0, 0, 0);
}

void Reconstructor::setPromptGamma(const JPetBaseHit* hit){
  prompt_gamma_hit = hit->getPos();
  prompt_gamma_time = hit->getTime();
}

int Reconstructor::getSolution(TVector3 & point, Double_t & time, int whichSolution){
  int result = reconstruct();

  point = sol_hit[whichSolution];
  time = sol_time[whichSolution];

  return result;
};

int Reconstructor::reconstruct(){
  
  int errFlag = 0;

  // find the decay plane
  TVector3 normal = ((getGammaHit(1)-getGammaHit(0)).Cross( getGammaHit(2)-getGammaHit(0) )).Unit();
  
  // prepare transformation to the decay plane
  TVector3 z(0.,0.,1.);
  TVector3 rotAxis = normal.Cross( z );
  double angle = z.Angle( normal ); // radians
  
  TRotation rot;
  rot.Rotate(angle, rotAxis);
  
  // transform gamma hits to decay plane
  TVector3 gammas2D[3];
  for(int i=0;i<3;i++){
    gammas2D[i] = rot * getGammaHit(i);
  }
  
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
    M[k][2] = 2.*cvel*cvel*( getGammaTime(j) - getGammaTime(i) );       
    D[k] = pow(gammas2D[i].X(),2.)
      - pow(gammas2D[j].X(),2.)
      + pow(gammas2D[i].Y(),2.)
      - pow(gammas2D[j].Y(),2.)
      - cvel*cvel*pow(getGammaTime(i),2.)
      + cvel*cvel*pow(getGammaTime(j),2.);
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
  
  a = Ex*Ex + Ey*Ey - cvel*cvel;
  b = 2.*( Ex*(Fx-gammas2D[0].X()) + Ey*(Fy-gammas2D[0].Y()) + cvel*cvel*getGammaTime(0) );
  cc = pow(Fx-gammas2D[0].X(), 2.) + pow(Fy-gammas2D[0].Y(), 2.) - cvel*cvel*pow(getGammaTime(0), 2.);
  delta = b*b - 4.*a*cc;
    if( delta < 0. ){
      errFlag = 1;
      return errFlag;
    }
    sol_time[0] = (-1.*b - sqrt(delta))/(2.*a);
    sol_time[1] = (-1.*b + sqrt(delta))/(2.*a);
    
    for(int i = 0; i<2;++i){
      TVector3 sol2Dv( Ex*sol_time[i]+Fx, Ey*sol_time[i]+Fy, gammas2D[0].Z() );
      
      // transform the solution back to 3D
      sol_hit[i] =  rot.Inverse() * sol2Dv;
      
    }
    
    // check solution 2 for reasonability
    if( errFlag == 0 ){
      if( sol_time[1] < 0. || sol_time[1] > 50000.0  ){
	errFlag = 2;
      }else if( sol_hit[1].Perp() > getGammaHit(1).Perp() ||
		fabs( sol_hit[1].Z() ) > fStripLength/2. ){
	errFlag = 3;
      }else if( TMath::IsNaN( sol_time[1] ) ||
		TMath::IsNaN( sol_hit[1].X() ) ||
		TMath::IsNaN( sol_hit[1].Y() ) ||
		TMath::IsNaN( sol_hit[1].Z() )
		){
	errFlag = 4;
      }
    }
    
  
  return errFlag;
}


double Reconstructor::getPScreationTime(){
  double prompt_path = (prompt_gamma_hit - sol_hit[1]).Mag();
  double prompt_emission_time = prompt_gamma_time - prompt_path / cvel;
  return prompt_emission_time;
}

double Reconstructor::getPSlifetime(){
  return sol_time[1] - getPScreationTime();
}


/*
//---------------Kamil's code------------//

TVector3 Reconstructor::RecoPos( const JPetBaseHit  Hit1, const JPetBasetHit  Hit2, const  JPetBaseHit Hit3 )
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

	TVector3 Point = FindIntersection2( P1, P2, P3, Hit2.getTime()/1000 - Hit1.getTime()/1000, Hit3.getTime()/1000 - Hit1.getTime()/1000 );
	
	TVector3 brak(1000,1000,1000);
	if( Point(0) == 100. && Point(1) == 100. )
	  return brak;
	
	TVector3 RotationXr( cos(-Theta)+vecPerpendicular(0)*vecPerpendicular(0)*(1-cos(-Theta)), vecPerpendicular(0)*vecPerpendicular(1)*(1-cos(-Theta)), vecPerpendicular(1)*sin(-Theta) );
	TVector3 RotationYr( vecPerpendicular(0)*vecPerpendicular(1)*(1-cos(-Theta)), cos(-Theta)+vecPerpendicular(1)*vecPerpendicular(1)*(1-cos(-Theta)), -vecPerpendicular(0)*sin(-Theta) );
	TVector3 RotationZr( -vecPerpendicular(1)*sin(-Theta), vecPerpendicular(0)*sin(-Theta), cos(-Theta) );
	
	TVector3 Pointr( RotationXr*Point, RotationYr*Point, RotationZr*Point );
*/
	/*std::cout << std::endl;
	std::cout << "Points" << std::endl;
	std::cout << Pointr(0) << " " << Pointr(1) << " " << Pointr(2) << std::endl;
	std::cout << "Points" << std::endl;
	std::cout << std::endl;*/
/*	
	TVector3 Hit1Position( Hit1.getPosX(), Hit1.getPosY(), Hit1.getPosZ() );
	TVector3 Hit2Position( Hit2.getPosX(), Hit2.getPosY(), Hit2.getPosZ() );
	TVector3 Hit3Position( Hit3.getPosX(), Hit3.getPosY(), Hit3.getPosZ() );
	
	return Pointr;
}
*/

 /*
TVector3 Reconstructor::FindIntersection2( TVector3 Hit1Pos, TVector3 Hit2Pos, TVector3 Hit3Pos, double t21, double t31 )
{
  	//std::cout << t21 << " " << t31 << std::endl;

  
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
		//	std::cout << R0 << " " << MinDistance << std::endl;
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
		  //! Function not defined
		  //R0Min = FindMinFromQuadraticFit( R0s, Distances );
		  
		  R0Min = FindMinFromDerrivative( R0s, Distances );
		}
		//std::cout << R0Min << std::endl;
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

	// trail
	//R0s.clear();
	//Distances.clear();
	
	return RecoPoint;
}
*/
/*
double Reconstructor::FindMinFromDerrivative(  vector<double> Arg,  vector<double> Val )
{
	double Derr1 = Val[1] - Val[0];
	unsigned StopInd = 1;
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
	    //	std::cout << "Error, wtf, error, no idea?!?" << std::endl;
	    return 0;
	  }
	//Arg.clear();
	//Val.clear();
}

vector< vector<double> > Reconstructor::FindIntersectionPointsOfCircles( TVector3 Hit1Pos, TVector3 Hit2Pos, TVector3 Hit3Pos, double R1, double R2, double R3, double R13, double R21, double R32 )
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
*/
/*
double Reconstructor::CalcDistFromCentres( TVector3 Sol1, TVector3 Gamma1, TVector3 Gamma2, TVector3 Gamma3 )
{
	double distance1 = 0.;
	distance1 = sqrt( pow(Sol1(0) - Gamma1(0),2) + pow(Sol1(1) - Gamma1(1),2) + pow(Sol1(2) - Gamma1(2),2) ) + sqrt( pow(Sol1(0) - Gamma2(0),2) + pow(Sol1(1) - Gamma2(1),2) + pow(Sol1(2) - Gamma2(2),2) ) + sqrt( pow(Sol1(0) - Gamma3(0),2) + pow(Sol1(1) - Gamma3(1),2) + pow(Sol1(2) - Gamma3(2),2) );		
	return distance1;
}

double Reconstructor::CalcDistAdd( const JPetBaseHit  Hit1 )
{
	TVector3 vec1( Hit1.getPosX(), Hit1.getPosY(), Hit1.getPosZ() );
	double LengthR = 40;
	double Length0 = vec1.Mag();
	return (Length0 - LengthR)/29.979246;
}
*/
